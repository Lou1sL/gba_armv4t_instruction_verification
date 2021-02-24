
//--------------------------------------------------------------
// Move Shifted Register(LSL, LSR, ASR)
//--------------------------------------------------------------
//These instructions move a shifted value between Lo registers.
//All instructions in this group set the CPSR condition codes.
template <int op, int offset5>
void ARM7TDMI::ThumbMoveShiftedRegister(ThumbInstruction instruction)                    {
    int carry = registers.cpsr.c;
    std::uint32_t result = registers[instruction.msr.rs];
    switch (op){
        case 0b00: LSL(result, offset5 & 0xFF, carry, false, false);   break;
        case 0b01: LSR(result, offset5 & 0xFF, carry, false, false);   break;
        case 0b10: ASR(result, offset5 & 0xFF, carry, false, false);   break;
        case 0b11: /** TODO: Check if there is ROR in real hardware */ break;
    }
    registers.cpsr.c = carry;
    SRNZ(result);
    registers[instruction.msr.rd] = result;
    PipelineFetch16(true);
}

//--------------------------------------------------------------
//Add/Subtract(ADD, SUB)
//--------------------------------------------------------------
//These instructions allow the contents of a Lo register or a 3-bit immediate value to be added to or subtracted from a Lo register. 
//All instructions in this group set the CPSR condition codes.
template <int i, int op, int rn>
void ARM7TDMI::ThumbAddAndSubtract(ThumbInstruction instruction)                         {
    if (op == 0) 
        ADD(registers[instruction.as.rs], 
            i ? rn : registers[rn], 
            instruction.as.rd, 
            true);
    else
        SUB(registers[instruction.as.rs], 
            i ? rn : registers[rn], 
            instruction.as.rd, 
            true);
    PipelineFetch16(true);
}

//--------------------------------------------------------------
//Move/Compare/Add/Subtract Immediate(MOV, CMP, ADD, SUB)
//--------------------------------------------------------------
//The instructions in this group perform operations between a Lo register and an 8-bit immediate value.
//All instructions in this group set the CPSR condition codes.
template <int op, int rd>
void ARM7TDMI::ThumbMoveCompareAddAndSubtractImme(ThumbInstruction instruction)          {
    switch (op){
        case 0b00: MOV(            0, instruction.mcasi.offset8, rd, true); break;
        case 0b01: SUB(registers[rd], instruction.mcasi.offset8, -1, true); break;
        case 0b10: ADD(registers[rd], instruction.mcasi.offset8, rd, true); break;
        case 0b11: SUB(registers[rd], instruction.mcasi.offset8, rd, true); break;
    }
    PipelineFetch16(true);
}

