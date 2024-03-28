// Habanero target descriptions

#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/MC/MCInstrInfo.h"

#include "Habanero.h"
#include "TargetInfo/HabaneroTargetInfo.h"
#include "HabaneroMCTargetDesc.h"
#include "MCTargetDesc/HabaneroInfo.h"

using namespace llvm;

#define GET_REGINFO_MC_DESC
#include "HabaneroGenRegisterInfo.inc"

#define GET_INSTRINFO_MC_DESC
#include "HabaneroGenInstrInfo.inc"

static MCRegisterInfo *createHabaneroMCRegisterInfo(const Triple &t) {
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

// We need to define this function for linking succeed
extern "C" void LLVMInitializeHabaneroTargetMC() {
  HABANERO_DUMP_LOCATION();

  Target &habanero_target = getHabaneroTarget();

  // Register MC register info.
  TargetRegistry::RegisterMCRegInfo(habanero_target, createHabaneroMCRegisterInfo);
  // Register MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(habanero_target, createHabaneroMCInstrInfo);
}
