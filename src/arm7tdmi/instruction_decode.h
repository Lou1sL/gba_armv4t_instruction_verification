
template <std::uint32_t instruction>
constexpr auto ARM7TDMI::ARMDecoder() -> ARM7TDMI::ARMHandler {

    const int i27_25 = (instruction >> 25) & 0b111;
    const int i24_21 = (instruction >> 21) & 0b1111;
    const int i23_20 = (instruction >> 20) & 0b1111;
    const int i7_4   = (instruction >>  4) & 0b111;

    const int i25    = (instruction >> 25) & 0b1;
    const int i24    = (instruction >> 24) & 0b1;
    const int i23    = (instruction >> 23) & 0b1;
    const int i22    = (instruction >> 22) & 0b1;
    const int i21    = (instruction >> 21) & 0b1;
    const int i20    = (instruction >> 20) & 0b1;
    const int i6     = (instruction >>  6) & 0b1;
    const int i5     = (instruction >>  5) & 0b1;

    //Data processing & PSR transfer && I == 1
    if(i27_25 == 0b001){
        if((i20==0) && i24_21 >= 0b1000 && i24_21 <= 0b1011) return &ARM7TDMI::ARMPSRTransfer<i25, i22, i21>;
        else return &ARM7TDMI::ARMDataProcessing<i25, i24_21, i20, i7_4>;
    }else if(i27_25 == 0b000){
        //Branch and exchange
        //Note:middle [19:8] bits should be all 1 but since the table won't be generated so... 
        if ((0x0FF000F0 & instruction) == 0x01200010) return &ARM7TDMI::ARMBranchAndExchange;
        //Multiply
        else if ((0x0FC000F0 & instruction) == 0x00000090) return &ARM7TDMI::ARMMultiply<i21, i20>;
        //Multiply long
        else if ((0x0F8000F0 & instruction) == 0x00800090) return &ARM7TDMI::ARMMultiplyLong<i22, i21, i20>;
        //Single data swap
        //Note:middle [11:8] bits ignored for same reason.
        else if ((0x0FB000F0 & instruction) == 0x01000090) return &ARM7TDMI::ARMSingleDataSwap<i22>;
        else if ((instruction & 0xF0) == 0xB0 || (instruction & 0xD0) == 0xD0) return &ARM7TDMI::ARMHalfwordDataTransfer<i24, i23, i22, i21, i20, i6, i5>;
        else {
            if((i20==0) && i24_21 >= 0b1000 && i24_21 <= 0b1011) return &ARM7TDMI::ARMPSRTransfer<i25, i22, i21>;
            else return &ARM7TDMI::ARMDataProcessing<i25, i24_21, i20, i7_4>;
        }
    }
    //Undefined
    else if ((0x0E000010 & instruction) == 0x06000010) return &ARM7TDMI::ARMUndefined;
    //Single data transfer
    else if ((0x0C000000 & instruction) == 0x04000000) {
        return &ARM7TDMI::ARMSingleDataTransfer<i25, i24, i23, i22, i21, i20, i7_4>;
    }
    //Block data transfer
    else if ((0x0E000000 & instruction) == 0x08000000) return &ARM7TDMI::ARMBlockDataTransfer<i24, i23, i22, i21, i20, i7_4>;
    //Branch
    else if ((0x0E000000 & instruction) == 0x0A000000) return &ARM7TDMI::ARMBranch<i24, i23_20, i7_4>;
    //Coprocessor data transfer
    else if ((0x0E000000 & instruction) == 0x0C000000) return &ARM7TDMI::ARMUndefined;
    //Coprocessor data operation
    else if ((0x0F000010 & instruction) == 0x0E000000) return &ARM7TDMI::ARMUndefined;
    //Coprocessor register transfer
    else if ((0x0F000010 & instruction) == 0x0E000010) return &ARM7TDMI::ARMUndefined;
    //Software interrupt
    else if ((0x0F000000 & instruction) == 0x0F000000) return &ARM7TDMI::ARMSoftwareInterrupt;
    else return &ARM7TDMI::ARMUndefined;
}


