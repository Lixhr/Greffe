#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include "ProjectInfo.hpp"
#include "utils.hpp"
#include "GreffeCTX.hpp"
#include "GUI/Actions.hpp"

extern "C" {
#include <gum/gum.h>
}

static plugmod_t *idaapi init() {
    gum_init_embedded();
    register_instr_action();
    register_region_action();
    register_patch_action();
    greffe_msg("plugin loaded\n");
    return PLUGIN_OK;
}

static void idaapi term() {
    unregister_patch_action();
    unregister_instr_action();
    unregister_region_action();
    greffe_msg("plugin unloaded\n");
}

static bool idaapi run(size_t) {
    if (!g_ctx)
        g_ctx = std::make_unique<GreffeCTX>();
    set_range_color(0x60, 0x70, 0xffff8ca1);
    return true;
}

plugin_t PLUGIN = {
    IDP_INTERFACE_VERSION,
    PLUGIN_FIX,
    init,
    term,
    run,
    "Greffe instrumentation plugin",
    nullptr,
    "greffe",
    "Shift+R"
};