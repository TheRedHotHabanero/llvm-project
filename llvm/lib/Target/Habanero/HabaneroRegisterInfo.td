// Defines Habanero registers

class HabaneroReg<bits<16> enc, string name> : Register<name> {
    let HWEncoding = enc;
    let Namespace = "Habanero";
}

// Add GPRs
foreach i = 0-15 in {
    def R#i : HabaneroReg<i, "r"#i>;
}

def GPR : RegisterClass<"Habanero", [i32], 32,
    (add R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15)
>;
