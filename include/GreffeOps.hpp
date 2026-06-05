#pragma once

#include "ida.hpp"
#include <string>

void greffe_set_region(ea_t start, ea_t end);
void greffe_add_instr(ea_t ea, const std::string &handler_name = {});
void greffe_apply_patches();
