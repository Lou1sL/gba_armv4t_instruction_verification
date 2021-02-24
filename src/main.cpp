#include "arm7tdmi_debug.h"

#include <gba_console.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <stdio.h>
#include <string.h>

void ArmTest() {
    ARM7TDMI_DEBUG<1024> *cpusim = new ARM7TDMI_DEBUG<1024>();
    
    cpusim->SetMemory32(0x0,{
        0x0100A0E3,//mov r0, #1
        0x010080E2,//add r0, #1
        0x010080E2,//add r0, #1
    });

    cpusim->cpu.EXCEP_RST();
	printf("0x%08x : ", cpusim->cpu.instruction_register);
    cpusim->cpu.Step();
    printf("r0 is %d\n", cpusim->cpu.registers[0]);

    printf("0x%08x : ", cpusim->cpu.instruction_register);
    cpusim->cpu.Step();
    printf("r0 is %d\n", cpusim->cpu.registers[0]);

    printf("0x%08x : ", cpusim->cpu.instruction_register);
    cpusim->cpu.Step();
    printf("r0 is %d\n", cpusim->cpu.registers[0]);
}

int main() {
	irqInit();
	irqEnable(IRQ_VBLANK);
	consoleDemoInit();
	ArmTest();
	for(;;);
}
