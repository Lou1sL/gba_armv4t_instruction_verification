APP_NAME = gbainsver

SRC_DIR = ./src
BUILD_DIR = ./build
COMPILER_DIR = /home/wasm_arm-gcc
STLINK= ~/stlink.git
STM_COMMON=../../..

CC      = "$(COMPILER_DIR)/bin/arm-none-eabi-gcc"
CXX     = "$(COMPILER_DIR)/bin/arm-none-eabi-g++"
ASM     = "$(COMPILER_DIR)/bin/arm-none-eabi-as"
LINK    = "$(COMPILER_DIR)/bin/arm-none-eabi-g++"
OBJCOPY = "$(COMPILER_DIR)/bin/arm-none-eabi-objcopy"











SRCS= main.c system_stm32f4xx.c stm32f4xx_it.c

# Library modules
SRCS += stm32f4xx_exti.c stm32f4xx_gpio.c stm32f4xx_rcc.c stm32f4xx_syscfg.c
SRCS += stm32f4xx_tim.c misc.c
SRCS += stm32f4_discovery.c

#######################################################################################



CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

CFLAGS  = -g -O2 -Wall -Tstm32_flash.ld 
CFLAGS += -DUSE_STDPERIPH_DRIVER
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -I.

# Include files from STM libraries
CFLAGS += -I$(STM_COMMON)/Utilities/STM32F4-Discovery
CFLAGS += -I$(STM_COMMON)/Libraries/CMSIS/Include 
CFLAGS += -I$(STM_COMMON)/Libraries/CMSIS/ST/STM32F4xx/Include
CFLAGS += -I$(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/inc


# add startup file to build
SRCS += $(STM_COMMON)/Libraries/CMSIS/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f4xx.s 

OBJS = $(SRCS:.c=.o)

vpath %.c $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src $(STM_COMMON)/Utilities/STM32F4-Discovery

.PHONY: proj

all: proj

proj: $(BUILD_DIR)/$(APP_NAME).elf

$(BUILD_DIR)/$(APP_NAME).elf: $(SRCS)
	$(CXX) $(CFLAGS) $^ -o $@ 
	$(OBJCOPY) -O ihex $(BUILD_DIR)/$(APP_NAME).elf $(BUILD_DIR)/$(APP_NAME).hex
	$(OBJCOPY) -O binary $(BUILD_DIR)/$(APP_NAME).elf $(BUILD_DIR)/$(APP_NAME).bin

clean:
	-rm $(BUILD_DIR)/*.o
	-rm $(BUILD_DIR)/*.elf
	-rm $(BUILD_DIR)/*.map
	-rm $(BUILD_DIR)/*.bin
	-rm $(BUILD_DIR)/*.hex

burn: proj
	$(STLINK)/st-flash write $(BUILD_DIR)/$(APP_NAME).bin 0x8000000