//--------------------------------------------------------------
//ALU Operations(AND, EOR, LSL, LSR, ASR, ADC, SBC, ROR, TST, NEG, CMP, CMN, ORR, MUL, BIC, MVN)
//--------------------------------------------------------------
//All instructions in this group set the CPSR condition codes
template <int op>
void ARM7TDMI::ThumbALUOperation(ThumbInstruction instruction)                           {
    std::uint32_t rd       = instruction.aluo.rd;
    int carry = registers.cpsr.c;
    //LSL, LSR, ASR, ROR are all Register Specified so need 1I
    switch (op){
        case TMB_AND:              AND(registers[instruction.aluo.rd], registers[instruction.aluo.rs], rd, true); break;
        case TMB_EOR:              EOR(registers[instruction.aluo.rd], registers[instruction.aluo.rs], rd, true); break;
        case TMB_LSL: bus->ICyc(); LSL(registers[instruction.aluo.rd], registers[instruction.aluo.rs], carry, true, true); break;
        case TMB_LSR: bus->ICyc(); LSR(registers[instruction.aluo.rd], registers[instruction.aluo.rs], carry, true, true); break;
        case TMB_ASR: bus->ICyc(); ASR(registers[instruction.aluo.rd], registers[instruction.aluo.rs], carry, true, true); break;
        case TMB_ADC:              ADC(registers[instruction.aluo.rd], registers[instruction.aluo.rs], rd, true); break;
        case TMB_SBC:              SBC(registers[instruction.aluo.rd], registers[instruction.aluo.rs], rd, true); break;
        case TMB_ROR: bus->ICyc(); ROR(registers[instruction.aluo.rd], registers[instruction.aluo.rs], carry, true, true); break;
        case TMB_TST:              AND(registers[instruction.aluo.rd], registers[instruction.aluo.rs], -1, true); break;
        case TMB_NEG:              SUB(                             0, registers[instruction.aluo.rs], rd, true); break;
        case TMB_CMP:              SUB(registers[instruction.aluo.rd], registers[instruction.aluo.rs], -1, true); break;
        case TMB_CMN:              ADD(registers[instruction.aluo.rd], registers[instruction.aluo.rs], -1, true); break;
        case TMB_ORR:              ORR(registers[instruction.aluo.rd], registers[instruction.aluo.rs], rd, true); break;
        //TODO CYC: ARM7TDMI-S Data Sheet 4-23
        case TMB_MUL:              registers[instruction.aluo.rd] *= registers[instruction.aluo.rs]; carry = 0; SRNZ(registers[instruction.aluo.rd]); break;
        case TMB_BIC:              BIC(registers[instruction.aluo.rd], registers[instruction.aluo.rs], rd, true); break;
        case TMB_MVN:              MVN(                             0, registers[instruction.aluo.rs], rd, true); break;
    }
    registers.cpsr.c = carry;
    PipelineFetch16(true);
}

//--------------------------------------------------------------
//High RegisterOperations/Branch Exchange(ADD, CMP, MOV, BX)
//--------------------------------------------------------------
//There are four sets of instructions in this group.
//The ﬁrst three allow ADD, CMP and MOV operations to be performed between Lo and Hi registers, or a pair of Hi registers.
//The fourth, BX, allows a Branch to be performed which may also be used to switch processor state.
template <int op, int h1, int h2>
void ARM7TDMI::ThumbHighRegisterOperationsAndBranchExchange(ThumbInstruction instruction){
    //h1: 0 -> rd(r0-r7), 1 -> hd(r8-r15)
    std::uint32_t rdhd = instruction.hrobe.rdhd | (h1 ? 0b1000 : 0);
    //h1: 0 -> rs(r0-r7), 1 -> hs(r8-r15)
    std::uint32_t rshs = instruction.hrobe.rshs | (h2 ? 0b1000 : 0);
    std::uint32_t operand = registers[rshs];

    //If R15 is used as an operand, 
    //   the value will be the address of the instruction + 4 with bit 0 cleared.
    //Executing a BX PC in THUMB state from a non-word aligned address will result in unpredictable execution
    if (rshs == 15) operand &= ~1;

    switch (op){
        case 0b00:
            //ARMDataProcessing ADD
            registers[rdhd] += operand;
            if (rdhd == 15) {
                registers[15] &= ~1;
                PipelineReload16();
            } else {
                PipelineFetch16(true);
            }
        break;
        case 0b01:
            //ARMDataProcessing CMP
            SUB(registers[rdhd], operand, -1, true);
            PipelineFetch16(true);
        break;
        case 0b10:
            //ARMDataProcessing MOV
            registers[rdhd] = operand;
            if (rdhd == 15) {
                registers[15] &= ~1;
                PipelineReload16();
            } else {
                PipelineFetch16(true);
            }
        break;
        case 0b11:
            //ARMBranchAndExchange BX
            //Bit 0 of the address determines the processor state on entry to the routine.
            if (operand & 1) {
                registers[15] = operand & ~0b01;
                registers.cpsr.t = 1;
                PipelineReload16();
            } else {
                registers[15] = operand & ~0b11;
                registers.cpsr.t = 0;
                PipelineReload32();
            }
        break;
    }
}

