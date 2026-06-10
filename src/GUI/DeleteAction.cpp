#include <ida.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include "GUI/Actions.hpp"
#include "GUI/GreffePanel.hpp"
#include "GreffeOps.hpp"
#include "GreffeCTX.hpp"
#include "patch/PatchLayoutEntry.hpp"
#include "utils.hpp"

extern plugin_t PLUGIN;

static const char ACTION_NAME[] = "greffe:delete_instr";

struct DeleteActionHandler : public action_handler_t {
    int idaapi activate(action_activation_ctx_t *) override {
        ea_t ea = get_screen_ea();
        try {
            greffe_delete_instr(ea);
            GreffePanel::instance().refresh();

        } catch (const std::exception &e) {
            greffe_msg("error: %s\n", e.what());
            return 0;
        }
        return 1;
    }

    action_state_t idaapi update(action_update_ctx_t *ctx) override {
        if (ctx->widget_type != BWN_DISASM)
            return AST_DISABLE_FOR_WIDGET;
        if (!g_ctx)
            return AST_DISABLE;
        ea_t ea = ctx->cur_ea;
        if (ea == BADADDR)
            return AST_DISABLE;
        return is_greffed(ea) ? AST_ENABLE : AST_DISABLE;
    }
};

struct DeleteUIListener : public event_listener_t {
    ssize_t idaapi on_event(ssize_t code, va_list va) override {
        if (code == ui_populating_widget_popup) {
            TWidget *w = va_arg(va, TWidget *);
            if (get_widget_type(w) == BWN_DISASM) {
                TPopupMenu *popup = va_arg(va, TPopupMenu *);
                if (is_greffed(get_screen_ea()))
                    attach_action_to_popup(w, popup, ACTION_NAME);
            }
        }
        return 0;
    }
};

static DeleteActionHandler s_handler;
static DeleteUIListener    s_ui_listener;

static const action_desc_t s_action = ACTION_DESC_LITERAL(
    ACTION_NAME,
    "Delete greffe",
    &s_handler,
    "Shift+D",
    "Remove this greffe",
    -1
);

void register_delete_action() {
    register_action(s_action);
    hook_event_listener(HT_UI, &s_ui_listener, &PLUGIN);
}

void unregister_delete_action() {
    unhook_event_listener(HT_UI, &s_ui_listener);
    unregister_action(ACTION_NAME);
}
