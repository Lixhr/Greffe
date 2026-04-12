#include "TargetManager.hpp"
#include "GreffeCTX.hpp"
#include "ProjectInfo.hpp"
#include "patch/arch/StubsFactory.hpp"
#include "patch/patch_utils.hpp"
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

// ---------------------------------------------------------------------------
// Handler stub creation
// ---------------------------------------------------------------------------

static void create_handler_stub(const Target& t, const ProjectInfo& pinfo) {
    namespace fs = std::filesystem;

    auto dir = pinfo.getProjectDir() / "handlers";
    fs::create_directories(dir);

    auto path = dir / (sanitize(t.name()) + ".greffe.c");
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
    f << "void handler_" << sanitize(t.name()) << "(void)\n{\n}\n";
}

// ---------------------------------------------------------------------------
// Target name resolution
// ---------------------------------------------------------------------------

static std::string create_target_name(ea_t ea) {
    func_t *func = get_func(ea);
    if (!func) {
        std::ostringstream ss;
        ss << "no function at 0x" << std::hex << ea;
        throw std::runtime_error(ss.str());
    }

    qstring name;
    if (get_func_name(&name, ea) <= 0 || name.empty()) {
        std::ostringstream ss;
        ss << "sub_" << std::hex << func->start_ea;
        return ss.str();
    }

    if (ea != func->start_ea) {
        std::ostringstream ss;
        ss << name.c_str() << "+" << std::hex << (ea - func->start_ea);
        return ss.str();
    }

    return name.c_str();
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<IArchStubs> resolve_stubs(ea_t ea, const ProjectInfo& pinfo) {
    return StubsFactory::create(pinfo.getBits(), pinfo.getModeAt(ea));
}

std::pair<PatchPlan*, bool> TargetManager::append_target(ea_t               ea,
                                                         ea_t               end_ea,
                                                         const std::string& name,
                                                         const ProjectInfo& pinfo) {
    auto it = std::lower_bound(_plans.begin(), _plans.end(), (uint64_t)ea,
        [](const PatchPlan& p, uint64_t val) { return p.target.ea() < val; });

    if (it != _plans.end() && it->target.ea() == (uint64_t)ea)
        return {&*it, false};

    auto stubs = resolve_stubs(ea, pinfo);
    it = _plans.emplace(it, PatchPlan{
        Target{name, (uint64_t)ea, (uint64_t)end_ea},
        std::move(stubs)
    });
    return {&*it, true};
}

std::pair<PatchPlan*, bool> TargetManager::add_internal(ea_t ea, GreffeCTX& ctx) {
    const ProjectInfo& pinfo = ctx.pinfo;

    std::string name   = create_target_name(ea);
    ea_t        end_ea = get_item_end(ea);

    auto [plan, inserted] = append_target(ea, end_ea, name, pinfo);
    if (inserted) {
        try {
            ctx.layout.create_patch_entry(plan);
            create_handler_stub(plan->target, pinfo);
        } catch (...) {
            auto it = std::lower_bound(_plans.begin(), _plans.end(), (uint64_t)ea,
                [](const PatchPlan& p, uint64_t val) { return p.target.ea() < val; });
            _plans.erase(it);
            throw;
        }
    }
    return {plan, inserted};
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void TargetManager::add(ea_t ea, GreffeCTX& ctx) {
    try {
        add_internal(ea, ctx);
    }
    catch (const std::exception &e) {
        greffe_msg("%s\n", e.what());
    }
}

void TargetManager::remove(size_t idx, GreffeCTX& ctx) {
    if (idx >= _plans.size())
        throw std::runtime_error("index out of range");

    auto it = _plans.begin() + static_cast<ptrdiff_t>(idx);
    const std::string name = it->target.name();
    _plans.erase(it);

    namespace fs = std::filesystem;
    auto handler = ctx.pinfo.getProjectDir() / "handlers" / (sanitize(name) + ".greffe.c");
    fs::remove(handler);
}

const std::vector<PatchPlan>& TargetManager::plans() const { return _plans; }
std::vector<PatchPlan>&       TargetManager::plans()       { return _plans; }
