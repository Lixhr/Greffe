PREFIX  := arm-none-eabi-
CC      := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump

MODE    ?= trace
SRC_DIR := $(MODE)
BUILD   := $(MODE)/build
TARGET  := $(BUILD)/payload

SRCS_C := $(wildcard $(SRC_DIR)/*.c)
SRCS_S := $(wildcard $(SRC_DIR)/*.s)
OBJS   := $(patsubst $(SRC_DIR)/%.c, $(BUILD)/%.o, $(SRCS_C)) \
          $(patsubst $(SRC_DIR)/%.s, $(BUILD)/%.o, $(SRCS_S))

CFLAGS := \
    $(CPU_SPECIFIC)\
    -fPIC \
    -ffreestanding \
    -nostdlib -nostartfiles \
    -fno-stack-protector \
    -fdata-sections -ffunction-sections \
    -Os

LDFLAGS := \
    $(CPU_SPECIFIC) \
    -T link.ld \
    -nostdlib -nostartfiles \
    -Wl,-N

all: $(TARGET).bin

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SRC_DIR)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: $(SRC_DIR)/%.s | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS) | $(BUILD)
	$(CC) $(LDFLAGS) $^ -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(MODE)/build

disasm:
	$(OBJDUMP) -d $(TARGET).elf

raw_disasm:
	$(OBJDUMP) -b binary -m arm -M force-thumb -D $(TARGET).bin

.PHONY: all clean