#ifndef HABANEROINFO_H
#define HABANEROINFO_H

#include "llvm/MC/MCInstrDesc.h"

namespace llvm::HabaneroOp {

enum OperandType : unsigned {
  OPERAND_SIMM16 = MCOI::OPERAND_FIRST_TARGET
};

} // namespace llvm::HabaneroOp

#endif // HABANEROINFO_H
