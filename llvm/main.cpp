#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/CompileOnDemandLayer.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/IRTransformLayer.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/SmallVectorMemoryBuffer.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

using namespace llvm;
using namespace llvm::orc;

typedef int (*FunctionEntry) (int, char*[]);

//class JITMachine {
//private:
//  ExecutionSession ES;
//  std::map<VModuleKey, std::shared_ptr<SymbolResolver>> Resolvers;
//  std::unique_ptr<TargetMachine> TM;
//  const DataLayout DL;
//  LegacyRTDyldObjectLinkingLayer ObjectLayer;
//  LegacyIRCompileLayer<decltype(ObjectLayer), SimpleCompiler> CompileLayer;
//
//  using OptimizeFunction =
//      std::function<std::unique_ptr<Module>(std::unique_ptr<Module>)>;
//
//  LegacyIRTransformLayer<decltype(CompileLayer), OptimizeFunction> OptimizeLayer;
//
//  std::unique_ptr<JITCompileCallbackManager> CompileCallbackManager;
//  LegacyCompileOnDemandLayer<decltype(OptimizeLayer)> CODLayer;
//
//public:
//  JITMachine()
//      : TM(EngineBuilder().selectTarget()), DL(TM->createDataLayout()),
//        ObjectLayer(AcknowledgeORCv1Deprecation, ES,
//                    [this](VModuleKey K) {
//                      return LegacyRTDyldObjectLinkingLayer::Resources{
//                          std::make_shared<SectionMemoryManager>(),
//                          Resolvers[K]};
//                    }),
//        CompileLayer(AcknowledgeORCv1Deprecation, ObjectLayer,
//                     SimpleCompiler(*TM)),
//        OptimizeLayer(AcknowledgeORCv1Deprecation, CompileLayer,
//                      [this](std::unique_ptr<Module> M) {
//                        return optimizeModule(std::move(M));
//                      }),
//        CompileCallbackManager(cantFail(orc::createLocalCompileCallbackManager(
//            TM->getTargetTriple(), ES, 0))),
//        CODLayer(
//            AcknowledgeORCv1Deprecation, ES, OptimizeLayer,
//            [&](orc::VModuleKey K) { return Resolvers[K]; },
//            [&](orc::VModuleKey K, std::shared_ptr<SymbolResolver> R) {
//              Resolvers[K] = std::move(R);
//            },
//            [](Function &F) { return std::set<Function *>({&F}); },
//            *CompileCallbackManager,
//            orc::createLocalIndirectStubsManagerBuilder(
//                TM->getTargetTriple())) {
//    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
//  }
//
//  TargetMachine &getTargetMachine() { return *TM; }
//
//  VModuleKey addModule(std::unique_ptr<Module> M) {
//    // Create a new VModuleKey.
//    VModuleKey K = ES.allocateVModule();
//
//    // Build a resolver and associate it with the new key.
//    Resolvers[K] = createLegacyLookupResolver(
//        ES,
//        [this](const std::string &Name) -> JITSymbol {
//          if (auto Sym = CompileLayer.findSymbol(Name, false))
//            return Sym;
//          else if (auto Err = Sym.takeError())
//            return std::move(Err);
//          if (auto SymAddr =
//                  RTDyldMemoryManager::getSymbolAddressInProcess(Name))
//            return JITSymbol(SymAddr, JITSymbolFlags::Exported);
//          return nullptr;
//        },
//        [](Error Err) { cantFail(std::move(Err), "lookupFlags failed"); });
//
//    // Add the module to the JIT with the new key.
//    cantFail(CODLayer.addModule(K, std::move(M)));
//    return K;
//  }
//
//  JITSymbol findSymbol(const std::string Name) {
//    std::string MangledName;
//    raw_string_ostream MangledNameStream(MangledName);
//    Mangler::getNameWithPrefix(MangledNameStream, Name, DL);
//    return CODLayer.findSymbol(MangledNameStream.str(), true);
//  }
//
//  void removeModule(VModuleKey K) {
//    cantFail(CODLayer.removeModule(K));
//  }
//
//private:
//  std::unique_ptr<Module> optimizeModule(std::unique_ptr<Module> M) {
//    // Create a function pass manager.
//    auto FPM = std::make_unique<legacy::FunctionPassManager>(M.get());
//
//    // Add some optimizations.
//    FPM->add(createInstructionCombiningPass());
//    FPM->add(createReassociatePass());
//    FPM->add(createGVNPass());
//    FPM->add(createCFGSimplificationPass());
//    FPM->doInitialization();
//
//    // Run the optimizations over all functions in the module being added to
//    // the JIT.
//    for (auto &F : *M)
//      FPM->run(F);
//
//    return M;
//  }
//};

