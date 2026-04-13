#include <ida.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include "GUI/Actions.hpp"
#include "GreffeCTX.hpp"
#include "utils.hpp"

extern plugin_t PLUGIN;

static const char ACTION_NAME[] = "greffe:add_instr";

struct InstrActionHandler : public action_handler_t {
    int idaapi activate(action_activation_ctx_t *) override {
        ea_t ea = get_screen_ea();

        if (!g_ctx || !g_ctx->pinfo.getRegionsSet().has_regions()) {
            greffe_msg("define a patch region first\n");
            return 0;
        }

        greffe_msg("add target at 0x%llx\n", (ulonglong)ea);
        g_ctx->targets.add(ea, *g_ctx);
        return 1;
    }

    action_state_t idaapi update(action_update_ctx_t *ctx) override {
        if (ctx->widget_type == BWN_DISASM)
            return AST_ENABLE_FOR_WIDGET;
        return AST_DISABLE_FOR_WIDGET;
    }
};

struct InstrUIListener : public event_listener_t {
    ssize_t idaapi on_event(ssize_t code, va_list va) override {
        if (code == ui_populating_widget_popup) {
            TWidget *w = va_arg(va, TWidget *);
            if (get_widget_type(w) == BWN_DISASM) {
                TPopupMenu *popup = va_arg(va, TPopupMenu *);
                attach_action_to_popup(w, popup, ACTION_NAME);
            }
        }
        return 0;
    }
};

static InstrActionHandler s_handler;
static InstrUIListener    s_ui_listener;

static const action_desc_t s_action = ACTION_DESC_LITERAL(
    ACTION_NAME,
    "Add a greffe",
    &s_handler,
    "Shift+G",
    "Insert a greffe here",
    -1
);

void register_instr_action() {
    register_action(s_action);
    hook_event_listener(HT_UI, &s_ui_listener, &PLUGIN);
}

void unregister_instr_action() {
    unhook_event_listener(HT_UI, &s_ui_listener);
    unregister_action(ACTION_NAME);
}
