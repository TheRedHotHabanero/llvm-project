// Defines Habanero target

include "llvm/Target/Target.td"

include "HabaneroRegisterInfo.td"
include "HabaneroInstrInfo.td"

def HabaneroInstrInfo : InstrInfo;

def Habanero : Target {
    let InstructionSet = HabaneroInstrInfo;
}
