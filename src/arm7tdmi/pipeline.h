
inline auto ARM7TDMI::PipelineFetch16(bool isSequential) -> void {
    registers[15] &= ~0b01;
    instruction_register = pipeline[0];
    pipeline[0]          = pipeline[1];
    registers[15] += 2;
    pipeline[1]          = bus->R16(registers[15], isSequential);
}

inline auto ARM7TDMI::PipelineFetch32(bool isSequential) -> void {
    registers[15] &= ~0b11;
    instruction_register = pipeline[0];
    pipeline[0]          = pipeline[1];
    registers[15] += 4;
    pipeline[1]          = bus->R32(registers[15], isSequential);
}

inline auto ARM7TDMI::PipelineReload16() -> void {
    registers[15] &= ~0b01;
    instruction_register = bus->R16(registers[15], false);
    registers[15] += 2;
    pipeline[0]          = bus->R16(registers[15],  true);
    registers[15] += 2;
    pipeline[1]          = bus->R16(registers[15],  true);
}

inline auto ARM7TDMI::PipelineReload32() -> void {
    registers[15] &= ~0b11;
    instruction_register = bus->R32(registers[15], false);
    registers[15] += 4;
    pipeline[0]          = bus->R32(registers[15],  true);
    registers[15] += 4;
    pipeline[1]          = bus->R32(registers[15],  true);
}