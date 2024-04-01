#ifndef HABANEROMCASMINFO_H
#define HABANEROMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {

class Triple;

struct HabaneroELFMCAsmInfo : public MCAsmInfoELF {
  explicit HabaneroELFMCAsmInfo(const Triple &TT);
};

} // namespace llvm

#endif // HABANEROMCASMINFO_H
