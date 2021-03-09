#pragma once

#include "arm7tdmi.h"

#include <vector>
#include <tuple>

class BusBindGBA : public BusInterface {
public:
    std::vector<std::tuple<std::uint32_t, std::uint8_t>>* wa;
    BusBindGBA(std::vector<std::tuple<std::uint32_t, std::uint8_t>>* wa) : wa(wa){}
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
        wa->push_back(std::make_tuple(addr + 0, val));
    }
    void          W16  (std::uint32_t addr, std::uint16_t val, bool isSeq){
        wa->push_back(std::make_tuple(addr + 0, static_cast<std::uint8_t>(val >> 0)));
        wa->push_back(std::make_tuple(addr + 1, static_cast<std::uint8_t>(val >> 8)));
    }
    void          W32  (std::uint32_t addr, std::uint32_t val, bool isSeq){
        wa->push_back(std::make_tuple(addr + 0, static_cast<std::uint8_t>(val >>  0)));
        wa->push_back(std::make_tuple(addr + 1, static_cast<std::uint8_t>(val >>  8)));
        wa->push_back(std::make_tuple(addr + 2, static_cast<std::uint8_t>(val >> 16)));
        wa->push_back(std::make_tuple(addr + 3, static_cast<std::uint8_t>(val >> 24)));
    }
};

class ARM7TDMI_DEBUG_GBA {
public:
    BusBindGBA main_bus;
    ARM7TDMI cpu;
    std::vector<std::tuple<std::uint32_t, std::uint8_t>> writeAction;
    ARM7TDMI_DEBUG_GBA() : main_bus(&writeAction), cpu(&main_bus) {}
};