#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "Habanero.h"
#include "HabaneroRegisterInfo.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "HabaneroGenRegisterInfo.inc"

HabaneroRegisterInfo::HabaneroRegisterInfo() : HabaneroGenRegisterInfo(Habanero::R0) {
  HABANERO_DUMP_LOCATION();
}

const MCPhysReg *
HabaneroRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  // From HabaneroCallingConv.td
  return CSR_Habanero_SaveList;
}

BitVector HabaneroRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  const HabaneroFrameLowering *FrameLowering = getFrameLowering(MF);

  BitVector Reserved(getNumRegs());

  Reserved.set(Habanero::R0); // RA
  Reserved.set(Habanero::R1); // SP

  if (FrameLowering->hasFP(MF)) {
    Reserved.set(Habanero::R2); // FP
  }

  return Reserved;
}

bool HabaneroRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected non-zero SPAdj value");

  MachineInstr &Instr = *II;
  MachineFunction &Func = *Instr.getParent()->getParent();

  // Calculate offset
  int FrameIndex = Instr.getOperand(FIOperandNum).getIndex();
  Register FrameReg{};
  int Offset = getFrameLowering(Func)
                   ->getFrameIndexReference(Func, FrameIndex, FrameReg)
                   .getFixed();
  Offset += Instr.getOperand(FIOperandNum + 1).getImm();

  // Not supported offset
  if (!isInt<16>(Offset)) {
    llvm_unreachable("");
  }

  Instr.getOperand(FIOperandNum).ChangeToRegister(FrameReg, false);
  Instr.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);

  return false;
}

Register HabaneroRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return getFrameLowering(MF)->hasFP(MF) ? Habanero::R2 : Habanero::R1; // FP or SP
}
