#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"

#include "Habanero.h"
#include "HabaneroInstrInfo.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "HabaneroGenInstrInfo.inc"

HabaneroInstrInfo::HabaneroInstrInfo() : HabaneroGenInstrInfo() { HABANERO_DUMP_LOCATION(); }
