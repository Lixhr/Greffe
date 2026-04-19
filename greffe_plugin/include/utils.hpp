#pragma once

#include "ida.hpp"
#include "kernwin.hpp"
#include <cstdint>
#include <cstddef>
#include <string>
#include "PatchLayout.hpp"
#include <filesystem>

#define greffe_msg(fmt, ...) msg("[greffe] " fmt, ##__VA_ARGS__)

namespace Color {
    // ok
    constexpr bgcolor_t RELOCATED    = 0xff0595ff;
    constexpr bgcolor_t PATCHED      = 0xff27b6ff;
    constexpr bgcolor_t PATCH_REGION = 0xff71c9ff;
    constexpr bgcolor_t HANDLER_CODE = 0xff75ae6c;
    constexpr bgcolor_t TARGET       = RELOCATED;
}

void region_bzero(ea_t ea, size_t size);
void set_code_region(ea_t start, ea_t end);

void write_code_patch(ea_t ea, const uint8_t *bytes, ea_t end_ea);
void write_data_patch(ea_t addr, const uint8_t *bytes, size_t size);
void commit_gui(PatchLayout &layout);

void init_color_hook();
void term_color_hook();

void workdir_popup(const std::filesystem::path absolute,
                   const std::string relative);