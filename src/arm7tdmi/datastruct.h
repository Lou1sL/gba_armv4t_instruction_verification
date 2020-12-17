
union ARMInstruction {
    struct { const unsigned int oprand2:12, rd:4, rn:4, s:1, opcode:4, i:1, i27_26:2, cond:4; } dpsr;
    struct { const unsigned int rm:4, i7_4:4, rs:4, rn:4, rd:4, s:1, a:1, i27_22:6, cond:4; } mul;
    struct { const unsigned int rm:4, i7_4:4, rs:4, rdlo:4, rdhi:4, s:1, a:1, u:1, i27_23:5, cond:4; } mull;
    struct { const unsigned int rm:4, i11_4:8, rd:4, rn:4, i21_20:2, b:1, i27_23:5, cond:4; } sds;
    struct { const unsigned int rn:4, i27_4:24, cond:4; } be;
    struct { const unsigned int rm:4, i4:1, h:1, s:1, i11_7:5, rd:4, rn:4, l:1, w:1, i22:1, u:1, p:1, i27_25:3, cond:4; } hdtr;
    struct { const unsigned int offsetl:4, i4:1, h:1, s:1, offseth:5, rd:4, rn:4, l:1, w:1, i22:1, u:1, p:1, i27_25:3, cond:4; } hdti;
    struct { const unsigned int offset:12, rd:4, rn:4, l:1, w:1, b:1, u:1, p:1, i:1, i27_26:2, cond:4; } sdt;
    struct { const unsigned int i3_0:4, i4:1, i24_5:20, i27_25:3, cond:4; } unde;
    struct { const unsigned int reglist:16, rn:4, l:1, w:1, s:1, u:1, p:1, i27_25:3, cond:4; } bdt;
    struct { const unsigned int offset:24, l:1, i27_25:3, cond:4; } bra;
    struct { const unsigned int offset:8, cps:4, crd:4,  rn:4, l:1, w:1, n:1, u:1, p:1, i27_25:3, cond:4; } cdt;
    struct { const unsigned int crm:4, i4:1, cp:3, cps:4, crd:4, crn:4, cpopc:4 , i27_24:4, cond:4; } cdo;
    struct { const unsigned int crm:4, i4:1, cp:3, cps:4,  rd:4, crn:4, l:1, cpopc:3, i27_24:4, cond:4; } crt;
    struct { const unsigned int ign:24, i27_24:4, cond:4; } si;
    const std::uint32_t value;
    constexpr ARMInstruction(std::uint32_t v): value(v){}
};
union ThumbInstruction {
    struct { const unsigned int rd:3, rs:3, offset5:5, op:2, i15_13:3; } msr;
    struct { const unsigned int rd:3, rs:3, rn:3, op:1, i:1, i15_11:5; } as;
    struct { const unsigned int offset8:8, rd:3, op:2, i15_13:3; } mcasi;
    struct { const unsigned int rd:3, rs:3, op:4, i15_10:6; } aluo;
    struct { const unsigned int rdhd:3, rshs:3, h2:1, h1:1, op:2, i15_10:6; } hrobe;
    struct { const unsigned int word8:8, rd:3, i15_11:5; } pcrl;
    struct { const unsigned int rd:3, rb:3, ro:3, i9:1, b:1, l:1, i15_12:4; } lsro;
    struct { const unsigned int rd:3, rb:3, ro:3, i9:1, s:1, h:1, i15_12:4; } lssbh;
    struct { const unsigned int rd:3, rb:3, offset5:5, l:1, b:1, i15_13:3; } lsio;
    struct { const unsigned int rd:3, rb:3, offset5:5, l:1, i15_12:4; } lsh;
    struct { const unsigned int word8:8, rd:3, l:1, i15_12:4; } sprls;
    struct { const unsigned int word8:8, rd:3, sp:1, i15_12:4; } la;
    struct { const unsigned int sword7:7, s:1, i15_8:8; } aosp;
    struct { const unsigned int rlist:8, r:1, i10_9:2, l:1, i15_12:4; } ppr;
    struct { const unsigned int rlist:8, rb:3, l:1, i15_12:4; } mls;
    struct { const unsigned int soffset8:8, cond:4, i15_12:4; } cb;
    struct { const unsigned int value8:8, i15_8:8; } si;
    struct { const unsigned int offset11:11, i15_11:5; } ub;
    struct { const unsigned int offset:11, h:1, i15_10:4; } lbl;
    const std::uint32_t value;
    constexpr ThumbInstruction(std::uint32_t v): value(v){}
};

enum ARM_OPCODE {
    ARM_AND = 0b0000, ARM_EOR = 0b0001, ARM_SUB = 0b0010, ARM_RSB = 0b0011, ARM_ADD = 0b0100, ARM_ADC = 0b0101, ARM_SBC = 0b0110, ARM_RSC = 0b0111,
    ARM_TST = 0b1000, ARM_TEQ = 0b1001, ARM_CMP = 0b1010, ARM_CMN = 0b1011, ARM_ORR = 0b1100, ARM_MOV = 0b1101, ARM_BIC = 0b1110, ARM_MVN = 0b1111
};
enum THUMB_OPCODE {
    TMB_AND = 0b0000, TMB_EOR = 0b0001, TMB_LSL = 0b0010, TMB_LSR = 0b0011, TMB_ASR = 0b0100, TMB_ADC = 0b0101, TMB_SBC = 0b0110, TMB_ROR = 0b0111,
    TMB_TST = 0b1000, TMB_NEG = 0b1001, TMB_CMP = 0b1010, TMB_CMN = 0b1011, TMB_ORR = 0b1100, TMB_MUL = 0b1101, TMB_BIC = 0b1110, TMB_MVN = 0b1111
};
enum CONDITION {
    EQ = 0b0000, NE = 0b0001, CS = 0b0010, CC = 0b0011, MI = 0b0100, PL = 0b0101, VS = 0b0110, VC = 0b0111,
    HI = 0b1000, LS = 0b1001, GE = 0b1010, LT = 0b1011, GT = 0b1100, LE = 0b1101, AL = 0b1110, NV = 0b1111
};

enum MODE {
    MODE_USR = 0b10000, MODE_FIQ = 0b10001, MODE_IRQ = 0b10010, MODE_SVC = 0b10011, MODE_ABT = 0b10111, MODE_UND = 0b11011, MODE_SYS = 0b11111
};

constexpr std::size_t GENERAL_REG_SIZE = 16;
constexpr std::size_t BANK_COUNT = 6;
constexpr std::size_t BANK_SIZE = 7;
enum BANK {
    BANK_SYSUSR = 0, BANK_FIQ = 1, BANK_IRQ = 2, BANK_SVC = 3, BANK_ABT = 4, BANK_UND = 5
};
enum BANK_REG {
    BANK_R8 = 0, BANK_R9 = 1, BANK_R10 = 2, BANK_R11 = 3, BANK_R12 = 4, BANK_R13 = 5, BANK_R14 = 6
};