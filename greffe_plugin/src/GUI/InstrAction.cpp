#include <fstream>
#include <ida.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include "GUI/Actions.hpp"
#include "GreffeCTX.hpp"
#include "utils.hpp"
#include "StubsFactory.hpp"

extern plugin_t PLUGIN;

static const char ACTION_NAME[] = "greffe:add_instr";

static std::string create_target_name(ea_t ea) {
    std::ostringstream ss;
    ss << "_0x" << std::hex << ea << "_greffe";
    return ss.str();
}

static void create_handler_stub(const PatchPlan* plan, const ProjectInfo& pinfo) {
    namespace fs = std::filesystem;

    auto dir = pinfo.getProjectDir() / "handlers";
    fs::create_directories(dir);

    std::filesystem::path path = dir / std::string(plan->name + ".c");
    if (fs::exists(path))
        return;

    static const std::pair<std::string_view, std::string_view> attr_table[] = {
        { "thumb", "__attribute__((target(\"thumb\")))" },
    };

    std::string_view attr;
    const std::string mode = pinfo.getModeAt(static_cast<ea_t>(plan->target_ea));
    for (const auto& [m, a] : attr_table)
        if (mode == m) { attr = a; break; }

    std::ofstream f(path);
    if (!f)
        throw std::runtime_error("cannot create " + path.string());

    if (!attr.empty())
        f << attr << '\n';
    f << "void handler_" << plan->name << "(void)\n{\n}\n";
}


struct InstrActionHandler : public action_handler_t {
    int idaapi activate(action_activation_ctx_t *) override {
        ea_t ea = get_screen_ea();

        try {
            if (!g_ctx || !g_ctx->pinfo.getRegionsSet().has_regions()) {
                greffe_msg("define a patch region first\n");
                return 0;
            }
            GreffeCTX &ctx = *g_ctx;

            auto stubs = StubsFactory::create(ctx.pinfo.getBits(), ctx.pinfo.getModeAt(ea));
            auto plan  = std::make_unique<PatchPlan>(create_target_name(ea), ea, get_item_end(ea), std::move(stubs));

            ctx.layout.create_patch_entry(plan.get());
            PatchPlan *inserted = static_cast<PatchPlan*>(ctx.layout.queue_entry(std::move(plan)));
            create_handler_stub(inserted, ctx.pinfo);

            commit_gui(ctx.layout);

            ctx.layout.commit();
            g_ctx->layout.flush();
            g_ctx->layout._regions.refresh_all_data_items()
            greffe_msg("add target at 0x%llx\n", (ulonglong)ea);
        }
        catch (const std::exception &e) {
            greffe_msg("error: %s\n", e.what());
            // g_ctx->layout.flush();
            // replaced by a rollback
            return (0);
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
