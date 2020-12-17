
//Data Processing Shift Decoding
template <int i>
inline void ARM7TDMI::ARMDataProcessingShiftDecoder(std::uint32_t& operand2, int& carry){
    if (i == 0){
        //operand 2 is a register
        std::uint32_t shift = (operand2 & 0xFF0) >> 4;
        std::uint32_t rm    = (operand2 & 0x00F) >> 0;

        std::uint32_t reg_specified = (shift & 0b00000001) >> 0;
        std::uint32_t shift_type    = (shift & 0b00000110) >> 1;
        std::uint32_t shift_amount  = (shift & 0b11111000) >> 3;
        std::uint32_t shift_reg     = (shift & 0b11110000) >> 4;
        
        //The PC value will be the address of the instruction, plus 8 or 12 bytes due to instruction prefetching.
        //If the shift amount is specified in the instruction, the PC will be 8 bytes ahead. 
        //If a register is used to specify the shift amount the PC will be 12 bytes ahead.
        operand2 = registers[rm];
        if(reg_specified){
            bus->ICyc();
            if(rm == 15) operand2 += 4;
        }
        switch (shift_type) {
            case 0: LSL(operand2, (reg_specified ? registers[shift_reg] : shift_amount) & 0xFF, carry, reg_specified, false); break;
            case 1: LSR(operand2, (reg_specified ? registers[shift_reg] : shift_amount) & 0xFF, carry, reg_specified, false); break;
            case 2: ASR(operand2, (reg_specified ? registers[shift_reg] : shift_amount) & 0xFF, carry, reg_specified, false); break;
            case 3: ROR(operand2, (reg_specified ? registers[shift_reg] : shift_amount) & 0xFF, carry, reg_specified, false); break;
        }
    }else{
        //operand 2 is an immediate value
        //The immediate operand rotate field is a 4 bit unsigned integer
        //   which specifies a shift operation on the 8 bit immediate value.
        //This value is zero extended to 32 bits, and then subject to a rotate right by twice the value in the rotate field.
        //This enables many common constants to be generated, for example all powers of 2
        std::uint32_t rotate = ((operand2 & 0xF00) >> 8) * 2;
        std::uint32_t imm    = (operand2 & 0x0FF) >> 0;

        if(rotate == 0) operand2 = imm;
        carry = (imm >> (rotate - 1)) & 1;
        operand2 = (imm >> rotate) | (imm << (32 - rotate)); 
    }
}

