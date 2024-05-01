#ifndef HABANERO_H
#define HABANERO_H

#include <iostream>

#include "llvm/Support/raw_ostream.h"

#include "MCTargetDesc/HabaneroMCTargetDesc.h"

#define HABANERO_DUMP_LOCATION()                                                   \
  {                                                                            \
    llvm::errs().changeColor(llvm::raw_ostream::GREEN)                         \
        << "  Habanero loc: " << __func__ << "\n    " << __FILE__ << ":"           \
        << __LINE__ << "\n";                                                   \
    llvm::errs().changeColor(llvm::raw_ostream::WHITE);                        \
  }

namespace llvm {

class HabaneroTargetMachine;
class FunctionPass;

FunctionPass *createHabaneroISelDag(HabaneroTargetMachine &TM);

} // namespace llvm

#endif // HABANERO_H
