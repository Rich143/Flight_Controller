CC=arm-none-eabi-gcc	
HEX=arm-none-eabi-objcopy	

RM=rm -rf
BINARY_BASE_NAME=Flight_Controller

SRC_DIR = Src
BIN_BASE_DIR = Bin
DRIVER_DIR = stm32f0xx_hal_driver
BIN_DIR = $(BIN_BASE_DIR)/$(BINARY_BASE_NAME)
HYBRID_LIB_DIR = common
HYBRID_LIB_SRC = debug.c delay.c debounce.c id_chip.c watchdog.c

ELF_FILE = $(BIN_DIR)/$(BINARY_BASE_NAME).elf
BIN_FILE = $(BIN_DIR)/$(BINARY_BASE_NAME).bin
MAP_FILE = $(BIN_DIR)/$(BINARY_BASE_NAME).map

INCLUDE_DIRS= $(HYBRID_LIB_DIR)/Inc \
			  $(DRIVER_DIR)/Inc \
			  $(DRIVER_DIR)/CMSIS/Device/ST/STM32F0xx/Include \
			  $(DRIVER_DIR)/CMSIS/Include \
			  Inc \
			  $(HYBRID_LIB_DIR)/Inc \
			  $(SRC_DIR)/FreeRTOS/Source/include \
			  $(SRC_DIR)/FreeRTOS/Source/portable/GCC/ARM_CM0 \
			  $(SRC_DIR)/FreeRTOS/Source/CMSIS_RTOS

INCLUDE_FLAGS := $(addprefix -I,$(INCLUDE_DIRS))

DEFINES := "USE_HAL_DRIVER" "STM32F072xB"
DEFINE_FLAGS := $(addprefix -D,$(DEFINES))

LINK_SCRIPT="$(DRIVER_DIR)/STM32F072RB_FLASH.ld"

LINKER_FLAGS=-lc -lnosys -lm -mthumb -mcpu=cortex-m0  -Wl,--gc-sections -T$(LINK_SCRIPT) -static  -Wl,--start-group -lm -Wl,--end-group -Wl,-cref "-Wl,-Map=$(MAP_FILE)" -Wl,--defsym=malloc_getpagesize_P=0x1000

COMMON_FLAGS=-c -g -O2 -mcpu=cortex-m0 -std=gnu99 -Wall -mthumb
ASSEMBLER_FLAGS=$(COMMON_FLAGS) -x assembler-with-cpp
COMPILER_FLAGS=$(COMMON_FLAGS) -ffunction-sections -fdata-sections $(DEFINE_FLAGS)

SRC := $(wildcard $(SRC_DIR)/*.c) \
	   $(addprefix $(HYBRID_LIB_DIR)/Src/, $(HYBRID_LIB_SRC)) \
	   $(wildcard stm32f0xx_hal_driver/Src/*.c) \
	   $(wildcard $(SRC_DIR)/FreeRTOS/Source/*.c) \
	   $(wildcard $(SRC_DIR)/FreeRTOS/Source/portable/GCC/ARM_CM0/*.c) \
	   $(wildcard $(SRC_DIR)/FreeRTOS/Source/portable/MemMang/*.c) \
	   $(wildcard $(SRC_DIR)/FreeRTOS/Source/CMSIS_RTOS/*.c) \
	   stm32f0xx_hal_driver/CMSIS/Device/ST/STM32F0xx/Source/Templates/system_stm32f0xx.c

SRC := $(filter-out $(DRIVER_DIR)/CMSIS/Device/ST/STM32F0xx/Src/stm32f0xx_hal_msp_template.c,$(SRC))

SRCASM := $(DRIVER_DIR)/CMSIS/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f072xb.s

OBJS := $(SRC:%.c=$(BIN_DIR)/%.o) $(SRCASM:%.s=$(BIN_DIR)/%.o)

all: $(OBJ) $(ELF_FILE) $(BIN_FILE)

$(ELF_FILE): $(OBJS)
	$(CC) $(OBJS) $(LINKER_FLAGS) -o "$(ELF_FILE)"

$(BIN_FILE): $(ELF_FILE)
	$(HEX) -O binary "$<" "$@"

load: $(BIN_FILE)
	# this is stand alone stlink
	# openocd -f interface/stlink-v2.cfg -f target/stm32f0x_stlink.cfg -c init -c "reset init" -c halt -c "flash write_image erase $(BIN_FILE) 0x08000000" -c "verify_image $(BIN_FILE)" -c "reset run" -c shutdown
	# this is for nucleo stlink 
	openocd -f interface/stlink-v2-1.cfg -f target/stm32f0x.cfg -c "reset_config srst_only connect_assert_srst" -c init -c "reset halt" -c halt -c "flash write_image erase $(BIN_FILE) 0x08000000" -c "verify_image $(BIN_FILE)" -c "reset run" -c shutdown


connect: $(BIN_FILE)
	# this is stand alone stlink
	# openocd -f interface/stlink-v2.cfg -f target/stm32f0x_stlink.cfg -c init -c "reset init" -c halt -c "flash write_image erase $(BIN_FILE) 0x08000000" -c "verify_image $(BIN_FILE)" -c "reset run" -c shutdown
	# this is for nucleo stlink
	openocd -f interface/stlink-v2-1.cfg -f target/stm32f0x.cfg -c init -c "reset init" -c halt -c "flash write_image erase $(BIN_FILE) 0x08000000" -c "verify_image $(BIN_FILE)" &

debug: connect
	arm-none-eabi-gdb --eval-command="target remote localhost:3333" --eval-command="monitor reset halt" --eval-command="monitor arm semihosting enable"  $(ELF_FILE)

.PHONY: clean test
clean:
	$(RM) $(BIN_BASE_DIR)

test:
	cd test/; make run

$(BIN_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(COMPILER_FLAGS) $(INCLUDE_FLAGS) $< -o $@

$(BIN_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(ASSEMBLER_FLAGS) $(INCLUDE_FLAGS) $< -o $@

$(BIN_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(ASSEMBLER_FLAGS) $(INCLUDE_FLAGS) $< -o $@
