#ifndef HABANEROINSTRINFO_H
#define HABANEROINSTRINFO_H

#include "llvm/CodeGen/TargetInstrInfo.h"

#include "HabaneroRegisterInfo.h"
#include "MCTargetDesc/HabaneroInfo.h"

#define GET_INSTRINFO_HEADER
#include "HabaneroGenInstrInfo.inc"

namespace llvm {

struct HabaneroInstrInfo : public HabaneroGenInstrInfo {
  HabaneroInstrInfo();
};

} // namespace llvm

#endif // HABANEROINSTRINFO_H
