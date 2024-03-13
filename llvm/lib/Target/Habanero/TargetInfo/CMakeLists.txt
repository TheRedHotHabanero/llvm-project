add_llvm_component_library(LLVMHabaneroInfo
    HabaneroTargetInfo.cpp

    LINK_COMPONENTS
    MC
    Support

    ADD_TO_COMPONENT
    Habanero
)
