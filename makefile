
# Envirmental Settings -------------

COMPILER_DIR = /mnt/e/ProgramTools/gcc-arm-none-eabi/bin
STLINK_DIR   = /mnt/e/ProgramTools/stlink/bin
STM32LIB_DIR = /mnt/e/ProgramTools/STM32Lib

# ----------------------------------

CC      = $(COMPILER_DIR)/arm-none-eabi-gcc.exe
CXX     = $(COMPILER_DIR)/arm-none-eabi-g++.exe
ASM     = $(COMPILER_DIR)/arm-none-eabi-as.exe
LINK    = $(COMPILER_DIR)/arm-none-eabi-g++.exe
OBJCOPY = $(COMPILER_DIR)/arm-none-eabi-objcopy.exe

STFLASH = $(STLINK_DIR)/st-flash

# Libraries ------------------------

STM32LIB_INCLUDE  = -I$(STM32LIB_DIR)/Libraries/CMSIS/Include
STM32LIB_INCLUDE += -I$(STM32LIB_DIR)/Libraries/CMSIS/Device/ST/STM32F4xx/Include
STM32LIB_INCLUDE += -I$(STM32LIB_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/inc
# STM32LIB_INCLUDE += -I$(STM32LIB_DIR)/Utilities/STM32F4-Discovery

vpath %.c $(STM32LIB_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/src
STM32LIB_SRC  = stm32f4xx_exti.c stm32f4xx_gpio.c stm32f4xx_rcc.c stm32f4xx_syscfg.c
STM32LIB_SRC += stm32f4xx_tim.c misc.c
# vpath %.c $(STM32LIB_DIR)/Utilities/STM32F4-Discovery
# STM32LIB_SRC += stm32f4_discovery.c
# STM32LIB_SRC += "$(STM32LIB_DIR)/Libraries/CMSIS/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f4xx.s "

# ----------------------------------

# Project Settings -----------------

APP_NAME     = gbainsver
BUILD_DIR    = ./build
SRC_DIR      = ./src
INCLUDE_DIR  = ./src
LINKER_PATH  = ./src/stm32f4.ld

SRCS  = $(SRC_DIR)/main.cpp
# SRCS += $(SRC_DIR)/system_stm32f4xx.c $(SRC_DIR)/stm32f4xx_it.c
SRCS += $(STM32LIB_SRC)
OBJS  = $(SRCS:.c=.o)

# ----------------------------------

# Compiling Flags ------------------

CXXFLAGS  = -g -O2 -Wall -T $(LINKER_PATH) -std=c++17
CXXFLAGS += -DUSE_STDPERIPH_DRIVER
CXXFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CXXFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CXXFLAGS += $(STM32LIB_INCLUDE) -I$(INCLUDE_DIR) -Wl,-Map,$(BUILD_DIR)/$(APP_NAME).map

# ----------------------------------

.PHONY: all clean

all: proj

proj: $(BUILD_DIR)/$(APP_NAME).elf

$(BUILD_DIR)/$(APP_NAME).elf: $(SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@ 
	$(OBJCOPY) -O ihex $(BUILD_DIR)/$(APP_NAME).elf $(BUILD_DIR)/$(APP_NAME).hex
	$(OBJCOPY) -O binary $(BUILD_DIR)/$(APP_NAME).elf $(BUILD_DIR)/$(APP_NAME).bin

clean:
	-rm $(BUILD_DIR)/*.o
	-rm $(BUILD_DIR)/*.elf
	-rm $(BUILD_DIR)/*.map
	-rm $(BUILD_DIR)/*.bin
	-rm $(BUILD_DIR)/*.hex

burn: proj
	$(STFLASH) write $(BUILD_DIR)/$(APP_NAME).bin 0x8000000