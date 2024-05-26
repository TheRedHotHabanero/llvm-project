#include "llvm/Target/TargetMachine.h"

#include "Habanero.h"
#include "HabaneroSubtarget.h"

using namespace llvm;

#define DEBUG_TYPE "habanero-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "HabaneroGenSubtargetInfo.inc"

HabaneroSubtarget::HabaneroSubtarget(const Triple &TT, const StringRef &CPU,
                             const StringRef &FS, const TargetMachine &TM)
    : HabaneroGenSubtargetInfo(TM.getTargetTriple(), CPU, /* TuneCPU = */ CPU, FS),
      InstrInfo(), FrameLowering(*this), TLInfo(TM, *this) {
  HABANERO_DUMP_LOCATION();
}
