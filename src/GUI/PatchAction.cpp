#include <ida.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include "GUI/Actions.hpp"
#include "GreffeCTX.hpp"
#include "patch/HandlerCompiler.hpp"
#include "utils.hpp"
#include "name.hpp"
#include "offset.hpp"

extern plugin_t PLUGIN;

static const char PATCH_ACTION_NAME[] = "greffe:patch";

struct PatchActionHandler : public action_handler_t {
    int idaapi activate(action_activation_ctx_t *) override {
        if (!g_ctx || g_ctx->layout.patch_plans().empty()) {
            greffe_msg("no targets to patch\n");
            return 0;
        }

        try {
            g_ctx->layout.free_handler_bin();

            HandlerBin *bin = g_ctx->layout.place_handler_bin();

            for (auto& plan : g_ctx->layout.patch_plans()) {
                std::string sym = "handler_" + plan->name;

                plan->handler_addr = bin->handler_addr(sym);

                uint8_t *handler_slot = plan->bytes().data() + (plan->handler_ptr_addr - plan->ea());
                plan->stubs->write_ptr(handler_slot, plan->handler_addr);

                write_data_patch(plan->handler_ptr_addr,
                                 handler_slot,
                                 plan->stubs->sizeof_ptr());
                op_plain_offset(plan->handler_ptr_addr, 0, 0);
            }

            commit_gui(g_ctx->layout);
            g_ctx->layout.commit();

            greffe_msg("patched %zu target(s)\n", g_ctx->layout.patch_plans().size());
            return 0;
        } catch (const std::exception& e) {
            greffe_msg("patch failed: %s\n", e.what());
            g_ctx->layout.rollback();
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