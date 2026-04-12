#include <ida.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include <bytes.hpp>
#include "GUI/Actions.hpp"
#include "GreffeCTX.hpp"
#include "patch/HandlerCompiler.hpp"
#include "patch/patch_utils.hpp"
#include "utils.hpp"

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
                std::string sym = "handler_" + sanitize(plan.target.name());
                plan.stubs->write_ptr(plan.bytes().data() + plan.handler_offset,
                                      bin.handler_addr(sym));
            }

            for (const auto& branch : g_ctx->layout.branches())
                patch_bytes(static_cast<ea_t>(branch.addr()),
                            branch.bytes().data(),
                            static_cast<ssize_t>(branch.bytes().size()));

            for (const auto& plan : g_ctx->layout.patch_plans())
                patch_bytes(static_cast<ea_t>(plan.addr()),
                            plan.bytes().data(),
                            static_cast<ssize_t>(plan.bytes().size()));

            for (const auto& shstub : g_ctx->layout.shstubs())
                patch_bytes(static_cast<ea_t>(shstub.addr()),
                            shstub.bytes().data(),
                            static_cast<ssize_t>(shstub.bytes().size()));

            patch_bytes(static_cast<ea_t>(bin.addr()),
                        bin.bytes().data(),
                        static_cast<ssize_t>(bin.bytes().size()));

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