#ifndef HABANEROTARGETSTREAMER_H
#define HABANEROTARGETSTREAMER_H

#include "llvm/MC/MCStreamer.h"

namespace llvm {

struct HabaneroTargetStreamer : public MCTargetStreamer {
  HabaneroTargetStreamer(MCStreamer &S);
};

} // end namespace llvm

#endif // HABANEROTARGETSTREAMER_H
