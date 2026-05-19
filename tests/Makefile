CC_ARM        = arm-linux-gnueabihf-gcc
CC_ARM_BE     = toolchains/arm-be_linux/bin/armeb-linux-gnueabihf-gcc
CC_AARCH64    = toolchains/aarch64_linux/bin/aarch64-none-linux-gnu-gcc
CC_AARCH64_BE = toolchains/aarch64-be_linux/bin/aarch64_be-none-linux-gnu-gcc
SRC_DIR = srcs
SRCS    = $(SRC_DIR)/main.c $(SRC_DIR)/checksum.c

CFLAGS_COMMON     = -O1 -Wall -static -no-pie
CFLAGS_ARM        = $(CFLAGS_COMMON) -marm
CFLAGS_ARM_BE     = $(CFLAGS_COMMON) -marm -mbig-endian
CFLAGS_THUMB      = $(CFLAGS_COMMON) -mthumb
CFLAGS_THUMB_BE   = $(CFLAGS_COMMON) -mthumb -mbig-endian
CFLAGS_AARCH64    = $(CFLAGS_COMMON)
CFLAGS_AARCH64_BE = $(CFLAGS_COMMON)

BUILD_ARM        = arm32/build
BUILD_ARM_BE     = arm32-be/build
BUILD_THUMB      = arm32-thumb/build
BUILD_THUMB_BE   = arm32-thumb-be/build
BUILD_AARCH64    = aarch64-le/build
BUILD_AARCH64_BE = aarch64-be/build

OBJS_ARM      = $(addprefix $(BUILD_ARM)/,      $(notdir $(SRCS:.c=.o)))
OBJS_ARM_BE   = $(addprefix $(BUILD_ARM_BE)/,   $(notdir $(SRCS:.c=.o)))
OBJS_THUMB    = $(addprefix $(BUILD_THUMB)/,    $(notdir $(SRCS:.c=.o)))
OBJS_THUMB_BE = $(addprefix $(BUILD_THUMB_BE)/, $(notdir $(SRCS:.c=.o)))
OBJS_AARCH64    = $(addprefix $(BUILD_AARCH64)/,    $(notdir $(SRCS:.c=.o)))
OBJS_AARCH64_BE = $(addprefix $(BUILD_AARCH64_BE)/, $(notdir $(SRCS:.c=.o)))

all: $(BUILD_ARM)/test \
	 $(BUILD_ARM_BE)/test \
	 $(BUILD_THUMB)/test \
	 $(BUILD_THUMB_BE)/test \
	 $(BUILD_AARCH64)/test \
	 $(BUILD_AARCH64_BE)/test

$(BUILD_ARM)/%.o: $(SRC_DIR)/%.c | $(BUILD_ARM)
	$(CC_ARM) $(CFLAGS_ARM) -c -o $@ $<

$(BUILD_ARM_BE)/%.o: $(SRC_DIR)/%.c | $(BUILD_ARM_BE)
	$(CC_ARM_BE) $(CFLAGS_ARM_BE) -c -o $@ $<

$(BUILD_THUMB)/%.o: $(SRC_DIR)/%.c | $(BUILD_THUMB)
	$(CC_ARM) $(CFLAGS_THUMB) -c -o $@ $<

$(BUILD_THUMB_BE)/%.o: $(SRC_DIR)/%.c | $(BUILD_THUMB_BE)
	$(CC_ARM_BE) $(CFLAGS_THUMB_BE) -c -o $@ $<

$(BUILD_AARCH64)/%.o: $(SRC_DIR)/%.c | $(BUILD_AARCH64)
	$(CC_AARCH64) $(CFLAGS_AARCH64) -c -o $@ $<

$(BUILD_AARCH64_BE)/%.o: $(SRC_DIR)/%.c | $(BUILD_AARCH64_BE)
	$(CC_AARCH64_BE) $(CFLAGS_AARCH64_BE) -c -o $@ $<

$(BUILD_ARM)/test: $(OBJS_ARM)
	$(CC_ARM) $(CFLAGS_ARM) -o $@ $^

$(BUILD_ARM_BE)/test: $(OBJS_ARM_BE)
	$(CC_ARM_BE) $(CFLAGS_ARM_BE) -o $@ $^

$(BUILD_THUMB)/test: $(OBJS_THUMB)
	$(CC_ARM) $(CFLAGS_THUMB) -o $@ $^

$(BUILD_THUMB_BE)/test: $(OBJS_THUMB_BE)
	$(CC_ARM_BE) $(CFLAGS_THUMB_BE) -o $@ $^

$(BUILD_AARCH64)/test: $(OBJS_AARCH64)
	$(CC_AARCH64) $(CFLAGS_AARCH64) -o $@ $^

$(BUILD_AARCH64_BE)/test: $(OBJS_AARCH64_BE)
	$(CC_AARCH64_BE) $(CFLAGS_AARCH64_BE) -o $@ $^


$(BUILD_ARM) $(BUILD_ARM_BE) $(BUILD_THUMB) $(BUILD_THUMB_BE) $(BUILD_AARCH64) $(BUILD_AARCH64_BE):
	mkdir -p $@

clean:
	rm -rf $(BUILD_ARM) $(BUILD_ARM_BE) $(BUILD_THUMB) $(BUILD_THUMB_BE) $(BUILD_AARCH64) $(BUILD_AARCH64_BE)

re: clean all

test:
	~/.ida-pro-9.2/idat -L/tmp/greffe_test.log  -P- -A -S/home/cbeaufil/greffe/tests/test.py ~/greffe/tests/arm32/build/test
	cat /tmp/greffe_test.log

	
.PHONY: all run clean re
