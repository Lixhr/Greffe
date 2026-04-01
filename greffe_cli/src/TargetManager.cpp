#include "TargetManager.hpp"
#include "CLI/TargetCommands.hpp"
#include "ProjectInfo.hpp"
#include "patch/arch/StubsFactory.hpp"
#include "utils.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "TrampolineBuilder.hpp"

static std::vector<uint8_t> hex_decode(const std::string& hex) {
    std::vector<uint8_t> out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2)
        out.push_back(static_cast<uint8_t>(std::stoul(hex.substr(i, 2), nullptr, 16)));
    return out;
}

static std::string hex_encode(const std::vector<uint8_t>& bytes) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t b : bytes)
        ss << std::setw(2) << static_cast<int>(b);
    return ss.str();
}

TargetManager::TargetManager(IdaIPC& ipc)
    : _ipc(ipc) {}

json TargetManager::fetch_entry(const std::string& target) {
    json req = {
        {"action", "add"},
        {"body",   json::array({target})},
    };
    json resp = _ipc.send(req);

    if (!resp.value("ok", false))
        throw std::runtime_error(resp.value("body", std::string("unknown IDA error")));

    const auto& body = resp.at("body");
    if (!body.is_array() || body.empty())
        throw std::runtime_error("IDA returned an empty response for " + target);

    return body.at(0);
}

std::vector<ContextEntry> TargetManager::parse_context(const json& entry) {
    std::vector<ContextEntry> context;
    for (const auto& c : entry.at("context")) {
        context.push_back({
            json_get<uint64_t>(c, "ea"),
            hex_decode(json_get<std::string>(c, "raw")),
            json_get<std::string>(c, "mode"),
            c.value("is_xref_target", false),
        });
    }
    return context;
}

void TargetManager::validate_context_modes(const std::string& target, uint64_t ea,
                                           const std::vector<ContextEntry>& context) {
    std::string target_mode;
    for (const auto& c : context)
        if (c.ea == ea) { target_mode = c.mode; break; }

    if (target_mode.empty())
        return;

    for (const auto& c : context) {
        if (c.mode != target_mode) {
            char buf[17];
            snprintf(buf, sizeof(buf), "%lx", c.ea);
            throw std::runtime_error(
                target + ": cpu mode mismatch at 0x" + buf +
                " (" + c.mode + " vs " + target_mode + ")");
        }
    }
}

static std::shared_ptr<IArchStubs> resolve_stubs(const std::vector<ContextEntry>& context,
                                                  uint64_t ea, int bits) {
    std::string_view mode;
    for (const auto& c : context)
        if (c.ea == ea) { mode = c.mode; break; }
    return StubsFactory::create(bits, mode);
}

std::pair<PatchPlan*, bool> TargetManager::append_target(const json& entry,
                                                          std::vector<ContextEntry> context,
                                                          const ProjectInfo& pinfo) {
    uint64_t          ea   = json_get<uint64_t>(entry, "ea");
    const std::string name = json_get<std::string>(entry, "name");

    validate_context_modes(name, ea, context);

    auto it = std::lower_bound(_plans.begin(), _plans.end(), ea,
        [](const PatchPlan& p, uint64_t val) { return p.target.ea() < val; });

    if (it != _plans.end() && it->target.ea() == ea)
        return {&*it, false};

    auto stubs = resolve_stubs(context, ea, pinfo.getBits());
    it = _plans.emplace(it, PatchPlan{
        Target{name, ea, json_get<uint64_t>(entry, "end_ea"), std::move(context)}
      , std::move(stubs)
      , _current_id
    });
    return {&*it, true};
}

std::pair<PatchPlan*, bool> TargetManager::add_internal(const json& entry,
                                                        CLIContext& ctx) {
    std::lock_guard<std::mutex> lk(_mutex);

    const ProjectInfo& pinfo = ctx.pinfo;
    std::vector<ContextEntry> context = parse_context(entry);

    auto [plan, inserted] = append_target(entry, std::move(context), pinfo);
    if (inserted) {

        try {
            ctx.layout.create_patch_entry(&(*plan));
            create_handler_stub(plan->target, pinfo);

            _current_id ++;
        } 
        
        catch (...) {
            auto it = std::lower_bound(_plans.begin(), _plans.end(), plan->target.ea(),
                [](const PatchPlan& p, uint64_t val) { return p.target.ea() < val; });
            _plans.erase(it);

            throw;
        }

    }
    return {plan, inserted};
}

const PatchPlan& TargetManager::add(const std::string& target_str, CLIContext& ctx) {
    const json entry = fetch_entry(target_str);
    return *add_internal(entry, ctx).first;
}

bool TargetManager::add_direct(const json& entry, CLIContext& ctx) {
    return add_internal(entry, ctx).second;
}

void TargetManager::remove(const std::string& target) {
    std::lock_guard<std::mutex> lk(_mutex);
    decltype(_plans)::iterator it;

    if (target.size() > 2 && target[0] == '0' && (target[1] == 'x' || target[1] == 'X')) {
        uint64_t ea;
        try {
            ea = std::stoull(target, nullptr, 16);
        } catch (const std::out_of_range&) {
            throw std::runtime_error(target + ": address out of range");
        }
        it = std::find_if(_plans.begin(), _plans.end(),
            [ea](const PatchPlan& p) { return p.target.ea() == ea; });
    } else {
        it = std::find_if(_plans.begin(), _plans.end(),
            [&](const PatchPlan& p) { return p.target.name() == target; });
    }

    if (it == _plans.end())
        throw std::runtime_error(target + " not found");

    _plans.erase(it);
}

const std::vector<PatchPlan>& TargetManager::plans() const {
    std::lock_guard<std::mutex> lk(_mutex);
    return _plans;
}

void TargetManager::save(const std::filesystem::path& path,
                         uint64_t                     bin_base,
                         std::optional<uint64_t>      patch_base) const {
    std::lock_guard<std::mutex> lk(_mutex);
    json traced = json::array();
    for (const auto& p : _plans) {
        const Target& t = p.target;
        json ctx_arr = json::array();
        for (const auto& c : t.context())
            ctx_arr.push_back({ {"ea", c.ea}, {"raw", hex_encode(c.raw)}, {"mode", c.mode},
                                 {"is_xref_target", c.is_xref_target} });

        traced.push_back({
            {"name",    t.name()},
            {"ea",      t.ea()},
            {"end_ea",  t.end_ea()},
            {"context", std::move(ctx_arr)},
        });
    }

    json project;
    project["bin_base"]   = bin_base;
    project["patch_base"] = patch_base ? json(*patch_base) : json(nullptr);

    json root = {
        {"project", std::move(project)},
        {"traced",  std::move(traced)},
    };

    std::ofstream f(path);
    if (!f)
        throw std::runtime_error("cannot open " + path.string());
    f << root.dump(2) << '\n';
}

SavedProject TargetManager::load(const std::filesystem::path& path, const ProjectInfo& pinfo) {
    std::lock_guard<std::mutex> lk(_mutex);
    std::ifstream f(path);
    if (!f)
        throw std::runtime_error("cannot open " + path.string());

    json root = json::parse(f);

    SavedProject cfg;
    if (root.contains("project")) {
        const auto& proj = root["project"];
        if (proj.contains("bin_base") && proj["bin_base"].is_number())
            cfg.bin_base = proj["bin_base"].get<uint64_t>();
        if (proj.contains("patch_base") && proj["patch_base"].is_number())
            cfg.patch_base = proj["patch_base"].get<uint64_t>();
    }

    for (const auto& entry : root.at("traced"))
        append_target(entry, parse_context(entry), pinfo);

    return cfg;
}
