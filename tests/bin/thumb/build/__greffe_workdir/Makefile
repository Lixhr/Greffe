CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
CFLAGS  := -fno-pic -nostdlib -Os -ffunction-sections -fdata-sections -Ttext=0x0

NON_GREFFE_SRCS := $(filter-out %.greffe.c,$(wildcard handlers/*.c))
-include greffe_active.mk
SRCS := $(NON_GREFFE_SRCS) $(ACTIVE_GREFFE_SRCS)
OBJS := $(patsubst handlers/%.c,build/%.o,$(SRCS))

all: build/handlers.bin build/handlers.elf

build/handlers.elf: $(OBJS) | build
	$(CC) $(CFLAGS) -Wl,-Map=build/handlers.map -o $@ $^

build/handlers.bin: build/handlers.elf | build
	$(OBJCOPY) -O binary $< $@

build/%.o: handlers/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

.PHONY: all
