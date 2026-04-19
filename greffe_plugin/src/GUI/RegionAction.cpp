#include <ida.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include "GUI/Actions.hpp"
#include "utils.hpp"
#include "GreffeCTX.hpp"

extern plugin_t PLUGIN;

static const char REGION_ACTION_NAME[] = "greffe:set_patch_region";

struct RegionActionHandler : public action_handler_t {
    int idaapi activate(action_activation_ctx_t *ctx) override {
        twinpos_t p1, p2;

        if (!read_selection(ctx->widget, &p1, &p2)) {
            greffe_msg("set patch region: select a range first\n");
            return 0;
        }

        ea_t start_ea = p1.at->toea();
        ea_t end_ea   = p2.at->toea();

        if (!g_ctx)
            g_ctx = std::make_unique<GreffeCTX>();

        try {
            g_ctx->pinfo.getRegionsSet().add_region(start_ea, end_ea);
            greffe_msg("patch region added: 0x%llx - 0x%llx\n", start_ea, end_ea);

        } catch (const std::exception& e) {
            greffe_msg("%s\n", e.what());
        }
        return 1;
    }

    action_state_t idaapi update(action_update_ctx_t *ctx) override {
        if (ctx->widget_type == BWN_DISASM)
            return AST_ENABLE_FOR_WIDGET;
        return AST_DISABLE_FOR_WIDGET;
    }
};

static RegionActionHandler s_handler;

struct RegionUIListener : public event_listener_t {
    ssize_t idaapi on_event(ssize_t code, va_list va) override {
        if (code == ui_populating_widget_popup) {
            TWidget *w = va_arg(va, TWidget *);
            if (get_widget_type(w) == BWN_DISASM) {
                TPopupMenu *popup = va_arg(va, TPopupMenu *);
                const action_activation_ctx_t *ctx = va_arg(va, const action_activation_ctx_t *);
                if (ctx->has_flag(ACF_HAS_SELECTION))
                    attach_action_to_popup(w, popup, REGION_ACTION_NAME);
            }
        }
        return 0;
    }
};

static RegionUIListener s_ui_listener;

static const action_desc_t s_action = ACTION_DESC_LITERAL(
    REGION_ACTION_NAME,
    "Set as greffe patch region",
    &s_handler,
    "Shift+R",
    "Set a greffe patch region on the selected address range",
    -1
);

void register_region_action() {
    register_action(s_action);
    hook_event_listener(HT_UI, &s_ui_listener, &PLUGIN);
}

void unregister_region_action() {
    unhook_event_listener(HT_UI, &s_ui_listener);
    unregister_action(REGION_ACTION_NAME);
}
