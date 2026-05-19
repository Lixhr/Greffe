#pragma once
#include <ida.hpp>
#include <netnode.hpp>
#include <cstring>
#include "GreffeCTX.hpp"

#define NODE_NAME "$ Greffe"

void load_db(std::unique_ptr<GreffeCTX> &g_ctx);
void save_db(std::unique_ptr<GreffeCTX> &g_ctx);

enum DB_IDs {
    PatchRegions_entry,
    PatchLayout_entry,
    PatchPlans_entry,
};

struct DB_PatchRegions {
    size_t  n_regions;
    struct  DB_PatchRegions_entry {
        ea_t base;
        ea_t end;
    }   regions[];
};

struct DB_PatchLayout {
    size_t  n_entries;
    struct  DB_PatchLayout_entry {
        ea_t    base;
        ea_t    end;
        PLEType type;
    }   entries[];
};

struct DB_PatchPlan_meta {
    ea_t     target_ea;
    ea_t     target_end_ea;
    ea_t     handler_ptr_addr;
    uint32_t name_len;
    char     name[];
};