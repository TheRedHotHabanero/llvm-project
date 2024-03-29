// Habanero target descriptions

#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

#include "Habanero.h"
#include "HabaneroMCTargetDesc.h"
#include "MCTargetDesc/HabaneroInfo.h"
#include "TargetInfo/HabaneroTargetInfo.h"

using namespace llvm;

#define GET_REGINFO_MC_DESC
#include "HabaneroGenRegisterInfo.inc"

#define GET_INSTRINFO_MC_DESC
#include "HabaneroGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "HabaneroGenSubtargetInfo.inc"

static MCRegisterInfo *createHabaneroMCRegisterInfo(const Triple &TT) {
  HABANERO_DUMP_LOCATION();

  MCRegisterInfo *reg_info = new MCRegisterInfo();
  InitHabaneroMCRegisterInfo(reg_info, Habanero::R0);
  return reg_info;
}

static MCInstrInfo *createHabaneroMCInstrInfo() {
  HABANERO_DUMP_LOCATION();

  MCInstrInfo *instr_info = new MCInstrInfo();
  InitHabaneroMCInstrInfo(instr_info);
  return instr_info;
}

static MCSubtargetInfo *createHabaneroMCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU, StringRef FS) {
  HABANERO_DUMP_LOCATION();

  return createHabaneroMCSubtargetInfoImpl(TT, CPU, /* TuneCPU */ CPU, FS);
}

// We need to define this function for linking succeed
extern "C" void LLVMInitializeHabaneroTargetMC() {
  HABANERO_DUMP_LOCATION();

  Target &habanero_target = getHabaneroTarget();

  // Register MC register info.
  TargetRegistry::RegisterMCRegInfo(habanero_target, createHabaneroMCRegisterInfo);
  // Register MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(habanero_target, createHabaneroMCInstrInfo);
  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(habanero_target,
                                          createHabaneroMCSubtargetInfo);
}
