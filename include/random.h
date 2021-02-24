const u32 BLOCK_DATA_TRANSFER = 0b11101000100000000000000000000000;

u8 input [0x04];
u8 result[0xf0];

void Init(){
	for (u8 i=0;i<sizeof(result);i++){ result[i] = 0; }
	input[0] = 0x67;
	input[1] = 0x45;
	input[2] = 0x23;
	input[3] = 0x01;
}

void saveRegisters(void) {
	__asm__ __volatile__(
		//"ldr r2, =input+1   \n"
		//"ldr r3, [r2]   \n"
		//"mov r2, #0x00000FF0   \n"
		//"strb r2, [r1] \n"
		//"mrs r2, cpsr  \n"
		//"msr cpsr_fsxc, r2 \n"
		//"msr cpsr_fsx, r2 \n"
		
		"ldr   r0, =result   \n"
		"stmia r0, {r0-r15}  \n"
		"mrs   r1, CPSR      \n"
		"str   r1, [R0,#64]  \n"
	: : : "r0", "r1", "memory");
}

void printSavedRegisters(){
	for(u8 i=2;i<17;i++){
		u32 Reg = 
			((u32)result[i*4+3] << 24) | 
			((u32)result[i*4+2] << 16) | 
			((u32)result[i*4+1] <<  8) | 
			((u32)result[i*4+0] <<  0);
		if(i != 16) printf("R%02d :%08lX \n",i,Reg);
		else printf("CPSR:%08lX \n",Reg);
	}
}

void writeResultToSRAM(){
	*(vu8*) SRAM = 0x55;
	if (*(vu8*) SRAM != 0x55){ printf("SRAM error! Writing aborted!\n"); return; }
	for (u8 i=0;i<sizeof(result);i++){ ((vu8*) SRAM)[i] = result[i]; }
	printf("Writing SRAM complete!\n");
}

void doTest(){
	Init();
	saveRegisters();
	printSavedRegisters();
	writeResultToSRAM();
	printf("\nSUCCESS!\n");
}


//------------------------------------------------
//vim devkitARM/arm-none-eabi/lib/gba_cart.ld
__attribute__((noinline, section(".specialsec"))) extern void iwram_func (void) {
	for(u32 i=0x08000000u; i<UINT32_MAX; i+=1){
		volatile uint8_t* const scr_io = (uint8_t*)i;
		//bool new_line = ((i & 0x0F) == 0x00) || ((i & 0x0F) == 0x08);
		//if(new_line) printf("%06lx", (i - 0x08000000u));
		//printf(" %02x", *scr_io);
		for(u32 i=0; i<(16*10000); i++) { uint8_t* const temp = (uint8_t*)i; }
	}
}
/**
int main() {
	irqInit();
	irqEnable(IRQ_VBLANK);
	consoleDemoInit();
	doTest();
	//memcpy((void*)0x03000000, __specialsec_start, __specialsec_end - __specialsec_start);
	//memcpy((void*)(u32)0x02000000, (void*)(u32)0x0000000008011fb8, (void*)(u32)0x0000000008012078 - (void*)(u32)0x0000000008011fb8);
	//printf("COPY COMPLETE!\n");
	//((void (*)(void))(u32)0x02000000)();
	//printf("FIN!\n");
	//iwram_func();
}
**/
