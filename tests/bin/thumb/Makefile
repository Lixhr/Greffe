TARGET   = test
CC       = arm-none-eabi-gcc
OBJCOPY  = arm-none-eabi-objcopy
BUILD    = build
SRC_DIR  = srcs

CFLAGS   = -mcpu=cortex-m3 -mthumb -O1 -ffreestanding -nostdlib -Wall
LDFLAGS  = -T linker.ld -nostdlib
SRCS_FILES = vectors.c startup.s main.c mpu.c fault.c checksum.c
SRCS     = $(addprefix $(SRC_DIR)/, $(SRCS_FILES))

OBJS     = $(addprefix $(BUILD)/, $(addsuffix .o, $(basename $(SRCS_FILES))))

PATCHED  = $(BUILD)/__greffe_workdir/$(TARGET).bin.greffe


.PHONY: all run qemu clean

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).bin

$(BUILD)/%.o: $(SRC_DIR)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o: $(SRC_DIR)/%.s | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary --gap-fill 0xFF --pad-to 0x40000 $< $@

$(BUILD):
	mkdir -p $(BUILD)

run: $(BUILD)/$(TARGET).bin
	bash -c 'trap "stty sane 2>/dev/null" EXIT; qemu-system-arm -machine lm3s6965evb -nographic -device loader,file=$<,addr=0x0'

run-patched:
	bash -c 'trap "stty sane 2>/dev/null" EXIT; qemu-system-arm -machine lm3s6965evb -nographic -device loader,file=$(PATCHED),addr=0x0'

clean:
	rm -rf $(BUILD)