class JITMachine {
private:
  ExecutionSession ES;
  RTDyldObjectLinkingLayer ObjectLayer;
  IRCompileLayer CompileLayer;

  DataLayout DL;
  MangleAndInterner Mangle;
  ThreadSafeContext Ctx;

  JITDylib &MainJD;

public:
  JITMachine(JITTargetMachineBuilder JTMB, DataLayout DL)
      : ObjectLayer(ES,
                    []() { return std::make_unique<SectionMemoryManager>(); }),
        CompileLayer(ES, ObjectLayer,
                     std::make_unique<ConcurrentIRCompiler>(std::move(JTMB))),
        DL(std::move(DL)), Mangle(ES, this->DL),
        Ctx(std::make_unique<LLVMContext>()),
        MainJD(ES.createJITDylib("<main>")) {
    MainJD.addGenerator(
        cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(
            DL.getGlobalPrefix())));
  }

  static Expected<std::unique_ptr<JITMachine>> Create() {
    auto JTMB = JITTargetMachineBuilder::detectHost();

    if (!JTMB)
      return JTMB.takeError();

    auto DL = JTMB->getDefaultDataLayoutForTarget();
    if (!DL)
      return DL.takeError();

    return std::make_unique<JITMachine>(std::move(*JTMB), std::move(*DL));
  }

  const DataLayout &getDataLayout() const { return DL; }

  LLVMContext &getContext() { return *Ctx.getContext(); }

  Error addModule(std::unique_ptr<Module> M) {
    return CompileLayer.add(MainJD, ThreadSafeModule(std::move(M), Ctx));
  }

  Expected<JITEvaluatedSymbol> lookup(StringRef Name) {
    return ES.lookup({&MainJD}, Mangle(Name.str()));
  }
};

std::unique_ptr<Module>
compileIR (LLVMContext &ctx, std::string IR)
{
    SMDiagnostic errhandler;
    std::unique_ptr<MemoryBuffer> IRbuff = MemoryBuffer::getMemBuffer(IR);
    auto m = parseIR(*IRbuff, errhandler, ctx);
    if (!m) {
        errhandler.print("JIT", errs());
        exit(1);
    }
    return m;
}

int
main (int argc, char **argv)
{
    LLVMContext context;

    // only initialize the native target
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    // Create two seperate IR modules which reference one another but are
    // compiled separately
    std::string mainIR = 
        "declare i32 @foo()\n"
        "define i32 @main() {\n"
        "\t%result = call i32 @foo()\n"
        "\tret i32 %result\n"
        "}";
    std::string fooIR = 
        "define i32 @foo() {\n"
        "\tret i32 33\n"
        "}";
    //std::string redefineIR = 
    //    "define i32 @foo() {\n"
    //    "\tret i32 72\n"
    //    "}";
    auto main_module = compileIR(context, mainIR);
    auto foo_module = compileIR(context, fooIR);

    std::unique_ptr<JITMachine> JIT = cantFail(JITMachine::Create());
    auto err1 = JIT->addModule(std::move(main_module));
    auto err2 = JIT->addModule(std::move(foo_module));

    auto main = JIT->lookup("main");
    if (!main) {
        errs() << "Couldn't find symbol!\n";
        return 1;
    }
    FunctionEntry entry = (FunctionEntry) (intptr_t) main->getAddress();
    outs() << "main returns: " << entry(0, NULL) << "\n";

    return 0;
}