//--------------------------------------------------------------
// PC-relative Load(LDR)
//--------------------------------------------------------------
//This instruction loads a word from an address speciﬁed as a 10-bit immediate offset from the PC.
template <int rd>
void ARM7TDMI::ThumbPCRelativeLoad(ThumbInstruction instruction)                         {
    //The value speciﬁed by #Imm is a full 10-bit address, but must always be word-aligned (ie with bits 1:0 set to 0), 
    //   since the assembler places #Imm >> 2 in ﬁeld Word8.
    //Note The value of the PC will be 4 bytes greater than the address of this instruction, 
    //   but bit 1 of the PC is forced to 0 to ensure it is word aligned.
    registers[rd] = bus->R32((registers[15] & ~2) + (instruction.pcrl.word8 << 2), false);
    bus->ICyc();
    PipelineFetch16(false);
}

//--------------------------------------------------------------
//Load/Store With Register Offset(STR, STRB, LDR, LDRB)
//--------------------------------------------------------------
//These instructions transfer byte or word values between registers and memory.
//Memory addresses are pre-indexed using an offset register in the range 0-7.
template <int l, int b, int ro>
void ARM7TDMI::ThumbLoadAndStoreRegisterOffset(ThumbInstruction instruction)             {
    auto addr = registers[instruction.lsro.rb] + registers[ro];
    if(l == 0){
        //Store to memory
        if(b == 0){
            //Transfer word quantity
            bus->W32(addr, registers[instruction.lsro.rd], false);
        }else{
            //Transfer byte quantity
            bus->W8(addr, registers[instruction.lsro.rd], false);
        }
    }else{
        //Load from memory
        if(b == 0){
            //Transfer word quantity
            auto value = bus->R32(addr, false);
            auto shift = (addr & 3) * 8;
            registers[instruction.lsro.rd] = (value >> shift) | (value << (32 - shift));
        }else{
            //Transfer byte quantity
            registers[instruction.lsro.rd] = bus->R8(addr, false);
        }
        bus->ICyc();
    }
    PipelineFetch16(false);
}

//--------------------------------------------------------------
//Load/Store Sign-extended Byte/Halfword(STRH, LDRH, LDSB, LDSH)
//--------------------------------------------------------------
//These instructions load optionally sign-extended bytes or halfwords, and store halfwords.
template <int h, int s, int ro>
void ARM7TDMI::ThumbLoadAndStoreSignExtendedByteAndHalfword(ThumbInstruction instruction){
    auto addr = registers[instruction.lsro.rb] + registers[ro];
    if(s == 0){
        if(h == 0){
            //Store halfword(STRH)
            //Add Ro to base address in Rb. Store bits 0-15 of Rd at the resulting address.
            bus->W16(addr, registers[instruction.lsro.rd], false);
        }else{
            //Load halfword(LDRH)
            //Add Ro to base address in Rb. Load bits 0-15 of Rd from the resulting address, and set bits 16-31 of Rd to 0.
            auto value = static_cast<std::uint32_t>(bus->R16(addr, false));
            if(addr & 1) value = (value >> 8) | (value << 24);
            registers[instruction.lsro.rd] = value;
            bus->ICyc();
        }
    }else{
        if(h == 0){
            //Load sign-extended byte(LDSB)
            //Add Ro to base address in Rb. Load bits 0-7 of Rd from the resulting address, and set bits 8-31 of Rd to bit 7.
            auto value = static_cast<std::uint32_t>(bus->R8(addr, false));
            value |= (value & 0x80) ? 0xFFFFFF00 : 0;
            registers[instruction.lsro.rd] = value;
        }else{
            //Load sign-extended halfword(LDSH)
            //Add Ro to base address in Rb. Load bits 0-15 of Rd from the resulting address, and set bits 16-31 of Rd to bit 15.
            std::uint32_t value;
            if (addr & 1) {
                value = bus->R8(addr, false);
                value |= (value & 0x80) ? 0xFFFFFF00 : 0;
            } else {
                value = bus->R16(addr, false);
                value |= (value & 0x8000) ? 0xFFFF0000 : 0;
            }
            registers[instruction.lsro.rd] = value;
        }
        bus->ICyc();
    }
    PipelineFetch16(false);
}

