#include "TargetManager.hpp"
#include "CLI/TargetCommands.hpp"
#include "ProjectInfo.hpp"
#include "utils.hpp"
#include <algorithm>
#include <fstream>
#include <stdexcept>

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
            json_get<std::string>(c, "raw"),
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

bool TargetManager::add_internal() {

}

const Target& TargetManager::add(const std::string& target_str, const ProjectInfo& pinfo) {
    const json entry = fetch_entry(target_str);
    std::vector<ContextEntry> context = parse_context(entry);
    const uint64_t ea = json_get<uint64_t>(entry, "ea");
    validate_context_modes(target_str, ea, context);


    std::lock_guard<std::mutex> lk(_mutex);

    auto it = std::lower_bound(_targets.begin(), _targets.end(), ea,
        [](const Target& t, uint64_t val) { return t.ea() < val; });

    if (it != _targets.end() && it->ea() == ea)
        return;

    it = _targets.emplace(it,
        json_get<std::string>(entry, "name"),
        ea,
        json_get<uint64_t>(entry, "end_ea"),
        std::move(context)
    );

    create_handler_stub(*it, pinfo);
    return *it;
}

bool TargetManager::add_direct(const json& entry, const ProjectInfo& pinfo) {
    uint64_t ea = json_get<uint64_t>(entry, "ea");

    std::lock_guard<std::mutex> lk(_mutex);

    auto it = std::lower_bound(_targets.begin(), _targets.end(), ea,
        [](const Target& t, uint64_t val) { return t.ea() < val; });

    if (it != _targets.end() && it->ea() == ea)
        return false;

    std::vector<ContextEntry> context = parse_context(entry);

    it = _targets.emplace(it,
        json_get<std::string>(entry, "name"),
        ea,
        json_get<uint64_t>(entry, "end_ea"),
        std::move(context)
    );

    create_handler_stub(*it, pinfo);
    return true;
}

void TargetManager::remove(const std::string& target) {
    std::lock_guard<std::mutex> lk(_mutex);
    decltype(_targets)::iterator it;

    if (target.size() > 2 && target[0] == '0' && (target[1] == 'x' || target[1] == 'X')) {
        uint64_t ea;
        try {
            ea = std::stoull(target, nullptr, 16);
        } catch (const std::out_of_range&) {
            throw std::runtime_error(target + ": address out of range");
        }
        it = std::find_if(_targets.begin(), _targets.end(),
            [ea](const Target& t) { return t.ea() == ea; });
    } else {
        it = std::find_if(_targets.begin(), _targets.end(),
            [&](const Target& t) { return t.name() == target; });
    }

    if (it == _targets.end())
        throw std::runtime_error(target + " not found");

    _targets.erase(it);
}

std::vector<Target> TargetManager::targets() const {
    std::lock_guard<std::mutex> lk(_mutex);
    return _targets;
}

void TargetManager::save(const std::filesystem::path& path,
                         uint64_t                     bin_base,
                         std::optional<uint64_t>      patch_base) const {
    std::lock_guard<std::mutex> lk(_mutex);
    json traced = json::array();
    for (const auto& t : _targets) {
        json ctx_arr = json::array();
        for (const auto& c : t.context())
            ctx_arr.push_back({ {"ea", c.ea}, {"raw", c.raw}, {"mode", c.mode},
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

SavedProject TargetManager::load(const std::filesystem::path& path) {
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

    for (const auto& entry : root.at("traced")) {
        std::vector<ContextEntry> context = parse_context(entry);

        uint64_t ea = json_get<uint64_t>(entry, "ea");
        auto it = std::lower_bound(_targets.begin(), _targets.end(), ea,
            [](const Target& t, uint64_t val) { return t.ea() < val; });

        if (it != _targets.end() && it->ea() == ea)
            continue;

        _targets.emplace(it,
            json_get<std::string>(entry, "name"),
            ea,
            json_get<uint64_t>(entry, "end_ea"),
            std::move(context)
        );
    }

    return cfg;
}
