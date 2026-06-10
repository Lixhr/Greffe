#pragma once

#include "ida.hpp"
#include <string>
#include <vector>

void greffe_set_region(ea_t start, ea_t end);
void greffe_add(ea_t ea, const std::string &handler_name = {});
void greffe_delete(ea_t ea);
void greffe_clear_all();
void greffe_create_stubs();
void greffe_create_named_stub(const std::string &name);
void greffe_delete_handler(const std::string &name);
std::vector<std::string> greffe_list_handlers();
void greffe_apply_patches();
