#include <ida.hpp>
#include <expr.hpp>
#include "GreffeOps.hpp"
#include "GUI/GreffePanel.hpp"
#include "GreffeCTX.hpp"
#include "utils.hpp"

static error_t idaapi idc_set_region(idc_value_t *argv, idc_value_t *res) {
    try {
        greffe_set_region((ea_t)argv[0].num, (ea_t)argv[1].num);
        res->set_long(1);
    } catch (const std::exception &e) {
        greffe_msg("GreffSetRegion error: %s\n", e.what());
        res->set_long(0);
    }
    return eOk;
}

static error_t idaapi idc_add_instr(idc_value_t *argv, idc_value_t *res) {
    try {
        greffe_add_instr((ea_t)argv[0].num);
        res->set_long(1);
    } catch (const std::exception &e) {
        greffe_msg("GreffAddInstr error: %s\n", e.what());
        res->set_long(0);
    }
    return eOk;
}

static error_t idaapi idc_add_instr_ex(idc_value_t *argv, idc_value_t *res) {
    try {
        greffe_add_instr((ea_t)argv[0].num, argv[1].c_str());
        res->set_long(1);
    } catch (const std::exception &e) {
        greffe_msg("GreffAddInstrEx error: %s\n", e.what());
        res->set_long(0);
    }
    return eOk;
}

static error_t idaapi idc_del_instr(idc_value_t *argv, idc_value_t *res) {
    try {
        greffe_delete_instr((ea_t)argv[0].num);
        GreffePanel::instance().refresh();
        res->set_long(1);
    } catch (const std::exception &e) {
        greffe_msg("GreffDelInstr error: %s\n", e.what());
        res->set_long(0);
    }
    return eOk;
}

static error_t idaapi idc_clear(idc_value_t *, idc_value_t *res) {
    try {
        greffe_clear_all();
        GreffePanel::instance().refresh();
        res->set_long(1);
    } catch (const std::exception &e) {
        greffe_msg("GreffClear error: %s\n", e.what());
        res->set_long(0);
    }
    return eOk;
}


static error_t idaapi idc_get_greffes(idc_value_t *, idc_value_t *res) {
    const auto &plans = g_ctx->layout.patch_plans();
    std::string json = "[";
    for (size_t i = 0; i < plans.size(); ++i) {
        const PatchPlan *p = plans[i];
        char buf[256];
        qsnprintf(buf, sizeof(buf),
            "{\"name\":\"%s\",\"handler\":\"%s\",\"ea\":\"0x%llx\"}",
            p->name.c_str(), p->handler_name.c_str(),
            (unsigned long long)p->target_ea);
        json += buf;
        if (i + 1 < plans.size())
            json += ",";
    }
    json += "]";
    res->set_string(json.c_str());
    return eOk;
}

static error_t idaapi idc_apply_patches(idc_value_t *, idc_value_t *res) {
    try {
        greffe_apply_patches();
        res->set_long(1);
    } catch (const std::exception &e) {
        greffe_msg("GreffApplyPatches error: %s\n", e.what());
        res->set_long(0);
    }
    return eOk;
}

static const char args_2long[]    = { VT_LONG, VT_LONG, 0 };
static const char args_1long[]    = { VT_LONG, 0 };
static const char args_long_str[] = { VT_LONG, VT_STR, 0 };
static const char args_none[]     = { 0 };

static const ext_idcfunc_t funcs[] = {
    { "GreffSetRegion",    idc_set_region,    args_2long,    nullptr, 0, 0 },
    { "GreffAddInstr",     idc_add_instr,     args_1long,    nullptr, 0, 0 },
    { "GreffDelInstr",     idc_del_instr,     args_1long,    nullptr, 0, 0 },
    { "GreffAddInstrEx",   idc_add_instr_ex,  args_long_str, nullptr, 0, 0 },
    { "GreffApplyPatches", idc_apply_patches, args_none,     nullptr, 0, 0 },
    { "GreffGetGreffes",   idc_get_greffes,   args_none,     nullptr, 0, 0 },
    { "GreffClear",        idc_clear,         args_none,     nullptr, 0, 0 },
};

void register_greffe_api() {
    for (const auto &f : funcs) {
        del_idc_func(f.name); // no-op if not registered; prevents duplicate on re-init
        add_idc_func(f);
    }
}

void unregister_greffe_api() {
}
