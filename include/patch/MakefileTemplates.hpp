#pragma once

#include "patch/arch/StubsFactory.hpp"
#include <string>
#include <string_view>

namespace MakefileTemplates {

// Renders a Makefile for the given arch descriptor.
inline std::string render(const ArchDescriptor& d) {
    std::string discard = "*(.note*) *(.comment*)";
    if (!d.ld_discard.empty()) {
        discard = std::string(d.ld_discard) + " " + discard;
    }

    std::string cflags = "-fno-pic -nostdlib -Os -ffunction-sections -fdata-sections";
    if (!d.cflags_extra.empty()) {
        cflags += " ";
        cflags += d.cflags_extra;
    }

    return std::string("CC      := ") + std::string(d.cc) + "\n"
         + "OBJCOPY := " + std::string(d.objcopy) + "\n"
         + "CFLAGS  := " + cflags + "\n"
         + "LDSCRIPT := build/handlers.ld\n"
         + "\n"
         + "NON_GREFFE_SRCS := $(filter-out %_greffe.c,$(wildcard handlers/*.c))\n"
         + "-include greffe_active.mk\n"
         + "SRCS := $(NON_GREFFE_SRCS) $(ACTIVE_GREFFE_SRCS)\n"
         + "OBJS := $(patsubst handlers/%.c,build/%.o,$(SRCS))\n"
         + "\n"
         + "all: build/handlers.bin build/handlers.elf\n"
         + "\n"
         + "$(LDSCRIPT): | build\n"
         + "\t@printf 'SECTIONS {\\n  . = 0x0;\\n  .text   : { *(.text*) }\\n"
           "  .rodata : { *(.rodata*) }\\n  .data   : { *(.data*) }\\n"
           "  .bss    : { *(.bss*) }\\n"
           "  /DISCARD/ : { " + discard + " }\\n}\\n' > $@\n"
         + "\n"
         + "build/handlers.elf: $(OBJS) $(LDSCRIPT) | build\n"
         + "\t$(CC) $(CFLAGS) -T $(LDSCRIPT) -Wl,-Map=build/handlers.map -o $@ $(OBJS)\n"
         + "\n"
         + "build/handlers.bin: build/handlers.elf | build\n"
         + "\t$(OBJCOPY) -j .text -j .rodata -j .data -O binary $< $@\n"
         + "\n"
         + "build/%.o: handlers/%.c | build\n"
         + "\t$(CC) $(CFLAGS) -c $< -o $@\n"
         + "\n"
         + "build:\n"
         + "\tmkdir -p build\n"
         + "\n"
         + "clean:\n"
         + "\trm -rf build\n"
         + "\n"
         + "re: clean all\n"
         + "\n"
         + ".PHONY: all clean re\n";
}

inline std::string get(int bits, const std::string& arch, const std::string& endianness) {
    const ArchDescriptor* d = StubsFactory::findDescriptor(bits, arch, endianness);
    if (!d)
        throw std::runtime_error("MakefileTemplates: no descriptor for arch: " + arch);
    return render(*d);
}

} // namespace MakefileTemplates
