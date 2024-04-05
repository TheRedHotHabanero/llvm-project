#ifndef HABANEROREGISTERINFO_H
#define HABANEROREGISTERINFO_H

#include "HabaneroFrameLowering.h"

#define GET_REGINFO_HEADER
#include "HabaneroGenRegisterInfo.inc"

namespace llvm {

struct HabaneroRegisterInfo final : public HabaneroGenRegisterInfo {
  HabaneroRegisterInfo();

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  bool eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  // Debug information queries.
  Register getFrameRegister(const MachineFunction &MF) const override;
};

} // end namespace llvm

#endif // HABANEROREGISTERINFO_H
