#include "GUI/GreffePanel.hpp"
#include "GUI/GreffeWindow.hpp"
#include "GUI/Actions.hpp"
#include "GreffeOps.hpp"
#include "GreffeCTX.hpp"
#include "PatchPlan.hpp"
#include "patch/PatchLayoutEntry.hpp"
#include "db.hpp"
#include <ida.hpp>
#include <kernwin.hpp>

extern plugin_t PLUGIN;

static GreffeWindow *s_window = nullptr;

static GreffeWindow &get_window() {
    if (s_window)
        return *s_window;

    s_window = new GreffeWindow();

    s_window->setHandlerChangedCb([](uint64_t ea, const std::string &name) {
        if (!g_ctx) return;
        for (PatchPlan *plan : g_ctx->layout.patch_plans()) {
            if ((uint64_t)plan->target_ea == ea) {
                plan->handler_name = name;
                break;
            }
        }
        save_db(g_ctx);
    });

    s_window->setHandlerAddedCb([](const std::string &name) {
        try {
            greffe_create_named_stub(name);
        } catch (const std::exception &e) {
            warning("%s", e.what());
        }
        GreffePanel::instance().refresh();
    });

    s_window->setHandlerDeletedCb([](const std::string &name) {
        try {
            greffe_delete_handler(name);
        } catch (const std::exception &e) {
            warning("%s", e.what());
        }
        GreffePanel::instance().refresh();
    });

    s_window->setGreffeDeletedCb([](uint64_t ea) {
        greffe_delete(static_cast<ea_t>(ea));
        GreffePanel::instance().refresh();
    });

    return *s_window;
}

GreffePanel &GreffePanel::instance() {
    static GreffePanel s_instance;
    return s_instance;
}

GreffePanel::GreffePanel() {}

void GreffePanel::show() {
    get_window().show();
}

void GreffePanel::refresh() {
    auto &win = get_window();
    win.setHandlers(greffe_list_handlers());

    std::vector<GreffeRow> rows;
    if (g_ctx) {
        for (PatchPlan *plan : g_ctx->layout.patch_plans())
            rows.push_back({(uint64_t)plan->target_ea, plan->handler_name});
    }
    win.setGreffes(rows);
}

static const char PANEL_ACTION_NAME[] = "greffe:show_panel";

struct PanelActionHandler : public action_handler_t {
    int idaapi activate(action_activation_ctx_t *) override {
        GreffePanel::instance().show();
        return 1;
    }
    action_state_t idaapi update(action_update_ctx_t *) override {
        return AST_ENABLE_ALWAYS;
    }
};

static PanelActionHandler s_panel_handler;

static const action_desc_t s_panel_action = ACTION_DESC_LITERAL(
    PANEL_ACTION_NAME,
    "Show greffe panel",
    &s_panel_handler,
    "Shift+M",
    "Open the patch manager panel",
    -1
);

void register_panel_action() {
    register_action(s_panel_action);
    attach_action_to_menu("Edit/Plugins/", PANEL_ACTION_NAME, SETMENU_APP);
}

void unregister_panel_action() {
    detach_action_from_menu("Edit/Plugins/", PANEL_ACTION_NAME);
    unregister_action(PANEL_ACTION_NAME);
}