//--------------------------------------------------------------
//Data Processing(AND, EOR, SUB, RSB, ADD, ADC, SBC, RSC, TST, TEQ, CMP, CMN, ORR, MOV, BIC, MVN)
//--------------------------------------------------------------
//The instruction produces a result by performing a specified arithmetic or logical operation on one or two operands. 
//The first operand is always a register (Rn). 
//The second operand may be a shifted register (Rm) or a rotated 8 bit immediate value (Imm) according to the value of the I bit in the instruction. 
//The condition codes in the CPSR may be preserved or updated as a result of this instruction, 
//   according to the value of the S bit in the instruction. 
//Certain operations (TST, TEQ, CMP, CMN) do not write the result to Rd.
//They are used only to perform tests and to set the condition codes on the result and always have the S bit set.
template <int i, int opcode, int s, int i7_4>
void ARM7TDMI::ARMDataProcessing(ARMInstruction instruction)      {
    bool set_flags = s;
    int rd = instruction.dpsr.rd;
    int rn = instruction.dpsr.rn;

    std::uint32_t operand1 = registers[rn];
    std::uint32_t operand2 = instruction.dpsr.oprand2;
    
    int carry = registers.cpsr.c;
    
    //If R15 (the PC) is used as an operand in a data processing instruction the register is used directly.
    //The PC value will be the address of the instruction, plus 8 or 12 bytes due to instruction prefetching.
    //If the shift amount is specified in the instruction, the PC will be 8 bytes ahead.
    //If a register is used to specify the shift amount the PC will be 12 bytes ahead.
    if((i == 0) && ((operand2 & 0x10) != 0) && (rn == 15)) operand1 += 4;

    ARMDataProcessingShiftDecoder<i>(operand2, carry);

    if (rd == 15 && set_flags) {
        registers.SwitchToMode(static_cast<MODE>((*registers.p_spsr).m));
        registers.cpsr.value = (*registers.p_spsr).value;
        set_flags = false;
    }

    switch (static_cast<ARM_OPCODE>(opcode)) {
        case ARM_AND: AND(operand1, operand2, rd, set_flags); if(set_flags) registers.cpsr.c = carry; break;
        case ARM_EOR: EOR(operand1, operand2, rd, set_flags); if(set_flags) registers.cpsr.c = carry; break;
        case ARM_SUB: SUB(operand1, operand2, rd, set_flags); break;
        case ARM_RSB: SUB(operand2, operand1, rd, set_flags); break;
        case ARM_ADD: ADD(operand1, operand2, rd, set_flags); break;
        case ARM_ADC: ADC(operand1, operand2, rd, set_flags); break;
        case ARM_SBC: SBC(operand1, operand2, rd, set_flags); break;
        case ARM_RSC: SBC(operand2, operand1, rd, set_flags); break;
        case ARM_TST: AND(operand1, operand2, -1,      true); registers.cpsr.c = carry; break;
        case ARM_TEQ: EOR(operand1, operand2, -1,      true); registers.cpsr.c = carry; break;
        case ARM_CMP: SUB(operand1, operand2, -1,      true); break;
        case ARM_CMN: ADD(operand1, operand2, -1,      true); break;
        case ARM_ORR: ORR(operand1, operand2, rd, set_flags); if(set_flags) registers.cpsr.c = carry; break;
        case ARM_MOV: MOV(       0, operand2, rd, set_flags); if(set_flags) registers.cpsr.c = carry; break;
        case ARM_BIC: BIC(operand1, operand2, rd, set_flags); if(set_flags) registers.cpsr.c = carry; break;
        case ARM_MVN: MVN(       0, operand2, rd, set_flags); if(set_flags) registers.cpsr.c = carry; break;
    }

    if (rd == 15) {
        if (registers.cpsr.t) PipelineReload16();
        else PipelineReload32();
    } else {
        if (registers.cpsr.t) PipelineFetch16(true);
        else PipelineFetch32(true);
    }
}

//--------------------------------------------------------------
//PSR Transfer (MRS, MSR)
//--------------------------------------------------------------
//The MRS and MSR instructions are formed from a subset of the Data Processing operations 
//   and are implemented using the TEQ, TST, CMN and CMP instructions without the S flag set.
//These instructions allow access to the CPSR and SPSR registers.
//The MRS instruction allows the contents of the CPSR or SPSR_<mode> to be moved to a general register.
//The MSR instruction allows the contents of a general register to be moved to the CPSR or SPSR_<mode> register.
//The MSR instruction also allows an immediate value or register contents to be transferred 
//   to the condition code flags (N,Z,C and V) of CPSR or SPSR_<mode> without affecting the control bits.
//In this case, the top four bits of the specified register contents or 32 bit immediate value 
//   are written to the top four bits of the relevant PSR.
template <int i, int p, int i21>
void ARM7TDMI::ARMPSRTransfer(ARMInstruction instruction)         {
    if(i21 == 0){
        //mrs
        registers[instruction.dpsr.rd] = (p == 0) ? registers.cpsr.value : registers.p_spsr->value;
    }
    else{
        //msr
        const std::uint32_t rm     = instruction.dpsr.oprand2 & 0x00F;
        //The immediate operand rotate field is a 4 bit unsigned integer 
        //   which specifies a shift operation on the 8 bit immediate value. 
        //This value is zero extended to 32 bits, and then subject to a rotate right by twice the value in the rotate field.
        //This enables many common constants to be generated, for example all powers of 2
        const std::uint32_t rotate = ((instruction.dpsr.oprand2 & 0xF00) >> 8) * 2;
        const std::uint32_t imm    =  instruction.dpsr.oprand2 & 0x0FF;

        std::uint32_t value = (i == 0) ?
            registers[rm] : 
            ((imm >> rotate) | (imm << (32 - rotate)));

        std::uint32_t mask = 0;
        if (instruction.value & (1 << 16)) mask |= 0x000000FF;
        if (instruction.value & (1 << 17)) mask |= 0x0000FF00;
        if (instruction.value & (1 << 18)) mask |= 0x00FF0000;
        if (instruction.value & (1 << 19)) mask |= 0xFF000000;
        value &= mask;

        if (p == 0) {
            if(mask & 0xFF) registers.SwitchToMode(static_cast<MODE>(value & 0b11111));
            //Note that the software must never change the state of the T bit in the CPSR.
            //If this happens, the processor will enter an unpredictable state.
            registers.cpsr = (registers.cpsr.value & ~mask) | value;
        } else if (registers.p_spsr != &registers.cpsr) {
            registers.p_spsr->value = (registers.p_spsr->value & ~mask) | value;
        }
    }
    if (registers.cpsr.t) PipelineFetch16(true);
    else PipelineFetch32(true);
}

