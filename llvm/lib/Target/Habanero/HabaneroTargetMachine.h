#ifndef HABANEROTARGETMACHINE_H
#define HABANEROTARGETMACHINE_H

#include <optional>

#include "llvm/Target/TargetMachine.h"

#include "HabaneroInstrInfo.h"
#include "HabaneroSubtarget.h"

namespace llvm {

class HabaneroTargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  HabaneroSubtarget Subtarget;

public:
  HabaneroTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    std::optional<Reloc::Model> RM,
                    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                    bool JIT);

  const HabaneroSubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

} // namespace llvm

#endif // HABANEROTARGETMACHINE_H
