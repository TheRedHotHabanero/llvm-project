add_llvm_component_library(LLVMHabaneroDesc
    HabaneroMCTargetDesc.cpp

    LINK_COMPONENTS
    MC
    Support
    HabaneroInfo

    ADD_TO_COMPONENT
    Habanero
)
