// Defines Habanero target

include "llvm/Target/Target.td"

include "HabaneroRegisterInfo.td"
include "HabaneroInstrInfo.td"

class HabaneroProcessor<string Name, list<SubtargetFeature> Features>: Processor<Name, NoItineraries, Features>;

// General purpose Habanero impl
def : HabaneroProcessor<"HabaneroA", []>;

def HabaneroInstrInfo : InstrInfo;

def Habanero : Target {
    let InstructionSet = HabaneroInstrInfo;
}
