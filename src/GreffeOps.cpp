#include "GreffeOps.hpp"
#include "GreffeCTX.hpp"
#include "utils.hpp"
#include "StubsFactory.hpp"
#include "PatchPlan.hpp"
#include <ida.hpp>
#include <ua.hpp>
#include <offset.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>

static std::string make_target_name(ea_t ea) {
    std::ostringstream ss;
    ss << "_0x" << std::hex << ea << "_greffe";
    return ss.str();
}

static void create_handler_stub(const PatchPlan *plan, const ProjectInfo &pinfo) {
    namespace fs = std::filesystem;

    auto dir = pinfo.getProjectDir() / "handlers";
    fs::create_directories(dir);

    fs::path path = dir / (plan->name + ".c");
    if (fs::exists(path))
        return;

    static const std::pair<std::string_view, std::string_view> attr_table[] = {
        { "thumb", "__attribute__((target(\"thumb\")))" },
    };

    std::string_view attr;
    const ArchKey key = pinfo.getArchKeyAt(static_cast<ea_t>(plan->target_ea));
    for (const auto &[m, a] : attr_table)
        if (key.mode == m) { attr = a; break; }

    std::ofstream f(path);
    if (!f)
        throw std::runtime_error("cannot create " + path.string());

    if (!attr.empty())
        f << attr << '\n';
    f << "void handler_" << plan->name << "(void)\n{\n}\n";
}

void greffe_set_region(ea_t start, ea_t end) {
    if (!g_ctx)
        g_ctx = std::make_unique<GreffeCTX>();
    g_ctx->pinfo.getRegionsSet().add_region(start, end);
    greffe_msg("patch region added: 0x%llx - 0x%llx\n", (ulonglong)start, (ulonglong)end);
}

void greffe_add_instr(ea_t ea) {
    if (!g_ctx || !g_ctx->pinfo.getRegionsSet().has_regions())
        throw std::runtime_error("define a patch region first");

    GreffeCTX &ctx = *g_ctx;
    ctx.layout.free_handler_bin();

    try {
        auto stubs = StubsFactory::create(ctx.pinfo.getArchKeyAt(ea));
        auto plan  = std::make_unique<PatchPlan>(make_target_name(ea),
                                                 ea,
                                                 get_item_end(ea),
                                                 std::move(stubs));

        ctx.layout.create_patch_entry(plan.get());
        auto inserted = static_cast<PatchPlan *>(ctx.layout.queue_entry(std::move(plan)));
        create_handler_stub(inserted, ctx.pinfo);

        commit_gui(ctx.layout);
        ctx.layout.commit();
        greffe_msg("add target at 0x%llx\n", (ulonglong)ea);
    } catch (...) {
        ctx.layout.rollback();
        throw;
    }
}

void greffe_apply_patches() {
    if (!g_ctx || g_ctx->layout.patch_plans().empty())
        throw std::runtime_error("no targets to patch");

    GreffeCTX &ctx = *g_ctx;
    ctx.layout.free_handler_bin();

    try {
        HandlerBin *bin = ctx.layout.place_handler_bin();

        for (auto &plan : ctx.layout.patch_plans()) {
            std::string sym = "handler_" + plan->name;
            plan->handler_addr = bin->handler_addr(sym);
            uint8_t *handler_slot = plan->bytes().data() + (plan->handler_ptr_addr - plan->ea());
            plan->stubs->write_ptr(handler_slot, plan->handler_addr);
            write_data_patch(plan->handler_ptr_addr, handler_slot, plan->stubs->sizeof_ptr());
            op_plain_offset(plan->handler_ptr_addr, 0, 0);
        }

        commit_gui(ctx.layout);
        ctx.layout.commit();
        greffe_msg("patched %zu targets\n", ctx.layout.patch_plans().size());
    } catch (...) {
        ctx.layout.rollback();
        throw;
    }
}
