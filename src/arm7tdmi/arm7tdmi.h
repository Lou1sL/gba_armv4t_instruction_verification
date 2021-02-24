#pragma once

#include <array>
#include <cinttypes>

#include "util.h"
#include "datastruct.h"
#include "registers.h"
#include "bus_interface.h"

// Even though the ARM CPU itself would allow to select between Little-Endian and Big-Endian format by using an external circuit,
//    in the GBA no such circuit exists, and the data format is always Little-Endian.
class ARM7TDMI {
public:
    ARM7TDMI(BusInterface* bus) : bus(bus) {  }
    BusInterface* bus;
    RegisterGroup registers;
    std::uint32_t instruction_register;
    std::uint32_t pipeline[2];
    void Step();
    static std::array<std::array<bool, 16>, 16>    condition_table;
    bool ReadCondition(CONDITION c);
    //Exception
    void EXCEP_RST();
    void EXCEP_IRQ();
    void EXCEP_DTA();
    void EXCEP_FIQ();
    void EXCEP_PFA();
    void EXCEP_SWI();
    void EXCEP_UND();
    //Instruction Decoding
    typedef void (ARM7TDMI::*ARMHandler)(ARMInstruction);
    typedef void (ARM7TDMI::*ThumbHandler)(ThumbInstruction);
    static std::array<ARMHandler, 0x1000>         arm_decode_table;
    static std::array<ThumbHandler, 0x400>      thumb_decode_table;
    template <std::uint32_t instruction>
    static constexpr auto ARMDecoder()   -> ARM7TDMI::ARMHandler;
    template <std::uint32_t instruction>
    static constexpr auto ThumbDecoder() -> ARM7TDMI::ThumbHandler;
    static constexpr auto ARMDecodeTableGeneration()   -> std::array<ARM7TDMI::ARMHandler, 0x1000>;
    static constexpr auto ThumbDecodeTableGeneration() -> std::array<ARM7TDMI::ThumbHandler, 0x400>;
    static constexpr auto ConditionTableGeneration()   -> std::array<std::array<bool, 16>, 16>;
private:
    //Pipeline Operation
    inline auto PipelineFetch16(bool isSequential)  -> void;
    inline auto PipelineFetch32(bool isSequential)  -> void;
    inline auto PipelineReload16() -> void;
    inline auto PipelineReload32() -> void;
    //ARM Operation
    template <int i>
    inline void ARMDataProcessingShiftDecoder(std::uint32_t& operand2, int& carry);
    template <int i, int opcode, int s, int i7_4>
    void ARMDataProcessing       (ARMInstruction instruction);
    template <int i, int p, int i21>
    void ARMPSRTransfer          (ARMInstruction instruction);
    template <int a, int s>
    void ARMMultiply             (ARMInstruction instruction);
    template <int u, int a, int s>
    void ARMMultiplyLong         (ARMInstruction instruction);
    template <int b>
    void ARMSingleDataSwap       (ARMInstruction instruction);
    void ARMBranchAndExchange    (ARMInstruction instruction);
    template <int p, int u, int i22, int w, int l, int s, int h>
    void ARMHalfwordDataTransfer (ARMInstruction instruction);
    template <int i, int p, int u, int b, int w, int l, int i7_4>
    void ARMSingleDataTransfer   (ARMInstruction instruction);
    void ARMUndefined            (ARMInstruction instruction);
    template <int p, int u, int s, int w, int l, int i7_4>
    void ARMBlockDataTransfer    (ARMInstruction instruction);
    template <int l, int i23_20, int i7_4>
    void ARMBranch               (ARMInstruction instruction);
    void ARMSoftwareInterrupt    (ARMInstruction instruction);
    //Thumb Operation
    template <int op, int offset5>
    void ThumbMoveShiftedRegister                     (ThumbInstruction instruction);
    template <int i, int op, int rn>
    void ThumbAddAndSubtract                          (ThumbInstruction instruction);
    template <int op, int rd>
    void ThumbMoveCompareAddAndSubtractImme           (ThumbInstruction instruction);
    template <int op>
    void ThumbALUOperation                            (ThumbInstruction instruction);
    template <int op, int h1, int h2>
    void ThumbHighRegisterOperationsAndBranchExchange (ThumbInstruction instruction);
    template <int rd>
    void ThumbPCRelativeLoad                          (ThumbInstruction instruction);
    template <int l, int b, int ro>
    void ThumbLoadAndStoreRegisterOffset              (ThumbInstruction instruction);
    template <int h, int s, int ro>
    void ThumbLoadAndStoreSignExtendedByteAndHalfword (ThumbInstruction instruction);
    template <int b, int l, int offset5>
    void ThumbLoadAndStoreImmediateOffset             (ThumbInstruction instruction);
    template <int l, int offset5>
    void ThumbLoadAndStoreHalfword                    (ThumbInstruction instruction);
    template <int l, int rd>
    void ThumbSPRelativeLoadAndStore                  (ThumbInstruction instruction);
    template <int sp, int rd>
    void ThumbLoadAddress                             (ThumbInstruction instruction);
    template <int s>
    void ThumbAddOffsetToStackPointer                 (ThumbInstruction instruction);
    template <int l, int r>
    void ThumbPushAndPopRegisters                     (ThumbInstruction instruction);
    template <int l, int rb>
    void ThumbMultipleLoadAndStore                    (ThumbInstruction instruction);
    template <int cond>
    void ThumbConditionalBranch                       (ThumbInstruction instruction);
    void ThumbSoftwareInterrupt                       (ThumbInstruction instruction);
    void ThumbUnconditionalBranch                     (ThumbInstruction instruction);
    template <int h>
    void ThumbLongBranchWithLink                      (ThumbInstruction instruction);
    void ThumbUndefined                               (ThumbInstruction instruction);
    //Arithmatic & Logic Operation
    inline void AND (std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag);
    inline void EOR (std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag);
    inline void SUB (std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag);
    inline void ADD (std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag);
    inline void ADC (std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag);
    inline void SBC (std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag);
    inline void ORR (std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag);
    inline void MOV (std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag);
    inline void BIC (std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag);
    inline void MVN (std::uint32_t s1, std::uint32_t s2, std::uint8_t dst_reg, bool set_flag);
    inline void LSL (std::uint32_t& operand, std::uint32_t amount, int& carry, bool reg_specified, bool set_nz);
    inline void LSR (std::uint32_t& operand, std::uint32_t amount, int& carry, bool reg_specified, bool set_nz);
    inline void ASR (std::uint32_t& operand, std::uint32_t amount, int& carry, bool reg_specified, bool set_nz);
    inline void ROR (std::uint32_t& operand, std::uint32_t amount, int& carry, bool reg_specified, bool set_nz);
    inline void SRNZ   (std::uint32_t value);
    inline void SRNZ64 (std::uint64_t value);
};

