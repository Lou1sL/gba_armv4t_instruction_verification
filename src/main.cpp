#include "arm7tdmi_gba_debug.h"

#include <gba_console.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <stdio.h>
#include <string.h>

ARM7TDMI_DEBUG_GBA *cpusim = NULL;

static inline void PrintReg(ARM7TDMI& cpu){
    for(std::size_t i=0; i<17; i++){
		if(i != 16) printf("R%02d :%08lX \n", i, cpu.registers[i]);
		else printf("CPSR:%08lX \n", cpu.registers.cpsr.value);
    }
}

static inline void RegSync(ARM7TDMI& cpu){

    std::uint32_t* v;

    v = &(cpu.registers.cpsr.value);
    __asm(" mov r0, %[val]" : : [val] "r" (v));
    __asm(" mrs r1, cpsr");
    __asm(" str r1, [r0]");

    v = &(cpu.registers[0]);
    __asm(" mov r0, %[val]" : : [val] "r" (v));
    __asm(" str r0, [r0]");
    __asm(" str r1, [r0, #4]");
    __asm(" str r2, [r0, #8]");
    __asm(" str r3, [r0, #12]");
    __asm(" str r4, [r0, #16]");
    __asm(" str r5, [r0, #20]");
    __asm(" str r6, [r0, #24]");
    __asm(" str r7, [r0, #28]");
    __asm(" str r8, [r0, #32]");
    __asm(" str r9, [r0, #36]");
    __asm(" str r10, [r0, #40]");
    __asm(" str r11, [r0, #44]");
    __asm(" str r12, [r0, #48]");
    __asm(" str r13, [r0, #52]");
    __asm(" str r14, [r0, #56]");
    __asm(" str r15, [r0, #60]");
    
    PrintReg(cpu);
}

int main(void){
	irqInit();
	irqEnable(IRQ_VBLANK);
	consoleDemoInit();

    cpusim = new ARM7TDMI_DEBUG_GBA();
    RegSync(cpusim->cpu);

	while(1) VBlankIntrWait();
}