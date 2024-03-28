#ifndef HABANEROINFO_H
#define HABANEROINFO_H

#include "llvm/MC/MCInstrDesc.h"

namespace llvm::HabaneroOp {

enum OperandType : unsigned {
  OPERAND_HABANERO_IMM20 = MCOI::OPERAND_FIRST_TARGET
};

} // namespace llvm::HabaneroOp

#endif // HABANEROINFO_H