//--------------------------------------------------------------
//Load/Store With Immediate Offset(STR, STRB, LDR, LDRB)
//--------------------------------------------------------------
//These instructions transfer byte or word values between registers and memory using an immediate 5 or 7-bit offset.
template <int b, int l, int offset5>
void ARM7TDMI::ThumbLoadAndStoreImmediateOffset(ThumbInstruction instruction)            {
    //Note
    //For word accesses (B = 0), the value speciﬁed by #Imm is a full 7-bit address,
    //   but must be word-aligned (ie with bits 1:0 set to 0), 
    //   since the assembler places #Imm >> 2 in the Offset5 ﬁeld
    if(l == 0){
        if(b == 0){
            //STR Rd, [Rb, #Imm]
            //Calculate the target address by adding together the value in Rb and Imm. Store the contents of Rd at the address.
            std::uint32_t addr = registers[instruction.lsio.rb] + (static_cast<std::uint32_t>(offset5) << 2);
            bus->W32(addr, registers[instruction.lsio.rd], false);
        }else{
            //STRB Rd, [Rb, #Imm]
            //Calculate the target address by adding together the value in Rb and Imm. Store the byte value in Rd at the address.
            bus->W8(registers[instruction.lsio.rb] + offset5, registers[instruction.lsio.rd], false);
        }
    }else{
        if(b == 0){
            //LDR Rd, [Rb, #Imm]
            //Calculate the source address by adding together the value in Rb and Imm. Load Rd from the address.
            std::uint32_t addr = registers[instruction.lsio.rb] + (static_cast<std::uint32_t>(offset5) << 2);
            auto value = bus->R32(addr, false);
            auto shift = (addr & 3) * 8;
            registers[instruction.lsio.rd] = (value >> shift) | (value << (32 - shift));
        }else{
            //LDRB Rd, [Rb, #Imm]
            //Calculate source address by adding together the value in Rb and Imm. Load the byte value at the address into Rd.
            registers[instruction.lsio.rd] = bus->R8(registers[instruction.lsio.rb] + offset5, false);
        }
        bus->ICyc();
    }
    PipelineFetch16(false);
}

//--------------------------------------------------------------
//Load/Store Halfword(STRH, LDRH)
//--------------------------------------------------------------
//These instructions transfer halfword values between a Lo register and memory.
//Addresses are pre-indexed, using a 6-bit immediate value.
template <int l, int offset5>
void ARM7TDMI::ThumbLoadAndStoreHalfword(ThumbInstruction instruction)                   {
    std::uint32_t addr = registers[instruction.lsh.rb] + (static_cast<std::uint32_t>(offset5) << 1);
    //Note
    //#Imm is a full 6-bit address but must be halfword-aligned (ie with bit 0 set to 0)
    //   since the assembler places #Imm >> 1 in the Offset5 ﬁeld.
    if(l == 0){
        //Store to memory
        bus->W16(addr, registers[instruction.lsh.rd], false);
    }else{
        //Load from memory
        auto value = bus->R16(addr, false);
        if (addr & 1) value = (value >> 8) | (value << 24);
        registers[instruction.lsh.rd] = value;
        bus->ICyc();
    }
    PipelineFetch16(false);
}

