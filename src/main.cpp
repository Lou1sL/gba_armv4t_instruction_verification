

#include <gba_console.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>

#include <memory>
#include <stdio.h>
#include <string.h>

#include "arm7tdmi_gba_debug.h"

void PrintMem(ARM7TDMI_DEBUG_GBA& sim){
    printf("  MEMORY  | SIMU | PHYS |\n");
    if(sim.writeAction.size() == 0) {
        printf(" -------- |  --  |  --  |\n");
    }else{
        for(const auto& ele : sim.writeAction){
            std::uint32_t addr = std::get<0>(ele);
            std::uint8_t sim_val = std::get<1>(ele);
            std::uint8_t phy_val = (reinterpret_cast<std::uint8_t*>(0))[addr];
            printf(" %08lX |  %02X  |  %02X  |\n", addr, sim_val, phy_val);
        }
    }
}
bool VerifyMem(ARM7TDMI_DEBUG_GBA& sim){
    if(sim.writeAction.size() == 0) return true;
    if(sim.writeAction.size() > 4) return false;
    for(const auto& ele : sim.writeAction)
        if((reinterpret_cast<std::uint8_t*>(0))[std::get<0>(ele)] != std::get<1>(ele))
            return false;
    return true;
}

void PrintRegs(std::uint32_t* ptr_simgreg, std::uint32_t* ptr_simcpsr, std::uint32_t* ptr_phytmp){
    printf(" REG | SIMULATE | PHYSICAL |\n");
    for(std::size_t i=0; i<17; i++){
		if(i != 16)
            printf(" r%02d | %08lX | %08lX |\n", i, ptr_simgreg[i], ptr_phytmp[i]);
		else
            printf(" cpsr| %08lX | %08lX |\n", *ptr_simcpsr, ptr_phytmp[i]);
	}
}

bool VerifyRegs(std::uint32_t* ptr_simgreg, std::uint32_t* ptr_simcpsr, std::uint32_t* ptr_phytmp){
    bool identical = true;
    for(std::size_t i=0; i<17; i++){
	    if(i != 16) identical = ptr_simgreg[i] == ptr_phytmp[i];
	    else identical = *ptr_simcpsr == ptr_phytmp[i];
        if(!identical) break;
	}
    return identical;
}

__attribute__((section(".iwram.testinst")))
extern void TestInstruction(
    std::uint32_t* ptr_simgreg, 
    std::uint32_t* ptr_simcpsr, 
    std::uint32_t* ptr_phytmp, 
    std::uint32_t* ptr_instruction,
    std::uint32_t* ptr_wrimem){
    
    //Write test instrucion
    __asm("ldr r0, %[val]" : : [val] "m" (ptr_instruction));
    __asm("ldr r0, [r0]");
    __asm("str r0, [pc, #32]"); //It overwrites offset + 4*(10-2), which is the test case's position

    //Sync cpsr of the simulator with physical processor
    __asm("ldr r0, %[val]" : : [val] "m" (ptr_simcpsr));
    __asm("mrs r1, cpsr");
    __asm("str r1, [r0]");
    
    //Use r1 as writable memory pointer
    __asm("ldr r2, %[val]" : : [val] "m" (ptr_wrimem));

    //Sync general registers of the simulator with physical processor
    __asm("ldr r0, %[val]" : : [val] "m" (ptr_simgreg));
    __asm("stmia r0, {r0-r15}");
    
    //Both should be synced by now :)
    
    //Fill instruction pipeline
    __asm("nop"); __asm("nop"); __asm("nop");

    /** TEST CASE START **/
    __asm("nop"); //This won't be nop because it has been overwritten
    /** TEST CASE END   **/

    //Obtain a copy of physical registers
    __asm("ldr r0, %[val]" : : [val] "m" (ptr_phytmp));
    __asm("mrs r1, cpsr");
    __asm("str r1, [r0, #64]");
    __asm("stmia r0, {r0-r15}");
    //Obtaining copy causes r15 to increase
    ptr_phytmp[15] -= 4*4;
    //Obtaining copy uses temp registers(r0, r1), so they are useless and we reset them to zero
    ptr_phytmp[0] = 0; ptr_phytmp[1] = 0; ptr_simgreg[0] = 0; ptr_simgreg[1] = 0;
}

void TestInstructionFinish(ARM7TDMI& sim){
    //Fill instruction pipeline
    sim.Step(); sim.Step(); sim.Step();

    /** TEST START **/
    sim.Step();
    /** TEST END   **/

    //The result would be in ptr_phytmp for physical processor & we can compare that with the simulated one
}

int main(void){
    irqInit();
    irqEnable(IRQ_VBLANK);
    consoleDemoInit();
    
    auto sim = std::make_unique<ARM7TDMI_DEBUG_GBA>();
    std::uint32_t phy_tmpreg[17] = {0};
    std::uint32_t writable_mem[4] = {0};
    std::uint32_t test_case[] = { 0xE3A020EE, 0xE5822000 }; //mov r2, #0xEE / str r2, [r2]
    
    std::uint32_t* ptr_simgreg = &(sim->cpu.registers.current[0]);
    std::uint32_t* ptr_simcpsr = &(sim->cpu.registers.cpsr.value);
    std::uint32_t* ptr_phytmp  = &(phy_tmpreg[0]);
    std::uint32_t* ptr_ins     = &(test_case[1]);
    std::uint32_t* ptr_wrimem  = &(writable_mem[0]);
    
    TestInstruction(ptr_simgreg, ptr_simcpsr, ptr_phytmp, ptr_ins, ptr_wrimem);
    TestInstructionFinish(sim->cpu);
    
    //PrintRegs(ptr_simgreg, ptr_simcpsr, ptr_phytmp);
    //printf(VerifyRegs(ptr_simgreg, ptr_simcpsr, ptr_phytmp) ? "REG IDENTICAL!\n" : "REG NOT INDENTICAL!\n");
    
    PrintMem(*sim);
    printf(VerifyMem(*sim) ? "MEM IDENTICAL!\n" : "MEM NOT INDENTICAL!\n");
    
    while(1) VBlankIntrWait();
}