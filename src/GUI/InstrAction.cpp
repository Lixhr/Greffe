#include <ida.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include "GUI/Actions.hpp"
#include "GreffeOps.hpp"
#include "GreffeCTX.hpp"
#include "PatchPlan.hpp"
#include "utils.hpp"
#include <set>
#include <string>

extern plugin_t PLUGIN;

static const char ACTION_NAME[] = "greffe:add_instr";

static std::string ask_handler_name(ea_t ea)
{
    if (!g_ctx)
        return {};

    qstrvec_t existing;
    std::set<std::string> seen;
    for (const PatchPlan *p : g_ctx->layout.patch_plans())
        if (seen.insert(p->handler_name).second)
            existing.push_back(p->handler_name.c_str());

    if (existing.empty())
        return {};

    int choice = ask_buttons("New handler", "Use existing", "Cancel",
                             1, "Handler for greffe at 0x%llx:", (ulonglong)ea);
    if (choice == -1)
        return "\x01";  // user cancelled

    if (choice == 1)
        return {};  // new handler

    std::string prompt = "Handler name\nAvailable:";
    for (const qstring &h : existing) {
        prompt += "\n  ";
        prompt += h.c_str();
    }

    qstring chosen = existing[0];
    if (!ask_str(&chosen, 0, "%s", prompt.c_str()))
        return "\x01";  // sentinel: user cancelled

    return std::string(chosen.c_str());
}

struct InstrActionHandler : public action_handler_t {
    int idaapi activate(action_activation_ctx_t *) override {
        ea_t ea = get_screen_ea();

        std::string handler = ask_handler_name(ea);
        if (!handler.empty() && handler[0] == '\x01')
            return 0;  // cancelled

        try {
            greffe_add_instr(ea, handler);
        } catch (const std::exception &e) {
            warning("%s", e.what());
            greffe_msg("error: %s\n", e.what());
            return 0;
        }
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

                if (!is_greffed(get_screen_ea()))
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
