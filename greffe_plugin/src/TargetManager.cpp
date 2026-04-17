#include "TargetManager.hpp"
#include "GreffeCTX.hpp"
#include "ProjectInfo.hpp"
#include "patch/arch/StubsFactory.hpp"
#include "utils.hpp"

#include <bytes.hpp>
#include <funcs.hpp>
#include <name.hpp>
#include <xref.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

static void create_handler_stub(const Target& t, const ProjectInfo& pinfo) {
    namespace fs = std::filesystem;

    auto dir = pinfo.getProjectDir() / "handlers";
    fs::create_directories(dir);

    auto path = dir / std::string(t.name() + ".c");
    if (fs::exists(path))
        return;

    static const std::pair<std::string_view, std::string_view> attr_table[] = {
        { "thumb", "__attribute__((target(\"thumb\")))" },
    };

    std::string_view attr;
    const std::string mode = pinfo.getModeAt(static_cast<ea_t>(t.ea()));
    for (const auto& [m, a] : attr_table)
        if (mode == m) { attr = a; break; }

    std::ofstream f(path);
    if (!f)
        throw std::runtime_error("cannot create " + path.string());

    if (!attr.empty())
        f << attr << '\n';
    f << "void handler_" << t.name() << "(void)\n{\n}\n";
}


static std::string create_target_name(ea_t ea) {
    std::ostringstream ss;
    ss << "_0x" << std::hex << ea << "_greffe";
    return ss.str();
}

static std::shared_ptr<IArchStubs> resolve_stubs(ea_t ea, const ProjectInfo& pinfo) {
    return StubsFactory::create(pinfo.getBits(), pinfo.getModeAt(ea));
}

std::pair<PatchPlan*, bool> TargetManager::append_target(ea_t                    ea,
                                                         ea_t                    end_ea,
                                                         const std::string&      name,
                                                         const ProjectInfo&      pinfo,
                                                         std::vector<PatchPlan>& plans) {
    auto it = std::lower_bound(plans.begin(), plans.end(), ea,
        [](const PatchPlan& p, ea_t val) { return p.target.ea() < val; });

    if (it != plans.end() && it->target.ea() == ea)
        return {&*it, false};

    auto stubs = resolve_stubs(ea, pinfo);
    it = plans.emplace(it, PatchPlan{
        Target{name, ea, end_ea},
        std::move(stubs)
    });
    return {&*it, true};
}

std::pair<PatchPlan*, bool> TargetManager::add_internal(ea_t ea, GreffeCTX& ctx) {
    const ProjectInfo& pinfo = ctx.pinfo;

    std::string name   = create_target_name(ea);
    ea_t        end_ea = get_item_end(ea);

    auto& plans = ctx.layout.patch_plans();
    auto [plan, inserted] = append_target(ea, end_ea, name, pinfo, plans);
    if (inserted) {
        try {
            ctx.layout.create_patch_entry(plan);
            create_handler_stub(plan->target, pinfo);
        } catch (...) {
            auto it = std::lower_bound(plans.begin(), plans.end(), ea,
                [](const PatchPlan& p, ea_t val) { return p.target.ea() < val; });
            plans.erase(it);
            throw;
        }
    }
    return {plan, inserted};
}


void TargetManager::add(ea_t ea, GreffeCTX& ctx) {
    try {
        add_internal(ea, ctx);
    }
    catch (const std::exception &e) {
        greffe_msg("%s\n", e.what());
    }
}

// void TargetManager::remove(size_t idx, GreffeCTX& ctx) {
//     auto& plans = ctx.layout.patch_plans();
//     if (idx >= plans.size())
//         throw std::runtime_error("index out of range");

//     auto it = plans.begin() + static_cast<ptrdiff_t>(idx);
//     const std::string name = it->target.name();
//     plans.erase(it);

//     namespace fs = std::filesystem;
//     auto handler = ctx.pinfo.getProjectDir() / "handlers" / std::string(name + ".c");
//     fs::remove(handler);
// }

