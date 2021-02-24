#include "arm7tdmi.h"
#include "pipeline.h"
#include "exception.h"
#include "alu.h"
#include "instruction_arm.h"
#include "instruction_thumb.h"
#include "instruction_decode.h"

void ARM7TDMI::Step(){
    if(registers.cpsr.t){
        (this->*thumb_decode_table[instruction_register >> 6])(instruction_register);
    }else{
        if(ReadCondition(static_cast<CONDITION>(instruction_register >> 28))){
            (this->*arm_decode_table[
                ((instruction_register >> 16) & 0xFF0) | 
                ((instruction_register >>  4) & 0x00F)
            ])(instruction_register);
        }else{
            PipelineFetch32(true);
        }
    }
}