CC=arm-none-eabi-gcc
HEX=arm-none-eabi-objcopy

RM=rm -rf
BINARY_BASE_NAME=Flight_Controller

SRC_DIR = Src
BIN_BASE_DIR = Bin
DRIVER_DIR = stm32f4xx_hal_driver
BIN_DIR = $(BIN_BASE_DIR)/$(BINARY_BASE_NAME)
COMMON_LIB_DIR = common
#COMMON_LIB_SRC = debug.c delay.c debounce.c id_chip.c watchdog.c
COMMON_LIB_SRC = debug.c

ELF_FILE = $(BIN_DIR)/$(BINARY_BASE_NAME).elf
BIN_FILE = $(BIN_DIR)/$(BINARY_BASE_NAME).bin
MAP_FILE = $(BIN_DIR)/$(BINARY_BASE_NAME).map

DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td


INCLUDE_DIRS= $(COMMON_LIB_DIR)/Inc \
			  $(DRIVER_DIR)/Inc \
			  $(DRIVER_DIR)/CMSIS/Device/ST/STM32F4xx/Include \
			  $(DRIVER_DIR)/CMSIS/Include \
			  Inc \
			  $(SRC_DIR)/FreeRTOS/Source/include \
			  $(SRC_DIR)/FreeRTOS/Source/portable/GCC/ARM_CM4F \
			  $(SRC_DIR)/FreeRTOS/Source/CMSIS_RTOS \
			  $(SRC_DIR)/FatFs/src

INCLUDE_FLAGS := $(addprefix -I,$(INCLUDE_DIRS))


DEFINES := "USE_HAL_DRIVER" "STM32F410Rx" $(if $(TARGET), $(TARGET), FC)
DEFINE_FLAGS := $(addprefix -D,$(DEFINES))

LINK_SCRIPT="$(DRIVER_DIR)/STM32F410RBTx_FLASH.ld"

# -l is to link a library
#  -lc links libc.a, the c std lib
#  -lnosys links libnosys.a, has some stubbed system calls?
#  -lm links libm.a, the math library
#  -mthumb selects the thumb instruction set
#  -mcpu=cortex-m4 selects the cortex-m4 processor
#  -Wl--gc-sections passes --gc-sections to the linker, which means it only links used data and functions, and discards the rest
#  -T specifies the link script to use
#  -static On systems that support dynamic linking, this prevents linking with the shared libraries. On other systems, this option has no effect.
#  -Wl,--start-group -m -Wl,--end-group creates a group that is searched repeatedly for circular dependencies until no new undefined references are created
#  --cref, Output a cross reference table. If a linker map file is being generated, the cross reference table is printed to the map file
# -Wl,--defsym=malloc_getpagesize_P=0x1000, set the default page size of malloc to 0x1000, which means the heap increases in size by 4096 bytes at a time
LINKER_FLAGS=-lc -lnosys -lm -mthumb -mcpu=cortex-m4  -Wl,--gc-sections -T$(LINK_SCRIPT) -static  -Wl,--start-group -lm -Wl,--end-group -Wl,-cref "-Wl,-Map=$(MAP_FILE)" -Wl,--defsym=malloc_getpagesize_P=0x1000 -mfloat-abi=hard -mfpu=fpv4-sp-d16

COMMON_FLAGS=-c -g -O2 -mcpu=cortex-m4 -std=gnu99 -Wall -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
#COMMON_FLAGS=-c -g -O2 -mcpu=cortex-m4 -std=gnu99 -Wall -mthumb -mfloat-abi=softfp
ASSEMBLER_FLAGS=$(COMMON_FLAGS) -x assembler-with-cpp

# -ffunction-sections and -fdata-sections, Place each function or data item into its own section in the output file
#  This is to allow linking only used functions and data
COMPILER_FLAGS=$(COMMON_FLAGS) -ffunction-sections -fdata-sections $(DEFINE_FLAGS) -Werror $(DEPFLAGS)
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

SRC := $(wildcard $(SRC_DIR)/*.c) \
	   $(wildcard stm32f4xx_hal_driver/Src/*.c) \
	   $(wildcard $(SRC_DIR)/FreeRTOS/Source/*.c) \
	   $(wildcard $(SRC_DIR)/FreeRTOS/Source/portable/GCC/ARM_CM4F/*.c) \
	   $(addprefix $(COMMON_LIB_DIR)/Src/, $(COMMON_LIB_SRC)) \
	   $(SRC_DIR)/FreeRTOS/Source/portable/MemMang/heap_4.c \
	   $(wildcard $(SRC_DIR)/FreeRTOS/Source/CMSIS_RTOS/*.c) \
	   $(SRC_DIR)/FatFs/src/ff.c \
	   stm32f4xx_hal_driver/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c

SRC := $(filter-out $(DRIVER_DIR)/CMSIS/Device/ST/STM32F4xx/Src/stm32f4xx_hal_msp_template.c,$(SRC))
SRC := $(filter-out $(DRIVER_DIR)/Src/stm32f4xx_hal_timebase_rtc_alarm_template.c,$(SRC)) # This seems to be some template file that needs to be modified to be used
SRC := $(filter-out $(DRIVER_DIR)/Src/stm32f4xx_hal_timebase_rtc_wakeup_template.c,$(SRC)) # This seems to be some template file that needs to be modified to be used
SRC := $(filter-out $(DRIVER_DIR)/Src/stm32f4xx_hal_timebase_tim_template.c,$(SRC)) # This seems to be some template file that needs to be modified to be used

SRCASM := $(DRIVER_DIR)/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f410rx.s

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
	openocd -f interface/stlink-v2-1.cfg -f target/stm32f4x.cfg -c "reset_config srst_only connect_assert_srst" -c init -c "reset halt" -c halt -c "flash write_image erase $(BIN_FILE) 0x08000000" -c "verify_image $(BIN_FILE)" -c "reset run" -c shutdown


connect: $(BIN_FILE)
	# this is stand alone stlink
	# openocd -f interface/stlink-v2.cfg -f target/stm32f0x_stlink.cfg -c init -c "reset init" -c halt -c "flash write_image erase $(BIN_FILE) 0x08000000" -c "verify_image $(BIN_FILE)" -c "reset run" -c shutdown
	# this is for nucleo stlink
	openocd -f interface/stlink-v2-1.cfg -f target/stm32f4x.cfg -c init -c "reset init" -c halt -c "flash write_image erase $(BIN_FILE) 0x08000000" -c "verify_image $(BIN_FILE)" &

debug: connect
	arm-none-eabi-gdb --eval-command="target remote localhost:3333" --eval-command="monitor reset halt" --eval-command="monitor arm semihosting enable"  $(ELF_FILE)

.PHONY: clean test
clean:
	$(RM) $(BIN_BASE_DIR)
	$(RM) $(DEPDIR)

test:
	cd test/; make run

$(BIN_DIR)/%.o: %.c
$(BIN_DIR)/%.o: %.c $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(DEPDIR)/$^)
	$(CC) $(COMPILER_FLAGS) $(INCLUDE_FLAGS) $< -o $@
	$(POSTCOMPILE)

$(BIN_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(ASSEMBLER_FLAGS) $(INCLUDE_FLAGS) $< -o $@

$(BIN_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(ASSEMBLER_FLAGS) $(INCLUDE_FLAGS) $< -o $@

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d


include $(wildcard $(patsubst %,$(DEPDIR)/%.d, $(basename $(SRC))))
