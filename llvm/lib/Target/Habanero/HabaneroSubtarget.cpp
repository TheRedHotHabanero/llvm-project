#include "llvm/Target/TargetMachine.h"

#include "Habanero.h"
#include "HabaneroSubtarget.h"

using namespace llvm;

#define DEBUG_TYPE "habanero-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "HabaneroGenSubtargetInfo.inc"

HabaneroSubtarget::HabaneroSubtarget(const StringRef &CPU, const StringRef &TuneCPU,
                             const StringRef &FS, const TargetMachine &TM)
    : HabaneroGenSubtargetInfo(TM.getTargetTriple(), CPU, TuneCPU, FS) {
  HABANERO_DUMP_LOCATION();
}
