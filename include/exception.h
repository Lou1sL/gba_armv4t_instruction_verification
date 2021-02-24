
// Exception vector addresses
constexpr std::uint32_t EVA_RST = 0x00000000;
constexpr std::uint32_t EVA_UND = 0x00000004;
constexpr std::uint32_t EVA_SWI = 0x00000008;
constexpr std::uint32_t EVA_PFA = 0x0000000C; //Unused
constexpr std::uint32_t EVA_DTA = 0x00000010; //Unused
constexpr std::uint32_t EVA_RSV = 0x00000014; //Reserved
constexpr std::uint32_t EVA_IRQ = 0x00000018;
constexpr std::uint32_t EVA_FIQ = 0x0000001C; //Unused

//When an exception arises, CPU is switched into ARM state, and the program counter (PC) is loaded by the respective address.
//Address  Prio  Exception                  Mode on Entry      Interrupt Flags
//BASE+00h 1     Reset                      Supervisor (_svc)  I=1, F=1
//BASE+18h 4     Normal Interrupt (IRQ)     IRQ        (_irq)  I=1, F=unchanged
//BASE+08h 6     Software Interrupt (SWI)   Supervisor (_svc)  I=1, F=unchanged
//BASE+04h 7     Undefined Instruction      Undefined  (_und)  I=1, F=unchanged
//
//BASE+1Ch 3     Fast Interrupt (FIQ)       FIQ        (_fiq)  I=1, F=1
//BASE+10h 2     Data Abort                 Abort      (_abt)  I=1, F=unchanged
//BASE+0Ch 5     Prefetch Abort             Abort      (_abt)  I=1, F=unchanged
//BASE+14h ??    Address Exceeds 26bit      Supervisor (_svc)  I=1, F=unchanged


//The ARM7TDMI processor handles an exception as follows:
//1. Preserves the address of the next instruction in the appropriate LR.
//When the exception entry is from ARM state, the ARM7TDMI processor copies
//the address of the next instruction into the LR, current PC+4 or PC+8 depending
//on the exception.
//When the exception entry is from Thumb state, the ARM7TDMI processor writes
//the value of the PC into the LR, offset by a value, current PC+4 or PC+8
//depending on the exception, that causes the program to resume from the correct
//place on return.
//The exception handler does not have to determine the state when entering an
//exception. For example, in the case of a SWI, MOVS PC, r14_svc always returns to
//the next instruction regardless of whether the SWI was executed in ARM or
//Thumb state.
//2. Copies the CPSR into the appropriate SPSR.
//3. Forces the CPSR mode bits to a value that depends on the exception.
//4. Forces the PC to fetch the next instruction from the relevant exception vector.


//When the nRESET signal goes LOW, the ARM7TDMI-S processor abandons the
//executing instruction.
//When nRESET goes HIGH again the ARM7TDMI-S processor:
//1. Forces M[4:0] to b10011 (Supervisor mode).
//2. Sets the I and F bits in the CPSR.
//3. Clears the CPSR T bit.
//4. Forces the PC to fetch the next instruction from address 0x00.
//5. Reverts to ARM state and resumes execution.
//After reset, all register values except the PC and CPSR are indeterminate.
//After nRESET has been taken HIGH, the ARM core does two further internal cycles
//before the first instruction is fetched from the reset vector (address 0x00000000). It then
//takes three MCLK cycles to advance this instruction through the
//Fetch-Decode-Execute stages of the ARM instruction pipeline before this first
//instruction is executed.
void ARM7TDMI::EXCEP_RST(){
    registers.Reset();
    registers.SwitchToMode(MODE_SVC);
    registers.cpsr.i = 1;
    registers.cpsr.f = 1;
    registers.cpsr.t = 0;
    registers[15] = EVA_RST;
    PipelineReload32();
}

void ARM7TDMI::EXCEP_DTA(){ /** Unused in GBA */ }

//when the F bit is set, FIQ interrupts are disabled
void ARM7TDMI::EXCEP_FIQ(){ /** Unused in GBA */ }

void ARM7TDMI::EXCEP_IRQ(){
    //when the I bit is set, IRQ interrupts are disabled
    if(registers.cpsr.i != 0) return;
    //When the exception entry is from ARM state, the ARM7TDMI processor copies
    //the address of the next instruction into the LR
    //When the exception entry is from Thumb state, the ARM7TDMI processor writes
    //the value of the PC into the LR
    registers.banked[BANK_IRQ][BANK_R14] = registers[15] - (registers.cpsr.t == 0 ? 4 : 0);
    registers.spsr[BANK_IRQ] = registers.cpsr;
    registers.SwitchToMode(MODE_IRQ);
    registers.cpsr.i = 1;
    registers.cpsr.t = 0;
    registers[15] = EVA_IRQ;
    PipelineReload32();
}

void ARM7TDMI::EXCEP_PFA(){ /** Unused in GBA */ }

void ARM7TDMI::EXCEP_SWI(){
    //The PC is saved in R14_svc upon entering the software interrupt trap,
    //   with the PC adjusted to point to the word after the SWI instruction.
    //MOVS PC,R14_svc will return to the calling program and restore the CPSR.
    registers.banked[BANK_SVC][BANK_R14] = registers[15] - (registers.cpsr.t == 0 ? 4 : 2);
    registers.spsr[BANK_SVC] = registers.cpsr;
    registers.SwitchToMode(MODE_SVC);
    registers.cpsr.i = 1;
    registers.cpsr.t = 0;
    registers[15] = EVA_SWI;
    PipelineReload32();
}

void ARM7TDMI::EXCEP_UND(){
    //Doesn't exist in Thumb
    registers.banked[BANK_UND][BANK_R14] = registers[15] - 4;
    registers.spsr[BANK_UND] = registers.cpsr;
    registers.SwitchToMode(MODE_UND);
    registers.cpsr.i = 1;
    registers[15] = EVA_UND;
    PipelineReload32();
}