//--------------------------------------------------------------
//Multiply and Multiply-Accumulate (MUL, MLA)
//--------------------------------------------------------------
//The multiply and multiply-accumulate instructions use an 8 bit Booth’s algorithm to perform integer multiplication.
template <int a, int s>
void ARM7TDMI::ARMMultiply(ARMInstruction instruction)            {
    //The destination register Rd must not be the same as the operand register Rm.
    //R15 must not be used as an operand or as the destination register.
    std::uint32_t res = 
        registers[instruction.mul.rm] * registers[instruction.mul.rs];
    
    //The multiply form of the instruction gives Rd:=Rm*Rs.
    //Rn is ignored, and should be set to zero for compatibility with possible future upgrades to the instruction set. 
    //The multiply-accumulate form gives Rd:=Rm*Rs+Rn, which can save an explicit ADD instruction in some circumstances. 
    if(a) { res += registers[instruction.mul.rn]; bus->ICyc(); }
    //The C (Carry) flag is set to a meaningless value and the V (oVerflow) flag is unaffected. 
    if(s) SRNZ(res);
    registers[instruction.mul.rd] = res;
    //TODO CYC: ARM7TDMI-S Data Sheet 4-23
    PipelineFetch32(true);
}

//--------------------------------------------------------------
//Multiply Long and Multiply-Accumulate Long (MULL,MLAL)
//--------------------------------------------------------------
//The multiply long instructions perform integer multiplication on two 32 bit operands and produce 64 bit results.
//Signed and unsigned multiplication each with optional accumulate give rise to four variations.
template <int u, int a, int s>
void ARM7TDMI::ARMMultiplyLong(ARMInstruction instruction)        {
    int rdhi = instruction.mull.rdhi;
    int rdlo = instruction.mull.rdlo;
    const int rs = instruction.mull.rs;
    const int rm = instruction.mull.rm;

    std::int64_t result = u ? (
        //The SMULL and SMLAL instructions treat all of their operands as two’s-complement signed numbers
        //   and write a two’scomplement signed 64 bit result.
        ((std::uint64_t)registers[rm] | ((registers[rm] & 0x80000000) ? 0xFFFFFFFF00000000 : 0x00)) *
        ((std::uint64_t)registers[rs] | ((registers[rs] & 0x80000000) ? 0xFFFFFFFF00000000 : 0x00))
    ) : (
        //The UMULL and UMLAL instructions treat all of their operands as unsigned binary numbers 
        //   and write an unsigned 64 bit result
        (std::int64_t)registers[rm] * (std::int64_t)registers[rs]
    );
    bus->ICyc();

    //The multiply-accumulate forms (UMLAL and SMLAL) take two 32 bit numbers,
    //   multiply them and add a 64 bit number to produce a 64 bit result of the form 
    //   RdHi,RdLo := Rm * Rs + RdHi,RdLo.
    //The lower 32 bits of the 64 bit number to add is read from RdLo.
    //The upper 32 bits of the 64 bit number to add is read from RdHi.
    //The lower 32 bits of the 64 bit result are written to RdLo.
    //The upper 32 bits of the 64 bit result are written to RdHi. 
    if (a) { result += (((std::int64_t)registers[rdhi] << 32) | registers[rdlo]); bus->ICyc(); }

    std::uint64_t uresult = result;
    registers[rdlo] = uresult & 0xFFFFFFFF;
    registers[rdhi] = uresult >> 32;

    if (s) SRNZ64(uresult);
    //TODO CYC: ARM7TDMI-S Data Sheet 4-25
    PipelineFetch32(true);
}

