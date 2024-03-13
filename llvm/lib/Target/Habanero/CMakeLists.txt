add_llvm_component_group(Habanero)

add_llvm_target(HabaneroCodeGen
    HabaneroTargetMachine.cpp

    LINK_COMPONENTS
    CodeGen
    Core
    Support
    Target
    HabaneroInfo

    ADD_TO_COMPONENT
    Habanero
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
