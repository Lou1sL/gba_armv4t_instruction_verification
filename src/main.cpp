

#include <gba_console.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <stdio.h>
#include <string.h>

#include "arm7tdmi_gba_debug.h"

//mov r2, #0xEE
std::uint32_t test_case[] = { 0xE3A020EE };

static inline void PrintRegs(ARM7TDMI& sim, std::uint32_t* ptr_phytmp){

    std::uint32_t* ptr_simgreg = &(sim.registers.current[0]);
    std::uint32_t* ptr_simcpsr = &(sim.registers.cpsr.value);

    printf("     | SIMULATE | PHYSICAL |\n");
    for(std::size_t i=0; i<17; i++){
		if(i != 16)
            printf(" r%02d | %08lX | %08lX |\n", i, ptr_simgreg[i], ptr_phytmp[i]);
		else
            printf(" cpsr| %08lX | %08lX |\n", *ptr_simcpsr, ptr_phytmp[i]);
	}
}

static inline bool CompareRegs(ARM7TDMI& sim, std::uint32_t* ptr_phytmp){
    
    std::uint32_t* ptr_simgreg = &(sim.registers.current[0]);
    std::uint32_t* ptr_simcpsr = &(sim.registers.cpsr.value);

    bool identical = true;
    for(std::size_t i=0; i<17; i++){
	    if(i != 16) identical = ptr_simgreg[i] == ptr_phytmp[i];
	    else identical = *ptr_simcpsr == ptr_phytmp[i];
        if(!identical) break;
	}
    return identical;
}

__attribute__((section(".iwram.testinst")))
extern void TestInstruction(std::uint32_t* ptr_simgreg, std::uint32_t* ptr_simcpsr, std::uint32_t* ptr_phytmp, std::uint32_t* ptr_instruction){
    
    //Write test instrucion
    __asm("ldr r0, %[val]" : : [val] "m" (ptr_instruction));
    __asm("ldr r0, [r0]");
    __asm("str r0, [pc, #28]"); //It overwrites offset + 4*(9-2), which is the test case's position

    //Sync cpsr of the simulator with physical processor
    __asm("ldr r0, %[val]" : : [val] "m" (ptr_simcpsr));
    __asm("mrs r1, cpsr");
    __asm("str r1, [r0]");
    
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

    ARM7TDMI_DEBUG_GBA *sim = new ARM7TDMI_DEBUG_GBA();
    std::uint32_t phy_tmp[17] = {0};

    TestInstruction(&(sim->cpu.registers.current[0]), &(sim->cpu.registers.cpsr.value), &(phy_tmp[0]), &(test_case[0]));
    TestInstructionFinish(sim->cpu);

    PrintRegs(sim->cpu, &(phy_tmp[0]));
    printf(CompareRegs(sim->cpu, &(phy_tmp[0])) ? "IDENTICAL!\n" : "NOT INDENTICAL!\n");

	while(1) VBlankIntrWait();
}