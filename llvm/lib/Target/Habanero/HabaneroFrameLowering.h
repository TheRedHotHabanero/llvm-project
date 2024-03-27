#ifndef HABANEROFRAMELOWERING_H
#define HABANEROFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

#include "Habanero.h"

namespace llvm {

class HabaneroFrameLowering final : public TargetFrameLowering {
public:
  explicit HabaneroFrameLowering()
      : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(4), 0) {
    HABANERO_DUMP_LOCATION();
  }

  /// Insert prologue code into function
  void emitPrologue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override {}
  /// Insert epilogue code into function
  void emitEpilogue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override {}

  /// hasFP - Return true if the specified function should have a dedicated
  /// frame pointer register. For most targets this is true only if the function
  /// has variable sized allocas or if frame pointer elimination is disabled.
  bool hasFP(const MachineFunction &MF) const override { return false; }
};

} // namespace llvm

#endif // HABANEROFRAMELOWERING_H
