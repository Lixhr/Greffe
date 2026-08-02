TOOLCHAIN_DIR = /opt/toolchains
CC_ARM        = $(TOOLCHAIN_DIR)/arm-le_linux/bin/arm-linux-gcc
CC_ARM_BE     = $(TOOLCHAIN_DIR)/arm-be_linux/bin/armeb-linux-gcc
CC_AARCH64    = $(TOOLCHAIN_DIR)/aarch64-le_linux/bin/aarch64-none-linux-gnu-gcc
CC_AARCH64_BE = $(TOOLCHAIN_DIR)/aarch64-be_linux/bin/aarch64_be-none-linux-gnu-gcc
SRC_DIR 	  = srcs
SRCS    	  = $(SRC_DIR)/main.c $(SRC_DIR)/checksum.c

CFLAGS_COMMON     = -O1 -Wall -static -no-pie
CFLAGS_ARM        = $(CFLAGS_COMMON) -marm
CFLAGS_ARM_BE     = $(CFLAGS_COMMON) -marm -mbig-endian
CFLAGS_THUMB      = $(CFLAGS_COMMON) -mthumb
CFLAGS_THUMB_BE   = $(CFLAGS_COMMON) -mthumb -mbig-endian
CFLAGS_AARCH64    = $(CFLAGS_COMMON)
CFLAGS_AARCH64_BE = $(CFLAGS_COMMON)

ARCH_ARM        = arm32
ARCH_ARM_BE     = arm32-be
ARCH_THUMB      = arm32-thumb
ARCH_THUMB_BE   = arm32-thumb-be
ARCH_AARCH64    = aarch64-le
ARCH_AARCH64_BE = aarch64-be

OBJDIR_ARM        = /tmp/$(ARCH_ARM)/build
OBJDIR_ARM_BE     = /tmp/$(ARCH_ARM_BE)/build
OBJDIR_THUMB      = /tmp/$(ARCH_THUMB)/build
OBJDIR_THUMB_BE   = /tmp/$(ARCH_THUMB_BE)/build
OBJDIR_AARCH64    = /tmp/$(ARCH_AARCH64)/build
OBJDIR_AARCH64_BE = /tmp/$(ARCH_AARCH64_BE)/build

BUILD_DIR = build

BIN_ARM        = $(BUILD_DIR)/$(ARCH_ARM)-test
BIN_ARM_BE     = $(BUILD_DIR)/$(ARCH_ARM_BE)-test
BIN_THUMB      = $(BUILD_DIR)/$(ARCH_THUMB)-test
BIN_THUMB_BE   = $(BUILD_DIR)/$(ARCH_THUMB_BE)-test
BIN_AARCH64    = $(BUILD_DIR)/$(ARCH_AARCH64)-test
BIN_AARCH64_BE = $(BUILD_DIR)/$(ARCH_AARCH64_BE)-test

OBJS_ARM      = $(addprefix $(OBJDIR_ARM)/,      $(notdir $(SRCS:.c=.o)))
OBJS_ARM_BE   = $(addprefix $(OBJDIR_ARM_BE)/,   $(notdir $(SRCS:.c=.o)))
OBJS_THUMB    = $(addprefix $(OBJDIR_THUMB)/,    $(notdir $(SRCS:.c=.o)))
OBJS_THUMB_BE = $(addprefix $(OBJDIR_THUMB_BE)/, $(notdir $(SRCS:.c=.o)))
OBJS_AARCH64    = $(addprefix $(OBJDIR_AARCH64)/,    $(notdir $(SRCS:.c=.o)))
OBJS_AARCH64_BE = $(addprefix $(OBJDIR_AARCH64_BE)/, $(notdir $(SRCS:.c=.o)))

all: $(BIN_ARM) \
	 $(BIN_ARM_BE) \
	 $(BIN_THUMB) \
	 $(BIN_THUMB_BE) \
	 $(BIN_AARCH64) \
	 $(BIN_AARCH64_BE)
	 @echo "> Build successful."

$(OBJDIR_ARM)/%.o: $(SRC_DIR)/%.c | $(OBJDIR_ARM)
	$(CC_ARM) $(CFLAGS_ARM) -c -o $@ $<

$(OBJDIR_ARM_BE)/%.o: $(SRC_DIR)/%.c | $(OBJDIR_ARM_BE)
	$(CC_ARM_BE) $(CFLAGS_ARM_BE) -c -o $@ $<

$(OBJDIR_THUMB)/%.o: $(SRC_DIR)/%.c | $(OBJDIR_THUMB)
	$(CC_ARM) $(CFLAGS_THUMB) -c -o $@ $<

$(OBJDIR_THUMB_BE)/%.o: $(SRC_DIR)/%.c | $(OBJDIR_THUMB_BE)
	$(CC_ARM_BE) $(CFLAGS_THUMB_BE) -c -o $@ $<

$(OBJDIR_AARCH64)/%.o: $(SRC_DIR)/%.c | $(OBJDIR_AARCH64)
	$(CC_AARCH64) $(CFLAGS_AARCH64) -c -o $@ $<

$(OBJDIR_AARCH64_BE)/%.o: $(SRC_DIR)/%.c | $(OBJDIR_AARCH64_BE)
	$(CC_AARCH64_BE) $(CFLAGS_AARCH64_BE) -c -o $@ $<

$(BIN_ARM): $(OBJS_ARM) | $(BUILD_DIR)
	$(CC_ARM) $(CFLAGS_ARM) -o $@ $^

$(BIN_ARM_BE): $(OBJS_ARM_BE) | $(BUILD_DIR)
	$(CC_ARM_BE) $(CFLAGS_ARM_BE) -o $@ $^

$(BIN_THUMB): $(OBJS_THUMB) | $(BUILD_DIR)
	$(CC_ARM) $(CFLAGS_THUMB) -o $@ $^

$(BIN_THUMB_BE): $(OBJS_THUMB_BE) | $(BUILD_DIR)
	$(CC_ARM_BE) $(CFLAGS_THUMB_BE) -o $@ $^

$(BIN_AARCH64): $(OBJS_AARCH64) | $(BUILD_DIR)
	$(CC_AARCH64) $(CFLAGS_AARCH64) -o $@ $^

$(BIN_AARCH64_BE): $(OBJS_AARCH64_BE) | $(BUILD_DIR)
	$(CC_AARCH64_BE) $(CFLAGS_AARCH64_BE) -o $@ $^


$(OBJDIR_ARM) $(OBJDIR_ARM_BE) $(OBJDIR_THUMB) $(OBJDIR_THUMB_BE) $(OBJDIR_AARCH64) $(OBJDIR_AARCH64_BE) $(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(ARCH_ARM) $(ARCH_ARM_BE) $(ARCH_THUMB) $(ARCH_THUMB_BE) $(ARCH_AARCH64) $(ARCH_AARCH64_BE) $(BUILD_DIR)

re: clean all

.PHONY: all run clean re
