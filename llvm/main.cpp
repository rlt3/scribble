#include <iostream>
#include <fstream>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h" // llvm::errs
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h" // TODO: handle 'llvm/MC/TargetRegistry.h' on different platforms
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/SmallVectorMemoryBuffer.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileOnDemandLayer.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/IRTransformLayer.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace llvm;
using namespace llvm::orc;

typedef int (*FunctionEntry) (int, char*[]);

class JITMachine {
private:
  ExecutionSession ES;
  std::map<VModuleKey, std::shared_ptr<SymbolResolver>> Resolvers;
  std::unique_ptr<TargetMachine> TM;
  const DataLayout DL;
  LegacyRTDyldObjectLinkingLayer ObjectLayer;
  LegacyIRCompileLayer<decltype(ObjectLayer), SimpleCompiler> CompileLayer;

  using OptimizeFunction =
      std::function<std::unique_ptr<Module>(std::unique_ptr<Module>)>;

  LegacyIRTransformLayer<decltype(CompileLayer), OptimizeFunction> OptimizeLayer;

  std::unique_ptr<JITCompileCallbackManager> CompileCallbackManager;
  LegacyCompileOnDemandLayer<decltype(OptimizeLayer)> CODLayer;

public:
  JITMachine()
      : TM(EngineBuilder().selectTarget()), DL(TM->createDataLayout()),
        ObjectLayer(AcknowledgeORCv1Deprecation, ES,
                    [this](VModuleKey K) {
                      return LegacyRTDyldObjectLinkingLayer::Resources{
                          std::make_shared<SectionMemoryManager>(),
                          Resolvers[K]};
                    }),
        CompileLayer(AcknowledgeORCv1Deprecation, ObjectLayer,
                     SimpleCompiler(*TM)),
        OptimizeLayer(AcknowledgeORCv1Deprecation, CompileLayer,
                      [this](std::unique_ptr<Module> M) {
                        return optimizeModule(std::move(M));
                      }),
        CompileCallbackManager(cantFail(orc::createLocalCompileCallbackManager(
            TM->getTargetTriple(), ES, 0))),
        CODLayer(
            AcknowledgeORCv1Deprecation, ES, OptimizeLayer,
            [&](orc::VModuleKey K) { return Resolvers[K]; },
            [&](orc::VModuleKey K, std::shared_ptr<SymbolResolver> R) {
              Resolvers[K] = std::move(R);
            },
            [](Function &F) { return std::set<Function *>({&F}); },
            *CompileCallbackManager,
            orc::createLocalIndirectStubsManagerBuilder(
                TM->getTargetTriple())) {
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
  }

  TargetMachine &getTargetMachine() { return *TM; }

  VModuleKey addModule(std::unique_ptr<Module> M) {
    // Create a new VModuleKey.
    VModuleKey K = ES.allocateVModule();

    // Build a resolver and associate it with the new key.
    Resolvers[K] = createLegacyLookupResolver(
        ES,
        [this](const std::string &Name) -> JITSymbol {
          if (auto Sym = CompileLayer.findSymbol(Name, false))
            return Sym;
          else if (auto Err = Sym.takeError())
            return std::move(Err);
          if (auto SymAddr =
                  RTDyldMemoryManager::getSymbolAddressInProcess(Name))
            return JITSymbol(SymAddr, JITSymbolFlags::Exported);
          return nullptr;
        },
        [](Error Err) { cantFail(std::move(Err), "lookupFlags failed"); });

    // Add the module to the JIT with the new key.
    cantFail(CODLayer.addModule(K, std::move(M)));
    return K;
  }

  JITSymbol findSymbol(const std::string Name) {
    std::string MangledName;
    raw_string_ostream MangledNameStream(MangledName);
    Mangler::getNameWithPrefix(MangledNameStream, Name, DL);
    return CODLayer.findSymbol(MangledNameStream.str(), true);
  }

  void removeModule(VModuleKey K) {
    cantFail(CODLayer.removeModule(K));
  }

private:
  std::unique_ptr<Module> optimizeModule(std::unique_ptr<Module> M) {
    // Create a function pass manager.
    auto FPM = std::make_unique<legacy::FunctionPassManager>(M.get());

    // Add some optimizations.
    FPM->add(createInstructionCombiningPass());
    FPM->add(createReassociatePass());
    FPM->add(createGVNPass());
    FPM->add(createCFGSimplificationPass());
    FPM->doInitialization();

    // Run the optimizations over all functions in the module being added to
    // the JIT.
    for (auto &F : *M)
      FPM->run(F);

    return M;
  }
};