//--------------------------------------------------------------
//Single Data Swap (SWP)
//--------------------------------------------------------------
//The data swap instruction is used to swap a byte or word quantity between a register and external memory.
//This instruction is implemented as a memory read followed by a memory write which are “locked” together 
//   (the processor cannot be interrupted until both operations have completed, and the memory manager is warned to treat them as inseparable). 
//This class of instruction is particularly useful for implementing software semaphores.
template <int b>
void ARM7TDMI::ARMSingleDataSwap(ARMInstruction instruction)      {
    //The swap address is determined by the contents of the base register (Rn).
    //The processor first reads the contents of the swap address.
    //Then it writes the contents of the source register (Rm) to the swap address, 
    //   and stores the old memory contents in the destination register (Rd).
    //The same register may be specified as both the source and destination
    //Do not use R15 as an operand (Rd, Rn or Rs) in a SWP instruction. 
    std::uint32_t xxx;
    if (b) {
        //Swap a byte
        xxx = bus->R8(registers[instruction.sds.rn], false);
        bus->W8(registers[instruction.sds.rn], (std::uint8_t)registers[instruction.sds.rm], false);
    } else {
        //Swap a word
        //The SWP instruction is implemented as a LDR followed by a STR 
        //   and the action of these is as described in the section on single data transfers.
        //In particular, the description of Big and Little Endian configuration applies to the SWP instruction. 
        xxx = bus->R32(registers[instruction.sds.rn], false);
        auto shift = (registers[instruction.sds.rn] & 0b11) * 8;
        xxx = (xxx >> shift) | (xxx << (32 - shift)); 
        bus->W32(registers[instruction.sds.rn], registers[instruction.sds.rm], false);
    }
    registers[instruction.sds.rd] = xxx;
    bus->ICyc();
    PipelineFetch32(true);
}

//--------------------------------------------------------------
//Branch and Exchange (BX)
//--------------------------------------------------------------
//This instruction performs a branch by copying the contents of a general register, Rn, into the program counter, PC. 
//The branch causes a pipeline flush and refill from the address specified by Rn.
//This instruction also permits the instruction set to be exchanged.
//When the instruction is executed, the value of Rn[0] determines whether the instruction stream will be decoded as ARM or THUMB instructions.
//If bit 0 of Rn = 1, subsequent instructions decoded as THUMB instructions
//If bit 0 of Rn = 0, subsequent instructions decoded as ARM instructions 
void ARM7TDMI::ARMBranchAndExchange(ARMInstruction instruction)   {
    std::uint32_t addr = registers[instruction.be.rn];
    if (addr & 1) {
        registers[15] = addr & ~1;
        registers.cpsr.t = 1;
        PipelineReload16();
    } else {
        registers[15] = addr & ~3;
        registers.cpsr.t = 0;
        PipelineReload32();
    }
}

