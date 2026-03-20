#include "TargetManager.hpp"
#include "utils.hpp"
#include <algorithm>
#include <fstream>
#include <stdexcept>

TargetManager::TargetManager(IdaIPC& ipc)
    : _ipc(ipc) {}

const Target& TargetManager::add(const std::string& target) {
    json req = {
        {"action", "add"},
        {"body",   json::array({target})},
    };

    json resp = _ipc.send(req);

    if (!resp.value("ok", false))
        throw std::runtime_error(resp.at("body").get<std::string>());

    const json& entry = resp.at("body").at(0);

    std::vector<ContextEntry> context;
    for (const auto& c : entry.at("context")) {
        context.push_back({
            json_get<uint64_t>(c, "ea"),
            json_get<std::string>(c, "raw"),
            json_get<std::string>(c, "mode"),
        });
    }

    uint64_t ea = json_get<uint64_t>(entry, "ea");

    std::string target_mode;
    for (const auto& c : context)
        if (c.ea == ea) { target_mode = c.mode; break; }

    if (!target_mode.empty()) {
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

    auto it = std::lower_bound(_targets.begin(), _targets.end(), ea,
        [](const Target& t, uint64_t val) { return t.ea() < val; });

    if (it != _targets.end() && it->ea() == ea)
        throw std::runtime_error(target + " already registered");

    it = _targets.emplace(it,
        json_get<std::string>(entry, "name"),
        ea,
        json_get<uint64_t>(entry, "end_ea"),
        std::move(context)
    );

    return *it;
}

void TargetManager::remove(const std::string& target) {
    decltype(_targets)::iterator it;

    if (target.size() > 2 && target[0] == '0' && (target[1] == 'x' || target[1] == 'X')) {
        uint64_t ea = std::stoull(target, nullptr, 16);
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

const std::vector<Target>& TargetManager::targets() const {
    return _targets;
}

void TargetManager::save(const std::filesystem::path& path) const {
    json traced = json::array();
    for (const auto& t : _targets) {
        json ctx_arr = json::array();
        for (const auto& c : t.context())
            ctx_arr.push_back({ {"ea", c.ea}, {"raw", c.raw}, {"mode", c.mode} });

        traced.push_back({
            {"name",    t.name()},
            {"ea",      t.ea()},
            {"end_ea",  t.end_ea()},
            {"context", std::move(ctx_arr)},
        });
    }

    json root = {
        {"project", { {"img_base", 0} }},
        {"traced",  std::move(traced)},
    };

    std::ofstream f(path);
    if (!f)
        throw std::runtime_error("cannot open " + path.string());
    f << root.dump(2) << '\n';
}

void TargetManager::load(const std::filesystem::path& path) {
    std::ifstream f(path);
    if (!f)
        throw std::runtime_error("cannot open " + path.string());

    json root = json::parse(f);
    for (const auto& entry : root.at("traced")) {
        std::vector<ContextEntry> context;
        for (const auto& c : entry.at("context"))
            context.push_back({
                json_get<uint64_t>(c, "ea"),
                json_get<std::string>(c, "raw"),
                json_get<std::string>(c, "mode"),
            });

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
}
