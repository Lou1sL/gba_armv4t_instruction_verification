
inline void ARM7TDMI::SRNZ64(std::uint64_t value){
    registers.cpsr.n = value >> 63;
    registers.cpsr.z = (value == 0);
}
inline void ARM7TDMI::SRNZ(std::uint32_t value){
    registers.cpsr.n = value >> 31;
    registers.cpsr.z = (value == 0);
}

inline void ARM7TDMI::AND(std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag){
    const std::uint32_t result = s1 & s2;
    if(set_flag) SRNZ(result);
    if(dst_reg >= 0) registers[dst_reg] = result;
}
inline void ARM7TDMI::EOR(std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag){
    const std::uint32_t result = s1 ^ s2;
    if(set_flag) SRNZ(result);
    if(dst_reg >= 0) registers[dst_reg] = result;
}
inline void ARM7TDMI::ORR(std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag){
    const std::uint32_t result = s1 | s2;
    if(set_flag) SRNZ(result);
    if(dst_reg >= 0) registers[dst_reg] = result;
}
inline void ARM7TDMI::MOV(std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag){
    const std::uint32_t result =      s2;
    if(set_flag) SRNZ(result);
    registers[dst_reg] = result;
}
inline void ARM7TDMI::MVN(std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag){
    const std::uint32_t result =     ~s2;
    if(set_flag) SRNZ(result);
    registers[dst_reg] = result;
}
inline void ARM7TDMI::BIC(std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag){
    const std::uint32_t result = s1 &~s2;
    if(set_flag) SRNZ(result);
    registers[dst_reg] = result;
}

inline void ARM7TDMI::ADD(std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag){
    if (set_flag) {
        const std::uint64_t result64 = (std::uint64_t)s1 + (std::uint64_t)s2;
        const std::uint32_t result32 = (std::uint32_t)result64;
        SRNZ(result32);
        registers.cpsr.c = result64 >> 32;
        registers.cpsr.v = (~(s1 ^ s2) & (s2 ^ result32)) >> 31;
        if(dst_reg >= 0)registers[dst_reg] = result32;
    } else {
        if(dst_reg >= 0)registers[dst_reg] = s1 + s2;
    }
}
inline void ARM7TDMI::ADC(std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag){
    if (set_flag) {
        const std::uint64_t result64 = (std::uint64_t)s1 + (std::uint64_t)s2 + (std::uint64_t)registers.cpsr.c;
        const std::uint32_t result32 = (std::uint32_t)result64;
        SRNZ(result32);
        registers.cpsr.c = result64 >> 32;
        registers.cpsr.v = (~(s1 ^ s2) & (s2 ^ result32)) >> 31;
        registers[dst_reg] = result32;
    } else {
        registers[dst_reg] = s1 + s2 + registers.cpsr.c;
    }
}
inline void ARM7TDMI::SUB(std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag){
    const std::uint32_t result = s1 - s2;
    if (set_flag) {
        SRNZ(result);
        registers.cpsr.c = s1 >= s2;
        registers.cpsr.v = ((s1 ^ s2) & (s1 ^ result)) >> 31;
    }
    if(dst_reg >= 0)registers[dst_reg] = result;
}
inline void ARM7TDMI::SBC(std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag){
    const std::uint32_t s3 = (registers.cpsr.c) ^ 1;
    const std::uint32_t result = s1 - s2 - s3;
    if (set_flag) {
        SRNZ(result);
        registers.cpsr.c = (std::uint64_t)s1 >= (std::uint64_t)s2 + (std::uint64_t)s3;
        registers.cpsr.v = ((s1 ^ s2) & (s1 ^ result)) >> 31;
    }
    registers[dst_reg] = result;
}