//--------------------------------------------------------------
//SP-relative Load/Store(STR Rd, [SP, #Imm], LDR  Rd, [SP, #Imm])
//--------------------------------------------------------------
//The instructions in this group perform an SP-relative load or store.The THUMB assembler syntax is shown in the following table.
template <int l, int rd>
void ARM7TDMI::ThumbSPRelativeLoadAndStore(ThumbInstruction instruction)                 {
    //Note
    //The offset supplied in #Imm is a full 10-bit address, 
    //   but must always be word-aligned (ie bits 1:0 set to 0), 
    //   since the assembler places #Imm >> 2 in the Word8 ﬁeld.
    std::uint32_t addr = registers[13] + (static_cast<std::uint32_t>(instruction.sprls.word8) << 2);
    if(l == 0){
        //STR Rd, [SP #Imm]
        //Add unsigned offset (255 words, 1020 bytes) in Imm to the current value of the SP (R7).
        //Store the contents of Rd at the resulting address.
        bus->W32(addr, registers[rd], false);
    }else{
        //LDR Rd, [SP, #Imm]
        //Add unsigned offset (255 words, 1020 bytes) in Imm to the current value of the SP (R7).
        //Load the word from the resulting address into Rd.
        auto value = bus->R32(addr, false);
        auto shift = (addr & 3) * 8;
        registers[rd] = (value >> shift) | (value << (32 - shift));
        bus->ICyc();
    }
    PipelineFetch16(false);
}

//--------------------------------------------------------------
//Load Address(ADD Rd, PC, #Imm, ADD Rd, SP, #Imm)
//--------------------------------------------------------------
//These instructions calculate an address by adding an 10-bit constant to either the PC or the SP, and load the resulting address into a register.
template <int sp, int rd>
void ARM7TDMI::ThumbLoadAddress(ThumbInstruction instruction)                            {
    //Note
    //The value speciﬁed by #Imm is a full 10-bit value, 
    //   but this must be word-aligned (ie with bits 1:0 set to 0) since the assembler places #Imm >> 2 in ﬁeld Word8.
    //Where the PC is used as the source register (SP = 0), bit 1 of the PC is always read as 0.
    //The value of the PC will be 4 bytes greater than the address of the instruction before bit 1 is forced to 0.
    //The CPSR condition codes are unaffected by these instructions.
    const std::uint32_t offset = static_cast<std::uint32_t>(instruction.la.word8) << 2;
    if(sp == 0){
        //ADD Rd, PC, #Imm
        //Add #Imm to the current value of the program counter (PC) and load the result into Rd.
        registers[rd] = (registers[15] & ~2) + offset;
    }else{
        //ADD Rd, SP, #Imm
        //Add #Imm to the current value of the stack pointer (SP) and load the result into Rd.
        registers[rd] = registers[13] + offset; 
    }
    PipelineFetch16(true);
}

//--------------------------------------------------------------
//Add Offset to Stack Pointer(ADD SP, #Imm, ADD SP, #-Imm)
//--------------------------------------------------------------
//This instruction adds a 9-bit signed constant to the stack pointer.
template <int s>
void ARM7TDMI::ThumbAddOffsetToStackPointer(ThumbInstruction instruction)                {
    //The offset speciﬁed by #Imm can be up to -/+ 508, 
    //   but must be word-aligned (ie with bits 1:0 set to 0) 
    //   since the assembler converts #Imm to an 8-bit sign + magnitude number before placing it in ﬁeld SWord7.
    //The condition codes are not set by this instruction
    const std::uint32_t offset = static_cast<std::uint32_t>(instruction.aosp.sword7) << 2;
    if(s == 0){
        //ADD SP, #Imm
        registers[13] += offset;
    }else{
        //ADD SP, #-Imm
        registers[13] -= offset;
    }
    PipelineFetch16(true);
}

