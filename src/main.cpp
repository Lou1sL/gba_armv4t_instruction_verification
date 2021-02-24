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
u8 result[0xf0];
static inline void RegSync(ARM7TDMI& cpu){

    std::uint32_t* v;

    v = &(cpu.registers.cpsr.value);
    __asm("mov r0, %[val]" : : [val] "r" (v));
    __asm("mrs r1, cpsr");
    __asm("str r1, [r0]");

    v = &(cpu.registers[0]);
    __asm("mov r0, %[val]" : : [val] "r" (v));
    __asm("stmia r0, {r0-r15}");
    
    //PrintReg(cpu);
    
    
    __asm__ __volatile__(
        "ldr   r0, =result   \n"
        "stmia r0, {r0-r15}  \n"
        "mrs   r1, CPSR      \n"
        "str   r1, [R0,#64]  \n"
        : : : "r0", "r1", "memory");
        
	for(u8 i=2;i<17;i++){
		u32 Reg = ((u32)result[i*4+3] << 24) | ((u32)result[i*4+2] << 16) | ((u32)result[i*4+1] <<  8) | ((u32)result[i*4+0] <<  0);
		if(i != 16) printf("R%02d :%08lX \n",i,Reg);
		else printf("CPSR:%08lX \n",Reg);
	}
}

int main(void){
	irqInit();
	irqEnable(IRQ_VBLANK);
	consoleDemoInit();

    cpusim = new ARM7TDMI_DEBUG_GBA();
    RegSync(cpusim->cpu);

	while(1) VBlankIntrWait();
}