//--------------------------------------------------------------
//Halfword and Signed Data Transfer (LDRH, STRH, LDRSB, LDRSH)
//--------------------------------------------------------------
//These instructions are used to load or store half-words of data and also load sign-extended bytes or half-words of data.
//The memory address used in the transfer is calculated by adding an offset to or subtracting an offset from a base register.
//The result of this calculation may be written back into the base register if auto-indexing is required.
template <int p, int u, int i22, int w, int l, int s, int h>
void ARM7TDMI::ARMHalfwordDataTransfer(ARMInstruction instruction){
    std::uint32_t addr = registers[instruction.hdtr.rn];
    std::uint32_t offset = i22 ?
        //Halfword and signed data transfer with immediate offset
        (instruction.hdti.offseth << 4) | instruction.hdti.offsetl : 
        //Halfword and signed data transfer with register offset
        registers[instruction.hdtr.rm];

    //Pre/Post indexing
    //pre: add/subtract offset before transfer
    //The offset may be added to (U=1) or subtracted from (U=0) the base register Rn
    if(p) addr += u ? offset : -offset;

    //01 = Unsigned halfwords
    //Setting S=0 and H=1 may be used to transfer unsigned Half-words between an ARM7TDMI-S register and memory.
    if(s == 0 && h == 1){
        if(l){
            bus->ICyc();
            //Little endian configuration
            //A halfword load (LDRSH or LDRH) expects data on data bus inputs 15 through to 0 
            //   if the supplied address is on a word boundary and on data bus inputs 31 through to 16 if it is a halfword boundary, (A[1]=1).
            //The supplied address should always be on a halfword boundary.
            //If bit 0 of the supplied address is HIGH, the ARM7TDMI-S will load an unpredictable value.
            //The selected halfword is placed in the bottom 16 bits of the destination register.
            //For unsigned half-words (LDRH), the top 16 bits of the register are filled with zeros 
            //   and for signed half-words (LDRSH) the top 16 bits are filled with the sign bit, bit 15 of the halfword.
            std::uint32_t val = bus->R16(addr, false);
            if ((addr & 1) != 0) { /** val = (val >> 8) | (val << 24); TODO: An unpredictable value? */ }
            registers[instruction.hdtr.rd] = val;
        }
        //When R15 is the source register (Rd) of a Half-word store (STRH) instruction, 
        //   the stored address will be address of the instruction plus 12.
        else bus->W16(addr, registers[instruction.hdtr.rd] + instruction.hdtr.rd == 15 ? 4 : 0, false);
    }
    //10 = Signed byte
    //The S bit controls the loading of sign-extended data.
    //When S=1 the H bit selects between Bytes (H=0) and Half-words (H=1).
    //The L bit should not be set low (Store) when Signed (S=1) operations have been selected. 
    else if(s == 1 && h == 0){
        if(l){
            bus->ICyc();
            //Little endian configuration
            //A signed byte load (LDRSB) expects data on data bus inputs 7 through to 0 if the supplied address is on a word boundary, 
            //   on data bus inputs 15 through to 8 if it is a word address plus one byte, and so on. 
            //The selected byte is placed in the bottom 8 bit of the destination register, 
            //   and the remaining bits of the register are filled with the sign bit, bit 7 of the byte.
            std::uint32_t val = bus->R8(addr, false);
            registers[instruction.hdtr.rd] = val & 0x80 ? val | 0xFFFFFF00 : val;
        }
        else /** TODO:Illegal operaiton */ ;
    }
    //Signed halfwords
    else if(s == 1 && h == 1){
        if(l){
            bus->ICyc();
            //Little endian configuration
            //A halfword load (LDRSH or LDRH) expects data on data bus inputs 15 through to 0 
            //   if the supplied address is on a word boundary and on data bus inputs 31 through to 16 if it is a halfword boundary, (A[1]=1).
            //The supplied address should always be on a halfword boundary.
            //If bit 0 of the supplied address is HIGH, the ARM7TDMI-S will load an unpredictable value.
            //The selected halfword is placed in the bottom 16 bits of the destination register.
            //For unsigned half-words (LDRH), the top 16 bits of the register are filled with zeros 
            //   and for signed half-words (LDRSH) the top 16 bits are filled with the sign bit, bit 15 of the halfword. 
            //TODO:Test
            std::uint32_t val = bus->R16(addr, false);
            if ((addr & 1) != 0) { /** val = bus->R8(addr, false); if (val & 0x0080) val |= 0xFFFFFF00; TODO: An unpredictable value? */ }
            registers[instruction.hdtr.rd] = val & 0x8000 ? val | 0xFFFF0000 : val;
        }
        else /** TODO:Illegal operaiton */ ;
    }

    if (
        //The modified base value may be written back into the base (W=1), or the old base may be kept (W=0).
        //In the case of post-indexed addressing, the write back bit is redundant and is always set to zero, 
        //   since the old base value can be retained if necessary by setting the offset to zero. 
        //Therefore post-indexed data transfers always write back the modified base. 
        //The Write-back bit should not be set high (W=1) when post-indexed addressing(P=0) is selected. 
        (w || !p) && 
        //You can't load to and write back to the same register 
        (!l || instruction.hdtr.rn != instruction.hdtr.rd)
    ){
        //Pre/Post indexing
        //post: add/subtract offset after transfer
        //The offset may be added to (U=1) or subtracted from (U=0) the base register Rn
        if (!p) addr += u ? offset : -offset;
        registers[instruction.hdtr.rn] = addr;
    }

    if (l && instruction.hdtr.rd == 15) PipelineReload32();
    else PipelineFetch32(static_cast<bool>(l));
}

