######################################
# target — standalone GitHub release tree
# Layout: Driver / User / Util / Middleware / platform  are siblings
######################################
TARGET = awakeno
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
CURR_DIR := $(dir $(mkfile_path))

######################################
# building variables
######################################
DEBUG = 1
OPT = -Og
CFG_CMBACKTRACE = 1

#######################################
# paths (all relative to this Makefile)
#######################################
BUILD_DIR = $(abspath $(CURR_DIR)/out)
PLATFORM  = $(abspath $(CURR_DIR)/platform/gd/gd32e10x)
CMB_DIR   = $(abspath $(CURR_DIR)/Middleware/CmBacktrace)

######################################
# source
######################################
C_SOURCES =  \
User/Src/gd32e10x_it.c \
User/Src/main.c \
User/Src/systick.c \
User/Src/rlk_cdc.c \
User/Src/rlk_cmd.c \
Driver/Src/rlk_gpio.c \
User/Src/usb_delay.c \
User/Src/rlk_key_state.c \
User/Src/uart_adapter.c \
User/Src/global_val.c \
User/Src/nvm.c \
User/Src/function.c \
Driver/Src/i2c.c \
Driver/Src/at24cxx.c \
Driver/Src/mcp4725.c \
Driver/Src/ads1115.c \
Driver/Src/bh1750.c \
Driver/Src/adc_gd32.c \
Driver/Src/audio_dac.c \
Driver/Src/audio_pwm.c \
Util/Src/wave_data.c \
Driver/Src/ir_snd_rcv.c \
Driver/Src/spi_flash.c \
Driver/Src/gt22l16a2y.c \
Driver/Src/i2c_lcd.c \
Driver/Src/fontlib.c \
Driver/Src/i2s_codec.c \
Driver/Src/menu_key.c \
Driver/Src/matrix_key.c \
Driver/Src/drv_uart.c \
Driver/Src/drv_timer.c \
Util/Src/ringbuffer.c \
Util/Src/printf.c \
Util/Src/tone.c \
$(PLATFORM)/sdk/CMSIS/GD/GD32E10x/Source/system_gd32e10x.c \
$(PLATFORM)/bsp/start_up/start.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_adc.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_bkp.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_can.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_crc.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_ctc.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_dac.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_dbg.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_dma.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_exmc.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_exti.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_fmc.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_fwdgt.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_gpio.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_i2c.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_misc.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_pmu.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_rcu.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_rtc.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_spi.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_timer.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_usart.c \
$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Source/gd32e10x_wwdgt.c \
$(PLATFORM)/sdk/GD32E10x_usbfs_library/driver/Source/drv_usb_core.c \
$(PLATFORM)/sdk/GD32E10x_usbfs_library/driver/Source/drv_usb_dev.c \
$(PLATFORM)/sdk/GD32E10x_usbfs_library/driver/Source/drv_usbd_int.c \
$(PLATFORM)/sdk/GD32E10x_usbfs_library/device/core/Source/usbd_core.c \
$(PLATFORM)/sdk/GD32E10x_usbfs_library/device/core/Source/usbd_enum.c \
$(PLATFORM)/sdk/GD32E10x_usbfs_library/device/core/Source/usbd_transc.c \
$(PLATFORM)/sdk/GD32E10x_usbfs_library/device/class/cdc/Source/cdc_acm_core.c

ASM_SOURCES =  \
$(PLATFORM)/bsp/start_up/startup_gd32e103xb.s

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
CPU = -mcpu=cortex-m4
FPU = -mfpu=fpv4-sp-d16
FLOAT-ABI = -mfloat-abi=softfp
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

AS_DEFS =
C_DEFS =  \
-DUSE_STDPERIPH_DRIVER \
-DUSE_USB_FS \
-DGD32E10X \
-DGD32E103V_EVAL

AS_INCLUDES =

C_INCLUDES = \
-I$(PLATFORM)/sdk/CMSIS/GD/GD32E10x/Include \
-I$(PLATFORM)/sdk/CMSIS \
-I$(PLATFORM)/sdk/GD32E10x_standard_peripheral/Include \
-I$(PLATFORM)/sdk/GD32E10x_usbfs_library/driver/Include \
-I$(PLATFORM)/sdk/GD32E10x_usbfs_library/device/class/cdc/Include \
-I$(PLATFORM)/sdk/GD32E10x_usbfs_library/device/core/Include \
-I$(PLATFORM)/sdk/GD32E10x_usbfs_library/ustd/class/cdc \
-I$(PLATFORM)/sdk/GD32E10x_usbfs_library/ustd/common \
-I$(CMB_DIR)/cm_backtrace \
-I$(CMB_DIR)/cm_backtrace/Languages/en-US \
-IUser/Inc \
-IDriver/Inc \
-IUtil/Inc \
-I$(CMB_DIR)

ifeq ($(CFG_CMBACKTRACE), 1)
C_SOURCES +=  \
$(CMB_DIR)/cm_backtrace/cm_backtrace.c
ASM_SOURCES +=  \
$(CMB_DIR)/cm_backtrace/fault_handler/gcc/cmb_fault.s
C_DEFS +=  \
-DUSE_CMBACKTRACE
endif

ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################
LDSCRIPT = $(PLATFORM)/bsp/start_up/gd32e103xb.ld
LIBS = -lc -lm -lnosys -l:GT22L16A2Y.a
LIBDIR = -L$(abspath $(CURR_DIR)/Driver/Lib)
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

#######################################
# build
#######################################
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/wave_data.o: CFLAGS += -Os
$(BUILD_DIR)/i2s_codec.o: CFLAGS += -Os
$(BUILD_DIR)/audio_dac.o: CFLAGS += -Os
$(BUILD_DIR)/audio_pwm.o: CFLAGS += -Os
$(BUILD_DIR)/rlk_cmd.o: CFLAGS += -Os
$(BUILD_DIR)/main.o: CFLAGS += -Os

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir -p "$@" 2>/dev/null || mkdir "$@"

#######################################
# clean — empty out/, keep the directory
#######################################
clean:
ifeq ($(OS),Windows_NT)
	-cmd /c "del /q /f $(subst /,\,$(BUILD_DIR))\* >nul 2>&1"
else
	-rm -f $(BUILD_DIR)/*
endif

-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