template <std::uint32_t instruction>
constexpr auto ARM7TDMI::ThumbDecoder() -> ARM7TDMI::ThumbHandler {

    const int i12_11 = (instruction >> 11) & 0b11;
    const int i11_8  = (instruction >>  8) & 0b1111;
    const int i10_8  = (instruction >>  8) & 0b111;
    const int i10_6  = (instruction >>  6) & 0b11111;
    const int i9_8   = (instruction >>  8) & 0b11;
    const int i9_6   = (instruction >>  6) & 0b1111;
    const int i8_6   = (instruction >>  6) & 0b111;

    const int i12    = (instruction >> 12) & 0b1;
    const int i11    = (instruction >> 11) & 0b1;
    const int i10    = (instruction >> 10) & 0b1;
    const int i9     = (instruction >>  9) & 0b1;
    const int i8     = (instruction >>  8) & 0b1;
    const int i7     = (instruction >>  7) & 0b1;
    const int i6     = (instruction >>  6) & 0b1;

    if ((0xF800 & instruction) < 0x1800) return &ARM7TDMI::ThumbMoveShiftedRegister<i12_11, i10_6>;
    else if ((0xF800 & instruction) == 0x1800) return &ARM7TDMI::ThumbAddAndSubtract<i10, i9, i8_6>;
    else if ((0xE000 & instruction) == 0x2000) return &ARM7TDMI::ThumbMoveCompareAddAndSubtractImme<i12_11, i10_8>;
    else if ((0xFC00 & instruction) == 0x4000) return &ARM7TDMI::ThumbALUOperation<i9_6>;
    else if ((0xFC00 & instruction) == 0x4400) return &ARM7TDMI::ThumbHighRegisterOperationsAndBranchExchange<i9_8, i7, i6>;
    else if ((0xF800 & instruction) == 0x4800) return &ARM7TDMI::ThumbPCRelativeLoad<i10_8>;
    else if ((0xF200 & instruction) == 0x5000) return &ARM7TDMI::ThumbLoadAndStoreRegisterOffset<i11, i10, i8_6>;
    else if ((0xF200 & instruction) == 0x5200) return &ARM7TDMI::ThumbLoadAndStoreSignExtendedByteAndHalfword<i11, i10, i8_6>;
    else if ((0xE000 & instruction) == 0x6000) return &ARM7TDMI::ThumbLoadAndStoreImmediateOffset<i12, i11, i10_6>;
    else if ((0xF000 & instruction) == 0x8000) return &ARM7TDMI::ThumbLoadAndStoreHalfword<i11, i10_6>;
    else if ((0xF000 & instruction) == 0x9000) return &ARM7TDMI::ThumbSPRelativeLoadAndStore<i11, i10_8>;
    else if ((0xF000 & instruction) == 0xA000) return &ARM7TDMI::ThumbLoadAddress<i11, i10_8>;
    else if ((0xFF00 & instruction) == 0xB000) return &ARM7TDMI::ThumbAddOffsetToStackPointer<i7>;
    else if ((0xF600 & instruction) == 0xB400) return &ARM7TDMI::ThumbPushAndPopRegisters<i11, i8>;
    else if ((0xF000 & instruction) == 0xC000) return &ARM7TDMI::ThumbMultipleLoadAndStore<i11, i10_8>;
    else if ((0xFF00 & instruction) <  0xDF00) return &ARM7TDMI::ThumbConditionalBranch<i11_8>;
    else if ((0xFF00 & instruction) == 0xDF00) return &ARM7TDMI::ThumbSoftwareInterrupt;
    else if ((0xF800 & instruction) == 0xE000) return &ARM7TDMI::ThumbUnconditionalBranch;
    else if ((0xF000 & instruction) == 0xF000) return &ARM7TDMI::ThumbLongBranchWithLink<i11>;
    else return &ARM7TDMI::ThumbUndefined;
}

constexpr auto ARM7TDMI::ARMDecodeTableGeneration() -> std::array<ARM7TDMI::ARMHandler, 0x1000> {
    std::array<ARM7TDMI::ARMHandler, 0x1000> table{};
    static_for<std::size_t, 0, 0x1000>([&](auto i) {
        table[i] = ARMDecoder<((i & 0xFF0) << 16) | ((i & 0x00F) << 4)>();
    });
    return table;
}

constexpr auto ARM7TDMI::ThumbDecodeTableGeneration() -> std::array<ARM7TDMI::ThumbHandler, 0x400>{
    std::array<ARM7TDMI::ThumbHandler, 0x400> table{};
    static_for<std::size_t, 0, 0x400>([&](auto i) {
        table[i] = ThumbDecoder<i << 6>();
    });
    return table;
}

constexpr auto ARM7TDMI::ConditionTableGeneration() -> std::array<std::array<bool, 16>, 16> {
    std::array<std::array<bool, 16>, 16> table{};
    for(int i=0;i<16;i++) {
        bool n = (i & 0b1000), z = (i & 0b0100), c = (i & 0b0010), v = (i & 0b0001);
        table[i] = std::array<bool, 16> {
            z     , !z    , c    , !c   , n          , !n        , v    , !v    ,
            c&&!z , !c||z , n==v , n!=v , !z&&(n==v) , z||(n!=v) , true , false
        };
    }
    return table;
}

bool ARM7TDMI::ReadCondition(CONDITION c){
    if(c==AL) return true;
    if(c==NV) return false;
    return condition_table[registers.cpsr.nzcv][static_cast<int>(c)];
}

std::array<ARM7TDMI::ARMHandler, 0x1000>   ARM7TDMI::arm_decode_table   = ARM7TDMI::ARMDecodeTableGeneration();
std::array<ARM7TDMI::ThumbHandler, 0x400>  ARM7TDMI::thumb_decode_table = ARM7TDMI::ThumbDecodeTableGeneration();
std::array<std::array<bool, 16>, 16>       ARM7TDMI::condition_table    = ARM7TDMI::ConditionTableGeneration();