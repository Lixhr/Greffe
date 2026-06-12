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
        greffe_msg("GreffeSetRegion error: %s\n", e.what());
        res->set_long(0);
    }
    return eOk;
}

// VT_WILD args are not converted by IDA; accept any integral IDC value.
static ea_t idc_to_ea(const idc_value_t &v) {
    switch (v.vtype) {
        case VT_LONG:  return (ea_t)v.num;
        case VT_INT64: return (ea_t)v.i64;
        default: throw std::runtime_error("expected an integer address");
    }
}

static error_t idaapi idc_add(idc_value_t *argv, idc_value_t *res) {
    const size_t argc = (size_t)res->num; // VT_WILD: IDA passes argc in res->num
    try {
        for (size_t i = 0; i < argc; ++i)
            greffe_add(idc_to_ea(argv[i]));
        res->set_long(1);
    } catch (const std::exception &e) {
        greffe_msg("GreffeAdd error: %s\n", e.what());
        res->set_long(0);
    }
    return eOk;
}

static error_t idaapi idc_add_ex(idc_value_t *argv, idc_value_t *res) {
    const size_t argc = (size_t)res->num;
    try {
        const char *handler = argv[0].c_str();
        for (size_t i = 1; i < argc; ++i)
            greffe_add(idc_to_ea(argv[i]), handler);
        res->set_long(1);
    } catch (const std::exception &e) {
        greffe_msg("GreffeAddEx error: %s\n", e.what());
        res->set_long(0);
    }
    return eOk;
}

static error_t idaapi idc_del(idc_value_t *argv, idc_value_t *res) {
    const size_t argc = (size_t)res->num;
    try {
        for (size_t i = 0; i < argc; ++i)
            greffe_delete(idc_to_ea(argv[i]));

        GreffePanel::instance().refresh();
        res->set_long(1);
    } catch (const std::exception &e) {
        greffe_msg("GreffeDel error: %s\n", e.what());
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
        greffe_msg("GreffeClear error: %s\n", e.what());
        res->set_long(0);
    }
    return eOk;
}

static error_t idaapi idc_get_array(idc_value_t *, idc_value_t *res) {
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
        greffe_msg("GreffeApplyPatches error: %s\n", e.what());
        res->set_long(0);
    }
    return eOk;
}

static const char args_2long[]    = { VT_LONG, VT_LONG, 0 };
static const char args_wild[]     = { VT_WILD, 0 };
static const char args_str_wild[] = { VT_STR, VT_WILD, 0 };
static const char args_none[]     = { 0 };

static const ext_idcfunc_t funcs[] = {
    { "GreffeSetRegion",    idc_set_region,    args_2long,    nullptr, 0, 0 },
    { "GreffeAdd",          idc_add,     args_wild,     nullptr, 0, 0 },
    { "GreffeDel",          idc_del,     args_wild,     nullptr, 0, 0 },
    { "GreffeAddEx",        idc_add_ex,  args_str_wild, nullptr, 0, 0 },
    { "GreffeApplyPatches", idc_apply_patches, args_none,     nullptr, 0, 0 },
    { "GreffeGetArray",     idc_get_array,   args_none,     nullptr, 0, 0 },
    { "GreffeClear",        idc_clear,         args_none,     nullptr, 0, 0 },
};

void register_greffe_api() {
    for (const auto &f : funcs) {
        del_idc_func(f.name); // no-op if not registered; prevents duplicate on re-init
        add_idc_func(f);
    }
}

void unregister_greffe_api() {
}
