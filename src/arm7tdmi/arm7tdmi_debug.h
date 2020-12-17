#pragma once

#include "arm7tdmi.h"

template <std::size_t N>
class BusBindFullRAM : public BusInterface {
public:
    std::array<std::uint8_t,N>* memory;
    BusBindFullRAM(std::array<std::uint8_t,N>* mem) : memory(mem){}
    std::uint8_t  R8   (std::uint32_t addr, bool isSeq){
        return *reinterpret_cast<std::uint8_t*>(&(reinterpret_cast<std::uint8_t*>(memory))[addr]);
    }
    std::uint16_t R16  (std::uint32_t addr, bool isSeq){
        return *reinterpret_cast<std::uint16_t*>(&(reinterpret_cast<std::uint8_t*>(memory))[addr]);
    }
    std::uint32_t R32  (std::uint32_t addr, bool isSeq){
        return *reinterpret_cast<std::uint32_t*>(&(reinterpret_cast<std::uint8_t*>(memory))[addr]);
    }
    void          W8   (std::uint32_t addr, std::uint8_t  val, bool isSeq){
        *reinterpret_cast<std::uint8_t*>(&(reinterpret_cast<std::uint8_t*>(memory))[addr]) = val;
    }
    void          W16  (std::uint32_t addr, std::uint16_t val, bool isSeq){
        *reinterpret_cast<std::uint16_t*>(&(reinterpret_cast<std::uint8_t*>(memory))[addr]) = val;
    }
    void          W32  (std::uint32_t addr, std::uint32_t val, bool isSeq){
        *reinterpret_cast<std::uint32_t*>(&(reinterpret_cast<std::uint8_t*>(memory))[addr]) = val;
    }
};


template <std::size_t MEM_SIZE>
class ARM7TDMI_DEBUG {
public:
    std::array<std::uint8_t,MEM_SIZE> memory;
    BusBindFullRAM<MEM_SIZE> main_bus;
    ARM7TDMI cpu;

    ARM7TDMI_DEBUG() : main_bus(&memory), cpu(&main_bus) { memory.fill(0xFF); }

    //Attention:
    //Due to pipelined CPU nature, changing memory data may not take effect immediatly.
    //For example:
    //During an interrupt(e.g RST), pipeline will already be filled with the data from EVA, and ready for branch.
    //So if you set EVA data afterwards, it simply won't work.
    template <std::size_t SZ>
    void SetMemory32(std::uint32_t addr, const std::uint32_t(&data)[SZ]) {
        for(std::size_t i=0; i<SZ; i++){
            main_bus.W8(addr+i*4+3, (std::uint8_t)(data[i] >>  0), false);
            main_bus.W8(addr+i*4+2, (std::uint8_t)(data[i] >>  8), false);
            main_bus.W8(addr+i*4+1, (std::uint8_t)(data[i] >> 16), false);
            main_bus.W8(addr+i*4  , (std::uint8_t)(data[i] >> 24), false);
        }
    }
    void SetMemory8(std::uint32_t addr, std::uint8_t data){
        main_bus.W8(addr, data, false);
    }
    std::uint8_t GetMemory8(std::uint32_t addr){
        return main_bus.R8(addr, false);
    }
    void ResetMemory(){
        memory.fill(0xFF);
    }

    void SetStackPointer(std::uint32_t val){
        for(std::size_t i=0; i<BANK_COUNT; i++)
            cpu.registers.banked[i][BANK_R13] = val;
        cpu.registers[13] = val;
    }
};