//Register specified shift amount:
//Only the least significant byte of the contents of Rs is used to determine the shift amount.
//Rs can be any general register other than R15.
//If this byte is zero, the unchanged contents of Rm will be used as the second operand, 
//   and the old value of the CPSR C flag will be passed on as the shifter carry output.
//If the byte has a value between 1 and 31,
//   the shifted result will exactly match that of an instruction specified shift with the same value and shift operation.
//If the value in the byte is 32 or more, the result will be a logical extension of the shift described above:
//1: LSL by 32 has result zero, carry out equal to bit 0 of Rm.
//2: LSL by more than 32 has result zero, carry out zero.
//3: LSR by 32 has result zero, carry out equal to bit 31 of Rm.
//4: LSR by more than 32 has result zero, carry out zero.
//5: ASR by 32 or more has result filled with and carry out equal to bit 31 of Rm.
//6: ROR by 32 has result equal to Rm, carry out equal to bit 31 of Rm.
//7: ROR by n where n is greater than 32 will give the same result and carry out as ROR by n-32;
//   therefore repeatedly subtract 32 from n until the amount is in the range 1 to 32 and see above.

inline void ARM7TDMI::LSL(std::uint32_t& operand, std::uint32_t amount, int& carry, bool reg_specified, bool set_nz){
    //LSL #0 is a special case, where the shifter carry out is the old value of the CPSR C flag.
    //The contents of Rm are used directly as the second operand. 
    if(amount ==  0){                                     return; }
    if(amount == 32){ carry = operand & 1;   operand = 0; return; }
    if(amount >  32){ carry = 0;             operand = 0; return; }
    carry = (operand << (amount - 1)) >> 31;
    operand <<= amount;
    if(set_nz) SRNZ(operand);
}
inline void ARM7TDMI::LSR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool reg_specified, bool set_nz){
    //The form of the shift field which might be expected to correspond to LSR #0 is used to encode LSR #32,
    //   which has a zero result with bit 31 of Rm as the carry output.
    //Logical shift right zero is redundant as it is the same as logical shift left zero,
    //   so the assembler will convert LSR #0 (and ASR #0 and ROR #0) into LSL #0,
    //   and allow LSR #32 to be specified. 
    if(amount ==  0){ if(reg_specified) return; else amount = 32; }
    if(amount == 32){ carry = operand >> 31; operand = 0; return; }
    if(amount >  32){ carry = 0;             operand = 0; return; }
    carry = (operand >> (amount - 1)) &  1;
    operand >>= amount;
    if(set_nz) SRNZ(operand);
}
inline void ARM7TDMI::ASR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool reg_specified, bool set_nz){
    //The form of the shift field which might be expected to give ASR #0 is used to encode ASR #32.
    //Bit 31 of Rm is again used as the carry output, and each bit of operand 2 is also equal to bit 31 of Rm.
    //The result is therefore all ones or all zeros, according to the value of bit 31 of Rm.
    if(amount ==  0){ if(reg_specified) return; else amount = 32; }
    int sign_bit = operand >> 31;
    if(amount >= 32){ carry = sign_bit; operand = 0xFFFFFFFF * sign_bit; return; }
    carry = (operand >> (amount - 1)) &  1;
    operand = (operand >> amount) | ((0xFFFFFFFF * sign_bit) << (32 - amount));
    if(set_nz) SRNZ(operand);
}
inline void ARM7TDMI::ROR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool reg_specified, bool set_nz){
    //The form of the shift field which might be expected to give ROR #0 is used to encode a special function of the barrel shifter, rotate right extended (RRX).
    //This is a rotate right by one bit position of the 33 bit quantity formed by appending the CPSR C flag to the most significant end of the contents of Rm.
    if (amount == 0) {
        if(reg_specified) return;
        auto lsb = operand & 1;
        operand = (operand >> 1) | (carry << 31);
        carry = lsb;
    }
    else {
        amount &= 0b11111;
        operand = (operand >> amount) | (operand << (32 - amount));
        carry = operand >> 31;
    }
    if(set_nz) SRNZ(operand);
}