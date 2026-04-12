#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace MakefileTemplates {

static const std::pair<std::string_view, std::string_view> table[] = {
    { "ARM", R"makefile(CC      := arm-none-eabi-gcc
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
)makefile" },

    { "arm64", R"makefile(CC      := aarch64-none-elf-gcc
OBJCOPY := aarch64-none-elf-objcopy
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
)makefile" },
};

inline std::string_view get(const std::string& arch) {
    for (const auto& [a, content] : table)
        if (a == arch) return content;

    throw std::runtime_error("MakefileTemplates: no template for arch: " + arch);
}

} // namespace MakefileTemplates
