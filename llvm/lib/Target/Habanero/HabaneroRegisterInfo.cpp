#include "llvm/CodeGen/TargetInstrInfo.h"

#include "Habanero.h"
#include "HabaneroRegisterInfo.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "HabaneroGenRegisterInfo.inc"

HabaneroRegisterInfo::HabaneroRegisterInfo() : HabaneroGenRegisterInfo(Habanero::R0) {
  HABANERO_DUMP_LOCATION();
}
