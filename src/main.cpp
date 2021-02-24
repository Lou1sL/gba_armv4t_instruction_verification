#include "arm7tdmi_gba_debug.h"

#include <gba_console.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <stdio.h>
#include <string.h>

ARM7TDMI_DEBUG_GBA *cpusim = NULL;

static inline void PrintSimReg(ARM7TDMI& cpu){
    for(std::size_t i=0; i<17; i++){
		if(i != 16) printf("%08lX  ", cpu.registers[i]);
		else printf("%08lX\n\n", cpu.registers.cpsr.value);
    }
}

static inline void PrintRealReg(std::uint32_t* reg){
    for(std::size_t i=0; i<17; i++){
		if(i != 16) printf("%08lX  ", reg[i]);
		else printf("%08lX\n\n", reg[16]);
	}
}


static inline void RegSync(ARM7TDMI& cpu){
    
    std::uint32_t reg_tmp[17] = {0};
    std::uint32_t* ptr_cpsr = &(cpu.registers.cpsr.value);
    std::uint32_t* ptr_reg  = &(cpu.registers[0]);
    std::uint32_t* ptr_tmp  = &(reg_tmp[0]);

    __asm("ldr r0, %[val]" : : [val] "m" (ptr_cpsr));
    __asm("mrs r1, cpsr");
    __asm("str r1, [r0]");

    __asm("ldr r0, %[val]" : : [val] "m" (ptr_reg));
    __asm("stmia r0, {r0-r15}");
    
    

    __asm("ldr r0, %[val]" : : [val] "m" (ptr_tmp));
    __asm("mrs r1, cpsr");
    __asm("str r1, [r0, #64]");
    __asm("stmia r0, {r0-r15}");

    PrintSimReg(cpu);
    PrintRealReg(reg_tmp);
}

int main(void){
	irqInit();
	irqEnable(IRQ_VBLANK);
	consoleDemoInit();

    cpusim = new ARM7TDMI_DEBUG_GBA();
    RegSync(cpusim->cpu);

	while(1) VBlankIntrWait();
}