//--------------------------------------------------------------
//Single Data Transfer (LDR, STR)
//--------------------------------------------------------------
//The single data transfer instructions are used to load or store single bytes or words of data.
//The memory address used in the transfer is calculated by adding an offset to or subtracting an offset from a base register.
//The result of this calculation may be written back into the base register if auto-indexing is required.
template <int i, int p, int u, int b, int w, int l, int i7_4>
void ARM7TDMI::ARMSingleDataTransfer(ARMInstruction instruction)  {
    std::uint32_t addr        = registers[instruction.sdt.rn];
    std::uint32_t shift       = (instruction.sdt.offset & 0xFF0) >> 4;
    std::uint32_t rm          = (instruction.sdt.offset & 0x00F) >> 0;
    //The register specified shift amounts are not available in this instruction class. 
    //std::uint32_t reg_specified = (shift & 0b00000001) >> 0;
    std::uint32_t shift_type    = (shift & 0b00000110) >> 1;
    std::uint32_t shift_amount  = (shift & 0b11111000) >> 3;
    //std::uint32_t shift_reg     = (shift & 0b11110000) >> 4;

    //In the case of post-indexed addressing, the write back bit is redundant and must be set to zero, 
    //   since the old base value can be retained by setting the offset to zero.
    //Therefore post-indexed data transfers always write back the modified base.
    //The only use of the W bit in a post-indexed data transfer is in privileged mode code, 
    //   where setting the W bit forces non-privileged mode for the transfer, 
    //   allowing the operating system to generate a user address in a system where the memory management hardware makes suitable use of this hardware.
    MODE previousMode;
    if (p == 0 && w != 0) {
        previousMode = static_cast<MODE>(registers.cpsr.m);
        registers.SwitchToMode(MODE_USR);
    }
    std::uint32_t offset;
    if (i == 0) {
        //0 = offset is an immediate value
        offset = instruction.sdt.offset;
    } else {
        //1 = offset is a register
        int carry = registers.cpsr.c;
        offset = registers[rm];
        switch (shift_type) {
            case 0: LSL(offset, shift_amount & 0xFF, carry, false, false); break;
            case 1: LSR(offset, shift_amount & 0xFF, carry, false, false); break;
            case 2: ASR(offset, shift_amount & 0xFF, carry, false, false); break;
            case 3: ROR(offset, shift_amount & 0xFF, carry, false, false); break;
        }
    }

    //1 = pre; add offset before transfer
    if (p != 0) addr += u ? offset : -offset;

    if (l != 0) {
        //LDR
        //0 = transfer word quantity, 1 = transfer byte quantity 
        std::uint32_t val;
        if(b != 0){
            val = bus->R8(addr, false);
        }
        else{
            //A word load (LDR) will normally use a word aligned address.
            //However, an address offset from a word boundary will cause the data to be rotated 
            //   into the register so that the addressed byte occupies bits 0 to 7. 
            //This means that half-words accessed at offsets 0 and 2 from the word boundary will be 
            //   correctly loaded into bits 0 through 15 of the register.
            //Two shift operations are then required to clear or to sign extend the upper 16 bits. 
            val = bus->R32(addr, false);
            auto shift = (addr & 0b11) * 8;
            val = (val >> shift) | (val << (32 - shift)); 
        }
        registers[instruction.sdt.rd] = val;
        bus->ICyc();
    } else {
        //STR
        std::uint32_t value = registers[instruction.sdt.rd];
        //When R15 is the source register (Rd) of a register store (STR) instruction, the stored value will be address of the instruction plus 12. 
        if (instruction.sdt.rd == 15) value += 4;
        //0 = transfer word quantity, 1 = transfer byte quantity 
        if (b != 0) bus->W8(addr, (std::uint8_t)value, false);
        //A word store (STR) should generate a word aligned address.
        //The word presented to the data bus is not affected if the address is not word aligned.
        //That is, bit 31 of the register being stored always appears on data bus output 31.
        else bus->W32(addr, value, false);
    }
    
    if (p == 0 && w != 0) registers.SwitchToMode(previousMode);

    if ((l == 0) || instruction.sdt.rn != instruction.sdt.rd) {
        //0 = post; add offset after transfer
        if (p == 0) registers[instruction.sdt.rn] += u ? offset : -offset;
        //Pre, but write back
        if (p != 0 && w != 0) registers[instruction.sdt.rn] = addr;
    }
    if ((l != 0) && instruction.sdt.rd == 15) PipelineReload32();
    else PipelineFetch32(static_cast<bool>(l));
}

