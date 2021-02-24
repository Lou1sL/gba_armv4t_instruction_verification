#pragma once

#include "arm7tdmi.h"

class BusBindGBA : public BusInterface {
public:
    BusBindGBA(){}
    std::uint8_t  R8   (std::uint32_t addr, bool isSeq){
        return *reinterpret_cast<std::uint8_t*>(&(reinterpret_cast<std::uint8_t*>(0))[addr]);
    }
    std::uint16_t R16  (std::uint32_t addr, bool isSeq){
        return *reinterpret_cast<std::uint16_t*>(&(reinterpret_cast<std::uint8_t*>(0))[addr]);
    }
    std::uint32_t R32  (std::uint32_t addr, bool isSeq){
        return *reinterpret_cast<std::uint32_t*>(&(reinterpret_cast<std::uint8_t*>(0))[addr]);
    }
    void          W8   (std::uint32_t addr, std::uint8_t  val, bool isSeq){
        //*reinterpret_cast<std::uint8_t*>(&(reinterpret_cast<std::uint8_t*>(0))[addr]) = val;
    }
    void          W16  (std::uint32_t addr, std::uint16_t val, bool isSeq){
        //*reinterpret_cast<std::uint16_t*>(&(reinterpret_cast<std::uint8_t*>(0))[addr]) = val;
    }
    void          W32  (std::uint32_t addr, std::uint32_t val, bool isSeq){
        //*reinterpret_cast<std::uint32_t*>(&(reinterpret_cast<std::uint8_t*>(0))[addr]) = val;
    }
};

class ARM7TDMI_DEBUG_GBA {
public:
    BusBindGBA main_bus;
    ARM7TDMI cpu;
    ARM7TDMI_DEBUG_GBA() : main_bus(), cpu(&main_bus) {  }
};