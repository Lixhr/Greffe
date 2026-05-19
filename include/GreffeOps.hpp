#pragma once

#include "ida.hpp"

void greffe_set_region(ea_t start, ea_t end);
void greffe_add_instr(ea_t ea);
void greffe_apply_patches();