//--------------------------------------------------------------
//Undefined Instruction
//--------------------------------------------------------------
//The undefined instruction trap mechanism is provided to trap usage of currently unused instruction encodings.
//This mechanism can be used to provide backwards compatibility by allowing emulation of instructions included in future architectures.
//Operating systems can also make use of this mechanism to set instruction breakpoints, 
//   which will return control to the operating system through the undefined instruction trap mechanism. 
void ARM7TDMI::ARMUndefined(ARMInstruction instruction)           {
    EXCEP_UND();
}

//--------------------------------------------------------------
//Block Data Transfer (LDM, STM)
//--------------------------------------------------------------
//Block data transfer instructions are used to load (LDM) or store (STM) any subset of the currently visible registers.
//They support all possible stacking modes, maintaining full or empty stacks which can grow up or down memory, 
//   and are very efficient instructions for saving or restoring context, or for moving large blocks of data around main memory.
//https://acjf3.github.io/papers/bdt.pdf
//https://mgba.io/2014/12/28/classic-nes/
template <int p, int u, int s, int w, int l, int i7_4>
void ARM7TDMI::ARMBlockDataTransfer(ARMInstruction instruction)   {
    //When the S bit is set in a LDM/STM instruction its meaning depends on whether or not R15 is in the transfer list and on the type of instruction.
    //The S bit should only be set if the instruction is to execute in a privileged mode. 
    //R15 not in list and S bit set (User bank transfer)
    //   For both LDM and STM instructions, the User bank registers are transferred rather than the register bank corresponding to the current mode.
    //   This is useful for saving the user state on process switches. Base write-back should not be used when this mechanism is employed. 
    //STM with R15 in transfer list and S bit set (User bank transfer)
    //   The registers transferred are taken from the User bank rather than the bank corresponding to the current mode.
    //   This is useful for saving the user state on process switches. Base write-back should not be used when this mechanism is employed. 
    MODE mode;
    bool is_r15_in_list = (instruction.bdt.reglist & (1 << 15)) != 0;
    bool is_stm_r15_and_s_set = l != 0 && is_r15_in_list && s;
    if (is_stm_r15_and_s_set) {
        mode = static_cast<MODE>(registers.cpsr.m);
        registers.SwitchToMode(MODE_USR);
    }

    int transfered_register_count = 0;
    int starting_register = 0;
    if(instruction.bdt.reglist != 0){
        for(int i=15; i>=0; i--){
            if ((~instruction.bdt.reglist) & (1 << i)) continue;
            transfered_register_count++;
            starting_register = i;
        }
    }
    //Any subset of the registers, or all the registers, may be specified. The only restriction is that the register list should not be empty.
    //TODO: Empty reglist.
    
    //The registers are transferred in the order lowest to highest, 
    //   so R15 (if in the list) will always be transferred last.
    //The lowest register also gets transferred to/from the lowest memory address.
    const std::uint32_t lo_addr = u != 0 ?
        //Increment addressing
        registers[instruction.bdt.rn] :
        //Decrement addressing
        registers[instruction.bdt.rn] - transfered_register_count * 4;

    const std::uint32_t hi_addr = u != 0 ?
        //Increment addressing
        registers[instruction.bdt.rn] + transfered_register_count * 4 :
        //Decrement addressing
        registers[instruction.bdt.rn];

    const std::uint32_t old_rn = registers[instruction.bdt.rn];
    const std::uint32_t new_rn = u != 0 ? hi_addr : lo_addr;

    std::uint32_t address = lo_addr;
    bool isSeq = false;
    for(int i=starting_register; i<16; i++){
        if ((~instruction.bdt.reglist) & (1 << i)) continue;

        if(
            //Pre-increment
            (p != 0 && u != 0) || 
            //Post-decrement
            (p == 0 && u == 0) 
        ) address += 4;
        
        if(l){
            //Load
            registers[i] = bus->R32(address, isSeq);
            //Use of the S bit:
            //LDM with R15 in transfer list and S bit set (Mode changes)
            //If the instruction is a LDM then SPSR_<mode> is transferred to CPSR at the same time as R15 is loaded.
            if (i == 15 && s) {
                auto& spsr = *registers.p_spsr;
                registers.SwitchToMode(static_cast<MODE>(spsr.m));
                registers.cpsr = spsr.value;
            }
        }else{
            //Store
            //During a STM, the first register is written out at the start of the second cycle.
            //A STM which includes storing the base, with the base as the first register to be stored, 
            //   will therefore store the unchanged value,
            //   whereas with the base second or later in the transfer order, will store the modified value.
            if (i == instruction.bdt.rn && i == starting_register) bus->W32(address, old_rn, isSeq);
            else if (i == instruction.bdt.rn && i != starting_register) bus->W32(address, new_rn, isSeq);
            //Whenever R15 is stored to memory the stored value is the address of the STM instruction plus 12.
            else if (i == 15) bus->W32(address, registers[15] + 4, isSeq);
            else bus->W32(address, registers[i], isSeq);
        }
        
        if(
            //Post-increment
            (p == 0 && u != 0) || 
            //Pre-decrement
            (p != 0 && u == 0) 
        ) address += 4;

        isSeq = true;
    }
    
    if (is_stm_r15_and_s_set) registers.SwitchToMode(mode);

    //When write-back is specified, the base is written back at the end of the second cycle of the instruction.
    //A LDM will always overwrite the updated base if the base is in the list. 
    //R15 should not be used as the base register in any LDM or STM instruction. 
    if (w){
        if((l == 0) || ((instruction.bdt.reglist & (1 << instruction.bdt.rn)) == 0)){
            registers[instruction.bdt.rn] = new_rn;
        }
    }

    if(l) bus->ICyc();


    if(l){
        if(is_r15_in_list){
            //PC has been modified
            if (registers.cpsr.t) PipelineReload16();
            else PipelineReload32();
        }else{
            PipelineFetch32(true);
        }
    }else{
        PipelineFetch32(false);
    }
}

