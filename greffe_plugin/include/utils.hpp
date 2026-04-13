#pragma once

#include "kernwin.hpp"
#include "ida.hpp"

#define greffe_msg(fmt, ...) msg("[greffe] " fmt, ##__VA_ARGS__)

namespace Color {
    constexpr bgcolor_t RELOCATED    = 0xff3b0e32;
    constexpr bgcolor_t PATCHED      = 0xff852a4c;
    constexpr bgcolor_t PATCH_REGION = 0xff8A84E2;
    constexpr bgcolor_t HANDLER_CODE = 0xff4a5235;
    constexpr bgcolor_t TARGET       = RELOCATED;
}

void set_range_color(ea_t start, ea_t end, bgcolor_t color);

void clear_all_range_colors();