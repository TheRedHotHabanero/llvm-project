// Defines Habanero instruction info

// Profiles and nodes
def habanero_ret: SDNode<"HabaneroISD::RET", SDTNone, [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

// Include instr formats
include "HabaneroInstrFormats.td"

// Operand, patterns and transformations
class HabaneroImmAsmOperand<int width> : AsmOperandClass {
    let Name = "HabaneroImm" # width;
    let RenderMethod = "addImmOperands";
    let DiagnosticType = !strconcat("Invalid", Name);
}

// 16-bit signed imm
def simm16 : Operand<i32>, ImmLeaf<i32, [{return isInt<16>(Imm);}]> {
    let ParserMatchClass = HabaneroImmAsmOperand<16>;
    let EncoderMethod = "getImmOpValue";
    let DecoderMethod = "decodeHabaneroImmOperand<16>";
    let MCOperandPredicate = [{
        int64_t Imm;
        if (MCOp.evaluateAsConstantImm(Imm))
            return isInt<16>(Imm);
        return MCOp.isBareSymbolRef();
    }];
    let OperandType = "OPERAND_SIMM16";
    let OperandNamespace = "HabaneroOp";
}

// Instructions

// br
let hasSideEffects = 0, mayLoad = 0, mayStore = 0, isCall = 1, isBranch = 1, isTerminator = 1 in
def BR : HabaneroInstr<0x01, (outs), (ins GPR:$rs1), "BR $rs1", [(brind GPR:$rs1)]>;

// ret
let isBarrier = 1, isReturn = 1, isTerminator = 1 in
def PseudoRET : HabaneroPseudoInstr<(outs), (ins), "# PseudoRET", [(habanero_ret)]>,
                PseudoInstExpansion<(BR R0)>;

// movli
let isAsCheapAsAMove = 1, isCodeGenOnly = 1, isReMaterializable = 1 in
def MOVLI : HabaneroInstr<0x02, (outs GPR:$rd), (ins simm16:$imm),
                "MOVli $rd $imm", [(set GPR:$rd, simm16:$imm)]>;
