CC      = arm-linux-gnueabihf-gcc
CC_BE   = toolchains/armeb_linux/bin/armeb-linux-gnueabihf-gcc
SRC_DIR = srcs
SRCS    = $(SRC_DIR)/main.c $(SRC_DIR)/checksum.c

CFLAGS_COMMON   = -O1 -Wall -static -no-pie
CFLAGS_ARM      = $(CFLAGS_COMMON) -marm
CFLAGS_THUMB    = $(CFLAGS_COMMON) -mthumb
CFLAGS_THUMB_BE = $(CFLAGS_COMMON) -mthumb -mbig-endian

BUILD_ARM      = arm32/build
BUILD_THUMB    = arm32-thumb/build
BUILD_THUMB_BE = arm32-thumb-be/build

OBJS_ARM      = $(addprefix $(BUILD_ARM)/,      $(notdir $(SRCS:.c=.o)))
OBJS_THUMB    = $(addprefix $(BUILD_THUMB)/,    $(notdir $(SRCS:.c=.o)))
OBJS_THUMB_BE = $(addprefix $(BUILD_THUMB_BE)/, $(notdir $(SRCS:.c=.o)))

all: $(BUILD_ARM)/test $(BUILD_THUMB)/test $(BUILD_THUMB_BE)/test

$(BUILD_ARM)/%.o: $(SRC_DIR)/%.c | $(BUILD_ARM)
	$(CC) $(CFLAGS_ARM) -c -o $@ $<

$(BUILD_THUMB)/%.o: $(SRC_DIR)/%.c | $(BUILD_THUMB)
	$(CC) $(CFLAGS_THUMB) -c -o $@ $<

$(BUILD_THUMB_BE)/%.o: $(SRC_DIR)/%.c | $(BUILD_THUMB_BE)
	$(CC_BE) $(CFLAGS_THUMB_BE) -c -o $@ $<

$(BUILD_ARM)/test: $(OBJS_ARM)
	$(CC) $(CFLAGS_ARM) -o $@ $^

$(BUILD_THUMB)/test: $(OBJS_THUMB)
	$(CC) $(CFLAGS_THUMB) -o $@ $^

$(BUILD_THUMB_BE)/test: $(OBJS_THUMB_BE)
	$(CC_BE) $(CFLAGS_THUMB_BE) -o $@ $^

$(BUILD_ARM) $(BUILD_THUMB) $(BUILD_THUMB_BE):
	mkdir -p $@

run: all
	qemu-arm $(BUILD_ARM)/test
	qemu-arm $(BUILD_THUMB)/test
	qemu-armeb $(BUILD_THUMB_BE)/test

clean:
	rm -rf $(BUILD_ARM) $(BUILD_THUMB) $(BUILD_THUMB_BE)

re: clean all

test:
	~/.ida-pro-9.2/idat -L/tmp/greffe_test.log -A -S/home/cbeaufil/greffe/tests/test.py ~/greffe/tests/arm32-thumb/build/test
	cat /tmp/greffe_test.log

	
.PHONY: all run clean re
