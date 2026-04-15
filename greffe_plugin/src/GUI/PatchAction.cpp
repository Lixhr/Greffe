#include <ida.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include "GUI/Actions.hpp"
#include "GreffeCTX.hpp"
#include "patch/HandlerCompiler.hpp"
#include "utils.hpp"
#include "name.hpp"

extern plugin_t PLUGIN;

static const char PATCH_ACTION_NAME[] = "greffe:patch";

struct PatchActionHandler : public action_handler_t {
    int idaapi activate(action_activation_ctx_t *) override {
        if (!g_ctx || g_ctx->targets.plans().empty()) {
            greffe_msg("no targets to patch\n");
            return 0;
        }

        try {
            g_ctx->layout.rebuild();

            HandlerBin bin = HandlerCompiler::build(g_ctx->layout.patch_plans(),
                                                    g_ctx->pinfo);
            g_ctx->layout.place_handler_bin(bin);

            for (auto& plan : g_ctx->layout.patch_plans()) {
                std::string sym = "handler_" + plan.target.name();

                ea_t handler_addr = bin.handler_addr(sym);
                uint8_t *handler_slot = plan.bytes().data() + (plan.handler_address - plan.addr());
                plan.stubs->write_ptr(handler_slot, handler_addr);
                bin.add_label(handler_addr & ~1, sym.c_str());


                write_data_patch(plan.handler_address,
                                 handler_slot,
                                 plan.stubs->sizeof_ptr(),
                                 Color::HANDLER_CODE);
            }

            write_code_patch(bin.addr(), bin.bytes().data(), bin.bytes().size(), Color::HANDLER_CODE);

            greffe_msg("patched %zu target(s)\n",
                       g_ctx->layout.patch_plans().size());
        } catch (const std::exception& e) {
            greffe_msg("patch failed: %s\n", e.what());
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