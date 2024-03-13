#include "TargetInfo/HabaneroTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

Target &llvm::getHabaneroTarget() {
  static Target HabaneroTarget;
  return HabaneroTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeHabaneroTargetInfo() {
  RegisterTarget<Triple::habanero> reg_habanero(getHabaneroTarget(), "habanero", "Habanero 32", "HABANERO");
}
