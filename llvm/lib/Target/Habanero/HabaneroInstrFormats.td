// Defines Habanero instruction formats

// General Habanero instruction format
//
// FF FF FF FF
// ^^          - Opcode
//    ^        - R1
//     ^       - R2 or Imm4
//       ^^ ^^ - R3 or Imm16

class HabaneroInstr<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern>: Instruction
{
    // Inst and Size: for tablegen(... -gen-emitter) and
    // tablegen(... -gen-disassembler) in CMakeLists.txt
    let Namespace = "Habanero";
    let DecoderNamespace = "Habanero";
    let Size = 4;

    field bits<32> Inst;

    bits<8> Opcode = op;
    bits<4>  r1 = 0;
    bits<4>  r2_imm4 = 0;
    bits<16> r3_imm16 = 0;

    let Inst{31-24} = Opcode;
    let Inst{23-20} = r1;
    let Inst{19-16} = r2_imm4;
    let Inst{15-0}  = r3_imm16;

    let OutOperandList = outs;
    let InOperandList  = ins;
    let AsmString   = asmstr;
    let Pattern     = pattern;
}

class HabaneroPseudoInstr<dag outs, dag ins, string asmstr, list<dag> pattern>: HabaneroInstr<0, outs, ins, asmstr, pattern> {
    let isPseudo = 1;
    let isCodeGenOnly = 1;
}
