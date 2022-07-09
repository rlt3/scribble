#ifndef SCRIBBLE_JIT
#define SCRIBBLE_JIT

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

class Procedure;

using namespace llvm;
using namespace llvm::orc;

typedef int (*FunctionEntry) ();

/*
 * The JIT handles the execution of the IR that is generated. It also owns all
 * the internal references of native procedures and the control-flow graph of
 * them.
 */
class JIT
{
protected:
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

    LLVMContext context;
    std::string tmp;

public:
    JIT ()
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

    static void
    preinitialize ()
    {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmParser();
        llvm::InitializeNativeTargetAsmPrinter();
    }

    /*
     * Create a function definition in the JIT using the given string as a name
     * which executes the given procedure when called. If that definition
     * already exists, then update it instead.
     */
    void
    createOrUpdateProcedure (std::string name, Procedure proc)
    {
    }

    void
    addIR (IR ir)
    {
        auto m = compileIR(ir.getString());
        /*auto k = */addModule(std::move(m));
    }

    /*
     * The entry point in the JIT isn't a hard address, but the internal name
     * for the latest version of the procedure, e.g. the high-level name of a
     * procedure may be `foobar', but after three revisions, the current
     * internal name may be `foobar_3_void_int`.
     */
    std::string&
    getProcedureEntry (std::string name)
    {
        return tmp;
    }

    /*
     * Execute the given procedure. This procedure is not meant to be defined
     * for future use.
     */
    void
    executeProcedure (Procedure &proc)
    {
        std::string ir = 
            "@stack = external global [4096 x i64]\n"
            "@top = external global [4096 x i64]*\n"
            + proc.getIRString();
        auto m = compileIR(ir);
        auto k = addModule(std::move(m));

        auto main = findSymbol("main");
        if (!main) {
            errs() << "Couldn't find symbol!\n";
            return;
        }

        auto entry = (FunctionEntry) (intptr_t) cantFail(main.getAddress());
        outs() << "proc returns: " << entry() << "\n";

        removeModule(k);
    }

    unsigned long*
    getStack ()
    {
        auto stack = findSymbol("stack");
        if (!stack) {
            errs() << "Couldn't find stack!\n";
            return NULL;
        }

        return (unsigned long*) cantFail(stack.getAddress());
    }

private:
    VModuleKey
    addModule (std::unique_ptr<Module> M)
    {
        // Add the module to the JIT with a new VModuleKey.
        auto K = ES.allocateVModule();
        cantFail(OptimizeLayer.addModule(K, std::move(M)));
        return K;
    }

    void
    removeModule (VModuleKey K)
    {
        cantFail(OptimizeLayer.removeModule(K));
    }

    JITSymbol
    findSymbol(const std::string Name)
    {
        return OptimizeLayer.findSymbol(mangle(Name), true);
    }

    std::unique_ptr<Module>
    compileIR (std::string IR)
    {
        SMDiagnostic errhandler;
        printf("IR:\n%s\n", IR.c_str());
        std::unique_ptr<MemoryBuffer> IRbuff = MemoryBuffer::getMemBuffer(IR);
        auto m = parseIR(*IRbuff, errhandler, context);
        if (!m) {
            errhandler.print("JIT", errs());
            exit(1);
        }
        return m;
    }

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

#endif