//--------------------------------------------------------------
//Push/Pop Registers(PUSH, POP)
//--------------------------------------------------------------
//The instructions in this group allow registers 0-7 and optionally LR to be pushed onto the stack, 
//   and registers 0-7 and optionally PC to be popped off the stack.
//The stack is always assumed to be Full Descending.
//https://acjf3.github.io/papers/bdt.pdf
//https://mgba.io/2014/12/28/classic-nes/
template <int l, int r>
void ARM7TDMI::ThumbPushAndPopRegisters(ThumbInstruction instruction)                    {
    //Any subset of the registers, or all the registers, may be specified. The only restriction is that the register list should not be empty.
    //TODO: Empty reglist.
    auto sp = registers[13];
    bool isSeq = false;
    if(l == 0){
        //Push
        for (int i = 0; i <= 7; i++) {
           if (instruction.ppr.rlist & (1 << i)) sp -= 4;
        }
        if (r) sp -= 4;
        registers[13] = sp;
        for (int i = 0; i <= 7; i++) {
            if (instruction.ppr.rlist & (1 << i)) {
                bus->W32(sp, registers[i], isSeq);
                isSeq = true;
                sp += 4;
            }
        }
        //PUSH { Rlist }
        //Push the registers specified by Rlist onto the stack. Update the stack pointer.
        //PUSH { Rlist, LR }
        //Push the Link Register and the registers specified by Rlist (if any) onto the stack. Update the stack pointer.
        if (r) bus->W32(sp, registers[14], isSeq);
    }else{
        //Pop
        for (int i = 0; i <= 7; i++) {
            if (instruction.ppr.rlist & (1 << i)) {
                //The stack is always assumed to be Full Descending.
                registers[i] = bus->R32(sp, isSeq);
                isSeq = true;
                sp += 4;
            }
        }
        if (r) sp += 4;
        registers[13] = sp;
        bus->ICyc();
        if (r == 0) {
            //POP { Rlist }
            //Pop values off the stack into the registers specified by Rlist. Update the stack pointer.
        }else{
            //POP { Rlist, PC }
            //Pop values off the stack and load into the registers specified by Rlist. Pop the PC off the stack. Update the stack pointer.
            registers[15] = bus->R32(sp - 4, isSeq) & ~1;
            PipelineReload16();
            return;
        }
    }
    PipelineFetch16(false);
}

//--------------------------------------------------------------
//Multiple Load/Store(STMIA, LDMIA)
//--------------------------------------------------------------
//These instructions allow multiple loading and storing of Lo registers.
template <int l, int rb>
void ARM7TDMI::ThumbMultipleLoadAndStore(ThumbInstruction instruction)                   {

    int transfered_register_count = 0;
    int starting_register = 0;
    if(instruction.mls.rlist != 0){
        for(int i=7; i>=0; i--){
            if (~(instruction.mls.rlist & (1 << i))) continue;
            transfered_register_count++;
            starting_register = i;
        }
    }
    //Any subset of the registers, or all the registers, may be specified. The only restriction is that the register list should not be empty.
    //TODO: Empty reglist.
    
    //The registers are transferred in the order lowest to highest.
    //The lowest register also gets transferred to/from the lowest memory address.
    const std::uint32_t old_rb = registers[rb];
    const std::uint32_t new_rb = registers[rb] + transfered_register_count * 4;

    std::uint32_t address = old_rb;
    bool isSeq = false;
    for(int i=starting_register; i<8; i++){
        if (~(instruction.mls.rlist & (1 << i))) continue;
        if(l != 0){
            //Load
            registers[i] = bus->R32(address, isSeq);
            isSeq = true;
        }else{
            //Store
            //During a STM, the first register is written out at the start of the second cycle.
            //A STM which includes storing the base, with the base as the first register to be stored, 
            //   will therefore store the unchanged value,
            //   whereas with the base second or later in the transfer order, will store the modified value.
            if (i == rb && i == starting_register) bus->W32(address, old_rb, false);
            else if (i == rb && i != starting_register) bus->W32(address, new_rb, true);
            else bus->W32(address, registers[i], true);
        }
        address += 4;
    }
    if(l != 0) bus->ICyc();
    //When write-back is specified, the base is written back at the end of the second cycle of the instruction.
    //A LDM will always overwrite the updated base if the base is in the list. 
    const bool is_rb_inside_list = (instruction.mls.rlist & (1 << rb)) != 0;
    if((l != 0) && is_rb_inside_list){
        registers[rb] = new_rb;
    }
    PipelineFetch16(false);
}

