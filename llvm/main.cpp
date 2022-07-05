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

class JITMachine {
private:
    ExecutionSession ES;
    std::shared_ptr<SymbolResolver> Resolver;
    std::unique_ptr<TargetMachine> TM;
    const DataLayout DL;
    LegacyRTDyldObjectLinkingLayer ObjectLayer;
    LegacyIRCompileLayer<decltype(ObjectLayer), SimpleCompiler> CompileLayer;

    using OptimizeFunction =
      std::function<std::unique_ptr<Module>(std::unique_ptr<Module>)>;

    LegacyIRTransformLayer<decltype(CompileLayer), OptimizeFunction> OptimizeLayer;

    std::unique_ptr<JITCompileCallbackManager> CompileCallbackMgr;
    std::unique_ptr<IndirectStubsManager> IndirectStubsMgr;

public:
    JITMachine()
        : Resolver(createLegacyLookupResolver(ES,
            [this](const std::string &Name) -> JITSymbol {
                if (auto Sym = IndirectStubsMgr->findStub(Name, false))
                    return Sym;
                if (auto Sym = OptimizeLayer.findSymbol(Name, false))
                    return Sym;
                else if (auto Err = Sym.takeError())
                    return std::move(Err);
                if (auto SymAddr =
                        RTDyldMemoryManager::getSymbolAddressInProcess(Name))
                    return JITSymbol(SymAddr, JITSymbolFlags::Exported);
                return nullptr;
            },
            [](Error Err) { cantFail(std::move(Err), "lookupFlags failed"); }))
        , TM(EngineBuilder().selectTarget())
        , DL(TM->createDataLayout())
        , ObjectLayer(AcknowledgeORCv1Deprecation, ES,
              [this](VModuleKey K) {
                  return LegacyRTDyldObjectLinkingLayer::Resources{
                    std::make_shared<SectionMemoryManager>(), Resolver};
              })
        , CompileLayer(AcknowledgeORCv1Deprecation,
                ObjectLayer, SimpleCompiler(*TM))
        , OptimizeLayer(AcknowledgeORCv1Deprecation, CompileLayer,
              [this](std::unique_ptr<Module> M) {
                  return optimizeModule(std::move(M));
              })
        , CompileCallbackMgr(cantFail(
              orc::createLocalCompileCallbackManager(TM->getTargetTriple(), ES, 0)))
    {
        auto IndirectStubsMgrBuilder =
            orc::createLocalIndirectStubsManagerBuilder(TM->getTargetTriple());
        IndirectStubsMgr = IndirectStubsMgrBuilder();
        llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
    }

    TargetMachine&
    getTargetMachine()
    {
        return *TM;
    }

    VModuleKey
    addModule (std::unique_ptr<Module> M)
    {
        // Add the module to the JIT with a new VModuleKey.
        auto K = ES.allocateVModule();
        cantFail(OptimizeLayer.addModule(K, std::move(M)));
        return K;
    }

    JITSymbol
    findSymbol(const std::string Name)
    {
        return OptimizeLayer.findSymbol(mangle(Name), true);
    }

    void
    removeModule (VModuleKey K)
    {
        cantFail(OptimizeLayer.removeModule(K));
    }

private:
    std::string
    mangle (const std::string &Name)
    {
        std::string MangledName;
        raw_string_ostream MangledNameStream(MangledName);
        Mangler::getNameWithPrefix(MangledNameStream, Name, DL);
        return MangledNameStream.str();
    }

    std::unique_ptr<Module>
    optimizeModule (std::unique_ptr<Module> M)
    {
        // Create a function pass manager.
        auto FPM = std::make_unique<legacy::FunctionPassManager>(M.get());

        // Add some optimizations.
        FPM->add(createInstructionCombiningPass());
        FPM->add(createReassociatePass());
        FPM->add(createGVNPass());
        FPM->add(createCFGSimplificationPass());
        FPM->doInitialization();

        // Run the optimizations over all functions in the module being added
        // to the JIT.
        for (auto &F : *M)
            FPM->run(F);

        return M;
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

void
runMain (JITMachine &JIT)
{
    auto main = JIT.findSymbol("main");
    if (!main) {
        errs() << "Couldn't find symbol!\n";
        return;
    }
    FunctionEntry entry = (FunctionEntry) (intptr_t) cantFail(main.getAddress());
    outs() << "main returns: " << entry(0, NULL) << "\n";
}

extern "C" {
    void
    callme ()
    {
        printf("You did it. Good job!\n");
    }
}

/*
 * Functions in LLVM cannot be updated in place without significant effort.
 * For that reasons, I am simpling going to use a numbering scheme for
 * procedure versions. For example, if there's a procedure `double` and it gets
 * updated, you might have functions `double_1`, `double_2`, etc, in the JIT.
 * Because a particular function call be calling `double_1` when the new
 * version `double_2` is created, then callees need to be updated. This
 * cascades.
 *
 * To accomodate this design, I think every procedure holds its current IR as a
 * string, keys to each version of the procedure -- along with each version's
 * callees, and the current version string. Therefore, every newly compiled
 * call to a procedure goes to the latest version.
 *
 * TODO: Designing the ABI. We are compiling bytecode into LLVM IR, so this is
 * where primitives will be defined. Need to look into automatic inlining of
 * them as well.
 */
int
main (int argc, char **argv)
{
    LLVMContext context;

    /* JITMachine must be instantiated after these initializations */
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();
    JITMachine JIT;

    std::string selfIR =
        "declare void @callme()\n"
        "define i32 @main() {\n"
        "\tcall void @callme()\n"
        "\tret i32 72\n"
        "}";
    auto m = compileIR(context, selfIR);

    /* 
     * Create the function external `callme` function within the Module `m'.
     * We *must* have the binary linked dynamically so that this symbol may
     * be found.
     */
    FunctionType *type = FunctionType::get(Type::getVoidTy(context), false);
    Function::Create(type, Function::ExternalLinkage, "callme", m.get());

    auto k = JIT.addModule(std::move(m));
    runMain(JIT);

    //std::string mainIR =
    //    "declare i32 @foo()\n"
    //    "define i32 @main() {\n"
    //    "\t%result = call i32 @foo()\n"
    //    "\tret i32 %result\n"
    //    "}";
    //std::string fooIR =
    //    "define i32 @foo() {\n"
    //    "\tret i32 33\n"
    //    "}";
    //std::string redefineIR =
    //    "define i32 @foo() {\n"
    //    "\tret i32 72\n"
    //    "}";

    ///*
    // * We specify each module here because we can only use them once. These are
    // * unique_ptrs and thus we give ownership of them to the JIT when we add
    // * them.
    // */
    //auto main1_module = compileIR(context, mainIR);
    //auto main2_module = compileIR(context, mainIR);
    //auto foo1_module = compileIR(context, fooIR);
    //auto foo2_module = compileIR(context, redefineIR);

    ///* Define the original, return 33 */
    //auto k1 = JIT.addModule(std::move(main1_module));
    //auto k2 = JIT.addModule(std::move(foo1_module));
    //runMain(JIT);

    ///*
    // * `foo` has changed, so update it, attempting to runMain here would cause
    // * a segmentation fault.
    // */
    //JIT.removeModule(k2);
    //auto k3 = JIT.addModule(std::move(foo2_module));

    ///* now update the tree that points to `foo` */
    //JIT.removeModule(k1);
    //auto k4 = JIT.addModule(std::move(main2_module));
    //runMain(JIT);

    return 0;
}
