#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include "ProjectInfo.hpp"
#include "utils.hpp"
#include "GreffeCTX.hpp"
#include "GUI/Actions.hpp"
#include "GUI/GreffePanel.hpp"
#include "db.hpp"

extern "C" {
#include <gum/gum.h>
}

extern plugin_t PLUGIN;

struct IDBHooks : public event_listener_t {
    ssize_t idaapi on_event(ssize_t code, va_list) override {
        if (code == idb_event::savebase && g_ctx)
            save_db(g_ctx);
        return 0;
    }
};
static IDBHooks s_idb_hooks;

static plugmod_t *idaapi init() {
    gum_init_embedded();
    register_instr_action();
    register_region_action();
    register_patch_action();
    register_delete_action();
    register_greffe_api();
    register_panel_action();
    init_color_hook();
    hook_event_listener(HT_IDB, &s_idb_hooks, &PLUGIN);
    greffe_msg("plugin loaded\n");
    return PLUGIN_OK;
}

static void idaapi term() {
    unhook_event_listener(HT_IDB, &s_idb_hooks);
    term_color_hook();
    unregister_panel_action();
    unregister_greffe_api();
    unregister_delete_action();
    unregister_patch_action();
    unregister_instr_action();
    unregister_region_action();
    greffe_msg("plugin unloaded\n");
}

static bool idaapi run(size_t) {
    if (!g_ctx)
        g_ctx = std::make_unique<GreffeCTX>();
    load_db(g_ctx);
    refresh_colors();
    GreffePanel::instance().refresh();
    greffe_msg("running\n");
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
    nullptr
};