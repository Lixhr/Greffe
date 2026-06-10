#include "GreffeOps.hpp"
#include "GreffeCTX.hpp"
#include "utils.hpp"
#include "StubsFactory.hpp"
#include "PatchPlan.hpp"
#include "patch/PatchLayoutEntry.hpp"
#include "db.hpp"
#include <ida.hpp>
#include <ua.hpp>
#include <offset.hpp>
#include <algorithm>
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

    fs::path path = dir / (plan->handler_name + ".c");
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
    f << "void handler_" << plan->handler_name << "(unsigned int greffe_id)\n{\n}\n";
}

void greffe_set_region(ea_t start, ea_t end) {
    if (!g_ctx)
        g_ctx = std::make_unique<GreffeCTX>();
    g_ctx->pinfo.getRegionsSet().add_region(start, end);
    greffe_msg("patch region added: 0x%llx - 0x%llx\n", (ulonglong)start, (ulonglong)end);
}

void greffe_add_instr(ea_t ea, const std::string &handler_name) {
    if (!g_ctx || !g_ctx->pinfo.getRegionsSet().has_regions())
        throw std::runtime_error("define a patch region first");

    GreffeCTX &ctx = *g_ctx;
    ctx.layout.free_handler_bin();

    try {
        auto stubs = StubsFactory::create(ctx.pinfo.getArchKeyAt(ea));
        std::string target_name = make_target_name(ea);
        std::string hname = handler_name.empty() ? target_name : handler_name;
        auto plan  = std::make_unique<PatchPlan>(std::move(target_name),
                                                 std::move(hname),
                                                 ea,
                                                 get_item_end(ea),
                                                 std::move(stubs));

        ctx.layout.create_patch_entry(plan.get());
        ctx.layout.queue_entry(std::move(plan));

        commit_gui(ctx.layout);
        ctx.layout.commit();
        greffe_msg("add target at 0x%llx\n", (ulonglong)ea);
    } catch (...) {
        ctx.layout.rollback();
        throw;
    }
}

void greffe_delete_instr(ea_t ea) {
    if (!g_ctx) return;

    auto *e = g_ctx->layout.entry_at(ea);
    if (!e) return;

    ea_t target_ea;
    if (e->type() == PLEType::entry_branch)
        target_ea = e->ea();
    else if (e->type() == PLEType::entry_plan)
        target_ea = static_cast<PatchPlan *>(e)->target_ea;
    else
        return;

    g_ctx->layout.entries_delete_if([target_ea](PatchLayoutEntry &entry) {
        if (entry.type() == PLEType::entry_branch)
            return entry.ea() == target_ea;
        if (entry.type() == PLEType::entry_plan)
            return static_cast<PatchPlan &>(entry).target_ea == target_ea;
        return false;
    });

    if (g_ctx->layout.patch_plans().empty())
        g_ctx->layout.entries_delete_if([](PatchLayoutEntry &entry) {
            return entry.type() == PLEType::entry_shstub;
        });

    save_db(g_ctx);
}

void greffe_create_named_stub(const std::string &name) {
    namespace fs = std::filesystem;
    if (!g_ctx) throw std::runtime_error("no context");

    auto dir = g_ctx->pinfo.getProjectDir() / "handlers";
    fs::create_directories(dir);
    fs::path path = dir / (name + ".c");
    if (fs::exists(path)) return;

    static const std::pair<std::string_view, std::string_view> attr_table[] = {
        { "thumb", "__attribute__((target(\"thumb\")))" },
    };
    std::string_view attr;
    auto plans = g_ctx->layout.patch_plans();
    if (!plans.empty()) {
        const ArchKey key = g_ctx->pinfo.getArchKeyAt(static_cast<ea_t>(plans[0]->target_ea));
        for (const auto &[m, a] : attr_table)
            if (key.mode == m) { attr = a; break; }
    }

    std::ofstream f(path);
    if (!f) throw std::runtime_error("cannot create " + path.string());
    if (!attr.empty()) f << attr << '\n';
    f << "void handler_" << name << "(unsigned int greffe_id)\n{\n}\n";
    greffe_msg("created handler stub: %s.c\n", name.c_str());
}

void greffe_delete_handler(const std::string &name) {
    namespace fs = std::filesystem;
    if (!g_ctx) throw std::runtime_error("no context");
    for (PatchPlan *plan : g_ctx->layout.patch_plans())
        if (plan->handler_name == name)
            plan->handler_name = {};
    fs::remove(g_ctx->pinfo.getProjectDir() / "handlers" / (name + ".c"));
    save_db(g_ctx);
    greffe_msg("deleted handler: %s\n", name.c_str());
}

std::vector<std::string> greffe_list_handlers() {
    namespace fs = std::filesystem;
    if (!g_ctx) return {};
    std::vector<std::string> result;
    auto dir = g_ctx->pinfo.getProjectDir() / "handlers";
    if (!fs::is_directory(dir)) return result;
    for (const auto &e : fs::directory_iterator(dir))
        if (e.path().extension() == ".c")
            result.push_back(e.path().stem().string());
    std::sort(result.begin(), result.end());
    return result;
}

void greffe_create_stubs() {
    if (!g_ctx || g_ctx->layout.patch_plans().empty())
        throw std::runtime_error("no targets to create stubs for");

    GreffeCTX &ctx = *g_ctx;
    size_t created = 0;
    for (const PatchPlan *plan : ctx.layout.patch_plans()) {
        if (plan->handler_name.empty())
            throw std::runtime_error("assign a handler name to every patch before creating stubs");
        namespace fs = std::filesystem;
        fs::path path = ctx.pinfo.getProjectDir() / "handlers" / (plan->handler_name + ".c");
        bool existed = fs::exists(path);
        create_handler_stub(plan, ctx.pinfo);
        if (!existed)
            ++created;
    }
    greffe_msg("created %zu handler stub(s)\n", created);
}

void greffe_apply_patches() {
    if (!g_ctx || g_ctx->layout.patch_plans().empty())
        throw std::runtime_error("no targets to patch");

    GreffeCTX &ctx = *g_ctx;

    for (const PatchPlan *plan : ctx.layout.patch_plans()) {
        if (plan->handler_name.empty())
            throw std::runtime_error("assign a handler name to every patch before applying");
        create_handler_stub(plan, ctx.pinfo);
    }

    ctx.layout.free_handler_bin();

    try {
        HandlerBin *bin = ctx.layout.place_handler_bin();

        for (auto &plan : ctx.layout.patch_plans()) {
            std::string sym = "handler_" + plan->handler_name;
            plan->handler_addr = bin->handler_addr(sym);

            uint8_t *handler_slot = plan->bytes().data() + (plan->handler_ptr_addr - plan->ea());
            plan->stubs->write_ptr(handler_slot, plan->handler_addr);
            write_data_patch(plan->handler_ptr_addr, handler_slot, plan->stubs->sizeof_ptr());
            op_plain_offset(plan->handler_ptr_addr, 0, 0);

            if (plan->greffe_id_addr) {
                uint8_t *id_slot = plan->bytes().data() + (plan->greffe_id_addr - plan->ea());
                plan->stubs->write_ptr(id_slot, plan->target_ea);
                write_data_patch(plan->greffe_id_addr, id_slot, plan->stubs->sizeof_ptr());
                op_plain_offset(plan->greffe_id_addr, 0, 0);
            }
        }

        commit_gui(ctx.layout);
        ctx.layout.commit();
        greffe_msg("patched %zu targets\n", ctx.layout.patch_plans().size());
    } catch (...) {
        ctx.layout.rollback();
        throw;
    }
}
