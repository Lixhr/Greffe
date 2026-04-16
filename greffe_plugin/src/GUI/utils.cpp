#include "utils.hpp"
#include "ua.hpp"
#include "bytes.hpp"
#include <lines.hpp>
#include "offset.hpp"

void set_code_region(ea_t start, ea_t end) {
    if (start == end)
        return;

    while (start < end) {
        int size = create_insn(start);
        if (size == 0) start++;
        else           start += size;
    }
}

void write_code_patch(ea_t addr, const uint8_t *bytes, size_t size, bgcolor_t color) {
    del_items(addr, DELIT_SIMPLE, size);
    patch_bytes(addr, bytes, size);
    set_range_color(addr, addr + size, color);
    set_code_region(addr, addr + size);
}

void write_data_patch(ea_t addr, const uint8_t *bytes, size_t size, bgcolor_t color) {
    del_items(addr, DELIT_SIMPLE, size);
    patch_bytes(addr, bytes, size);
    set_range_color(addr, addr + size, color);
}