int
main (int argc, char **argv)
{
    LLVMContext context;
    std::string errstr;
    SMDiagnostic errhandler;

    // only initialize the native target
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    // create a MemoryBuffer from our basic IR 
    std::string basicIR = 
        "define i32 @main() {\n"
        "\tret i32 72\n"
        "}";
    std::unique_ptr<MemoryBuffer> IRbuff = MemoryBuffer::getMemBuffer(basicIR);

    // create a Module from IR parsed
    std::unique_ptr<Module> ir_module = parseIR(*IRbuff, errhandler, context);

    // add the compiled ir to the JIT, compiling it to machine code
    JITMachine JIT;
    JIT.addModule(std::move(ir_module));

    // i think the following compiles the JIT module somehow?
    std::unique_ptr<Module> module = std::make_unique<Module>("my cool jit", context);
    module->setDataLayout(JIT.getTargetMachine().createDataLayout());

    auto main = JIT.findSymbol("main");
    if (!main) {
        errs() << "Couldn't find main!\n";
        return 1;
    }
    FunctionEntry entry = (FunctionEntry) (intptr_t) cantFail(main.getAddress());
    outs() << "main returns: " << entry(0, NULL) << "\n";

    return 0;
}

//int
//main (int argc, char **argv)
//{
//    LLVMContext context;
//    std::string errstr;
//    SMDiagnostic errhandler;
//
//    // only initialize the native target
//    llvm::InitializeNativeTarget();
//    llvm::InitializeNativeTargetAsmParser();
//    llvm::InitializeNativeTargetAsmPrinter();
//
//    // create a MemoryBuffer from our basic IR 
//    std::string basicIR = 
//        "define i32 @main() {\n"
//        "\tret i32 72\n"
//        "}";
//    std::unique_ptr<MemoryBuffer> IRbuff = MemoryBuffer::getMemBuffer(basicIR);
//
//    // create a Module from IR parsed
//    std::unique_ptr<Module> module = parseIR(*IRbuff, errhandler, context);
//
//    // get the native target
//    auto target_triple = sys::getDefaultTargetTriple();
//    auto target = TargetRegistry::lookupTarget(target_triple, errstr);
//    if (!target) {
//        errs() << errstr << "\n";
//        return 1;
//    }
//
//    // and construct a target machine from the native target
//    auto machine_cpu = "generic";
//    auto machine_features = "";
//    TargetOptions machine_options;
//    auto machine_reloc = Optional<Reloc::Model>();
//    auto target_machine = target->createTargetMachine(target_triple,
//            machine_cpu, machine_features, machine_options, machine_reloc);
//
//    // update our model with the specific features of the native target
//    module->setTargetTriple(sys::getDefaultTargetTriple());
//    module->setDataLayout(target_machine->createDataLayout());
//
//    // create a buffer for our machine code
//    SmallVector<char, 4096> objbuffer;
//      raw_svector_ostream objstream(objbuffer);
//
//    // create a pass manager, add the passes to emit machine code, and run them
//    legacy::PassManager passes;
//    MCContext *mccontext;
//    if (target_machine->addPassesToEmitMC(passes, mccontext, objstream)) {
//        errs() << "NativeTarget doesn't support emitting machine code!\n";
//        return 1;
//    }
//    passes.run(*module);
//
//    // flushes the objstream and initializes MemoryBuffer from objbuffer
//    std::unique_ptr<MemoryBuffer> mcbuffer(
//            new SmallVectorMemoryBuffer(std::move(objbuffer)));
//
//    module->print(errs(), nullptr);
//
//    // note: this binary isn't fully linked so it won't execute
//    std::ofstream bin;
//    bin.open("a.bin", std::ios::binary);
//    bin.write(mcbuffer->getBufferStart(), mcbuffer->getBufferSize());
//    bin.close();
//
//    //std::string mangledName;
//    //raw_string_ostream mangledNameStream(mangledName);
//    //Mangler::getNameWithPrefix(mangledNameStream, "main", module->getDataLayout());
//
//    Function *call = module->getFunction("main");
//    if (!call) {
//        errs() << "Didn't find main!\n";
//    } else {
//        outs() << "Found main!!!\n";
//    }
//
//    return 0;
//}