//--------------------------------------------------------------
//Branch and Branch with Link (B, BL)
//--------------------------------------------------------------
//Branch instructions contain a signed 2’s complement 24 bit offset.
//This is shifted left two bits, sign extended to 32 bits, and added to the PC.
//The instruction can therefore specify a branch of +/- 32Mbytes.
//The branch offset must take account of the prefetch operation, which causes the PC to be 2 words (8 bytes) ahead of the current instruction.
//Branches beyond +/- 32Mbytes must use an offset or absolute destination which has been previously loaded into a register.
//In this case the PC should be manually saved in R14 if a Branch with Link type operation is required.
template <int l, int i23_20, int i7_4>
void ARM7TDMI::ARMBranch(ARMInstruction instruction)              {
    //Branch with Link (BL) writes the old PC into the link register (R14) of the current bank.
    //The PC value written into R14 is adjusted to allow for the prefetch, 
    //   and contains the address of the instruction following the branch and link instruction.
    //Note that the CPSR is not saved with the PC and R14[1:0] are always cleared. 
    if (l) registers[14] = registers[15] - 4;
    std::uint32_t offset = 
        ((i23_20 & 0b1000) == 0 ?
        (std::uint32_t)instruction.bra.offset : 
        ((std::uint32_t)instruction.bra.offset | 0xFF000000)) << 2;
    registers[15] += offset;
    PipelineReload32();
}

//--------------------------------------------------------------
//Software Interrupt (SWI)
//--------------------------------------------------------------
//The software interrupt instruction is used to enter Supervisor mode in a controlled manner.
//The instruction causes the software interrupt trap to be taken, which effects the mode change.
//The PC is then forced to a fixed value (0x08) and the CPSR is saved in SPSR_svc.
//If the SWI vector address is suitably protected (by external memory management hardware) 
//   from modification by the user, a fully protected operating system may be constructed. 
void ARM7TDMI::ARMSoftwareInterrupt(ARMInstruction instruction)   {
    EXCEP_SWI();
}