//--------------------------------------------------------------
//Conditional Branch
//--------------------------------------------------------------
//The instructions in this group all perform a conditional Branch 
//   depending on the state of the CPSR condition codes. 
//The branch offset must take account of the prefetch operation, 
//   which causes the PC to be 1 word (4 bytes) ahead of the current instruction.
template <int cond>
void ARM7TDMI::ThumbConditionalBranch(ThumbInstruction instruction)                      {
    if(ReadCondition(static_cast<CONDITION>(cond))){
        std::uint32_t offset = instruction.cb.soffset8;
        if(offset & 0x80) offset |= 0xFFFFFF00;
        //While label speciﬁes a full 9-bit two’s complement address, 
        //   this must always be halfword-aligned (ie with bit 0 set to 0) 
        //   since the assembler actually places label >> 1 in ﬁeld SOffset8.
        //Cond = 1110 is undeﬁned, and should not be used.
        //Cond = 1111 creates the SWI instruction.(In decoding stage)
        registers[15] += offset << 1;
        PipelineReload16();
    }else{
        PipelineFetch16(true);
    }
}

//--------------------------------------------------------------
//Software Interrupt
//--------------------------------------------------------------
//The SWI instruction performs a software interrupt.
//On taking the SWI, the processor switches into ARM state and enters Supervisor (SVC) mode.
void ARM7TDMI::ThumbSoftwareInterrupt(ThumbInstruction instruction)                      {
    //Value8 is used solely by the SWI handler: it is ignored by the processor.
    //Perform Software Interrupt:
    //   Move the address of the next instruction into LR, 
    //      move CPSR to SPSR, load the SWI vector address (0x8) into the PC.
    //   Switch to ARM state and enter SVC mode.
    EXCEP_SWI();
}

//--------------------------------------------------------------
//Unconditional Branch
//--------------------------------------------------------------
//This instruction performs a PC-relative Branch.
//The branch offset must take account of the prefetch operation,
//   which causes the PC to be 1 word (4 bytes) ahead of the current instruction.
void ARM7TDMI::ThumbUnconditionalBranch(ThumbInstruction instruction)                    {
    //The address speciﬁed by label is a full 12-bit two’s complement address,
    //   but must always be halfword aligned (ie bit 0 set to 0), since the assembler places label >> 1 in the Offset11 ﬁeld.
    std::uint32_t offset = instruction.ub.offset11 << 1;
    if (offset & 0x400) offset |= 0xFFFFF800;
    registers[15] += offset;
    PipelineReload16();
}

//--------------------------------------------------------------
//Long Branch With Link(BL)
//--------------------------------------------------------------
//The assembler splits the 23-bit two’s complement half-word offset specifed by the label into two 11-bit halves, 
//   ignoring bit 0 (which must be 0), and creates two THUMB instructions.
template <int h>
void ARM7TDMI::ThumbLongBranchWithLink(ThumbInstruction instruction)                     {
    std::uint32_t offset = instruction.lbl.offset;
    if(h == 0){
        //Offset high
        //In the ﬁrst instruction the Offset ﬁeld contains the upper 11 bits of the target address.
        //This is shifted left by 12 bits and added to the current PC address.
        //The resulting address is placed in LR.
        offset <<= 12;
        if (offset & 0x400000) offset |= 0xFF800000;
        registers[14] = registers[15] + offset;
        PipelineFetch16(true);
    }else{
        //Offset low
        //In the second instruction the Offset ﬁeld contains an 11-bit representation lower half of the target address.
        //This is shifted left by 1 bit and added to LR. LR, which now contains the full 23-bit address, 
        //   is placed in PC, the address of the instruction following the BL is placed in LR and bit 0 of LR is set.
        std::uint32_t temp = registers[15] - 2;
        registers[15] = (registers[14] + (offset << 1)) & ~1;
        registers[14] = temp | 1;
        PipelineReload16();
    }
}

//--------------------------------------------------------------
//There is no such a thing called Undefined in Thumb, this is only a placeholder
//   for look up table element generated.
//--------------------------------------------------------------
void ARM7TDMI::ThumbUndefined(ThumbInstruction instruction)                              {
}