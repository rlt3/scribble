#include <iostream>
#include <fstream>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h" // llvm::errs
// TODO: handle 'llvm/MC/TargetRegistry.h' on different platforms
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/SmallVectorMemoryBuffer.h"

using namespace llvm;

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
        "define i64 @main() {\n"
        "\tret i64 72\n"
        "}";
    std::unique_ptr<MemoryBuffer> IRbuff = MemoryBuffer::getMemBuffer(basicIR);

    // create a Module from IR parsed
    std::unique_ptr<Module> module = parseIR(*IRbuff, errhandler, context);

    // get the native target
    auto target_triple = sys::getDefaultTargetTriple();
    auto target = TargetRegistry::lookupTarget(target_triple, errstr);
    if (!target) {
        errs() << errstr << "\n";
        return 1;
    }

    // and construct a target machine from the native target
    auto machine_cpu = "generic";
    auto machine_features = "";
    TargetOptions machine_options;
    auto machine_reloc = Optional<Reloc::Model>();
    auto target_machine = target->createTargetMachine(target_triple,
            machine_cpu, machine_features, machine_options, machine_reloc);

    // update our model with the specific features of the native target
    module->setTargetTriple(sys::getDefaultTargetTriple());
    module->setDataLayout(target_machine->createDataLayout());

    // create a buffer for our machine code
    SmallVector<char, 4096> objbuffer;
      raw_svector_ostream objstream(objbuffer);

    // create a pass manager, add the passes to emit machine code, and run them
    legacy::PassManager passes;
    MCContext *mccontext;
    if (target_machine->addPassesToEmitMC(passes, mccontext, objstream)) {
        errs() << "NativeTarget doesn't support emitting machine code!\n";
        return 1;
    }
    passes.run(*module);

    // flushes the objstream and initializes MemoryBuffer from objbuffer
    std::unique_ptr<MemoryBuffer> mcbuffer(
            new SmallVectorMemoryBuffer(std::move(objbuffer)));

    module->print(errs(), nullptr);

    // note: this binary isn't fully linked so it won't execute
    std::ofstream bin;
    bin.open("a.bin", std::ios::binary);
    bin.write(mcbuffer->getBufferStart(), mcbuffer->getBufferSize());
    bin.close();

    return 0;
}
