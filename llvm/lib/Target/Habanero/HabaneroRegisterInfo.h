#ifndef HABANEROREGISTERINFO_H
#define HABANEROREGISTERINFO_H

#include "HabaneroFrameLowering.h"

#define GET_REGINFO_HEADER
#include "HabaneroGenRegisterInfo.inc"

namespace llvm {

struct HabaneroRegisterInfo : public HabaneroGenRegisterInfo {
  HabaneroRegisterInfo();
};

} // end namespace llvm

#endif // HABANEROREGISTERINFO_H
