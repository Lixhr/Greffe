#pragma once
#include <ida.hpp>
#include <netnode.hpp>
#include <cstring>
#include "GreffeCTX.hpp"

#define NODE_NAME "$ Greffe"

void load_db(std::unique_ptr<GreffeCTX> &g_ctx);
void save_db(std::unique_ptr<GreffeCTX> &g_ctx);

enum DB_IDs {
    PatchRegions_entry
};

struct DB_PatchRegions {
    size_t  n_regions;
    struct DB_PatchRegions_entry {
        ea_t base;
        ea_t end;
    }   regions[];
};