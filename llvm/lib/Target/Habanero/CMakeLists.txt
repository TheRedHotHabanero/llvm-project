add_llvm_component_group(Habanero)

set(LLVM_TARGET_DEFINITIONS Habanero.td)

tablegen(LLVM HabaneroGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM HabaneroGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM HabaneroGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM HabaneroGenCallingConv.inc -gen-callingconv)
tablegen(LLVM HabaneroGenDAGISel.inc -gen-dag-isel)

add_public_tablegen_target(HabaneroCommonTableGen)

add_llvm_target(HabaneroCodeGen
    HabaneroTargetMachine.cpp
    HabaneroRegisterInfo.cpp
    HabaneroInstrInfo.cpp
    HabaneroSubtarget.cpp
    HabaneroISelLowering.cpp
    HabaneroISelDagToDag.cpp

    LINK_COMPONENTS
    CodeGen
    Core
    Support
    Target
    HabaneroInfo
    SelectionDAG

    ADD_TO_COMPONENT
    Habanero
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
