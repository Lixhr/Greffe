#pragma once

#include "ida.hpp"
#include "kernwin.hpp"
#include <cstdint>
#include <cstddef>

#define greffe_msg(fmt, ...) msg("[greffe] " fmt, ##__VA_ARGS__)

namespace Color {
    constexpr bgcolor_t RELOCATED    = 0xff3b0e32 & 0xff0a0a0a;
    constexpr bgcolor_t PATCHED      = 0xff852a4c & 0xff0a0a0a;
    constexpr bgcolor_t PATCH_REGION = 0xff8A84E2 & 0xff0a0a0a;
    constexpr bgcolor_t HANDLER_CODE = 0xff4a5235 & 0xff0a0a0a;
    constexpr bgcolor_t TARGET       = RELOCATED;
}

void set_range_color(ea_t start, ea_t end, bgcolor_t color);
void clear_all_range_colors();
void region_bzero(ea_t ea, size_t size);
void set_code_region(ea_t start, ea_t end);

void write_code_patch(ea_t addr, const uint8_t *bytes, size_t size, bgcolor_t color);
void write_data_patch(ea_t addr, const uint8_t *bytes, size_t size, bgcolor_t color);