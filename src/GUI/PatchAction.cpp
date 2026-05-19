#include <ida.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include "GUI/Actions.hpp"
#include "GreffeOps.hpp"
#include "GreffeCTX.hpp"
#include "utils.hpp"
#include "db.hpp"

extern plugin_t PLUGIN;

static const char PATCH_ACTION_NAME[] = "greffe:patch";

struct PatchActionHandler : public action_handler_t {
    int idaapi activate(action_activation_ctx_t *) override {
        try {
            greffe_apply_patches();
            save_db(g_ctx);
        } catch (const std::exception &e) {
            warning("%s", e.what());
            greffe_msg("error: %s\n", e.what());
            return 0;
        }
        return 1;
    }

    action_state_t idaapi update(action_update_ctx_t *) override {
        return AST_ENABLE_ALWAYS;
    }
};

static PatchActionHandler s_handler;

static const action_desc_t s_action = ACTION_DESC_LITERAL(
    PATCH_ACTION_NAME,
    "Apply greffe patches",
    &s_handler,
    "Shift+P",
    "Build handlers and apply all greffes",
    -1
);

void register_patch_action() {
    register_action(s_action);
}

void unregister_patch_action() {
    unregister_action(PATCH_ACTION_NAME);
}
