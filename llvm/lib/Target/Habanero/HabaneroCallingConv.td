// Define Habanero calling convension

// Return convension
def RetCC_Habanero: CallingConv<[
    CCIfType<[i32], CCAssignToReg<R15>>
]>;

// Args convension
def CC_Habanero: CallingConv<[
    // Promote i8 and i16 to i32
    CCIfType<[i8, i16], CCPromoteToType<i32>>,

    // Arguments allocations
    CCIfType<[i32], CCAssignToReg<[R8, R9, R10, R11]>>,
    CCIfType<[i32], CCAssignToStack<4, 4>>
]>

def CSR_Habanero: CalleeSavedRegs<(add R0, R1, /* skip FP */ R3, R4, R5, R6, R7)>;
