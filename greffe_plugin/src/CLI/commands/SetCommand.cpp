#include "TargetCommands.hpp"
#include "CLIContext.hpp"
#include "colors.hpp"
#include <iostream>
#include <stdexcept>

std::string_view SetCommand::name()        const { return "set"; }
std::string_view SetCommand::description() const { return "Set bin_base or patch_base"; }


uint64_t    SetCommand::get_value_from_strhex(const Args& args) {
    uint64_t value;

    try {
        std::size_t pos = 0;
        value = std::stoull(args[1], &pos, 0);

        if (pos != args[1].size())
            throw std::invalid_argument("");

        return value;
    } catch (const std::exception&) {
        throw std::runtime_error("invalid address: " + args[1]);
    }
}

void SetCommand::execute(CLIContext& ctx, const Args& args) {
    if (args.size() != 2)
        throw std::runtime_error("usage: set [bin_base|patch_base] <addr>");

    const std::string& field = args[0];
    uint64_t value = get_value_from_strhex(args);

    if (field == "bin_base") {
        ctx.bin_base = value;
        std::cout << Color::GREY << "bin_base = 0x" << std::hex << value
                  << std::dec << Color::RST << '\n';
    } else if (field == "patch_base") {
        ctx.pinfo.setPatchBase(value);
        std::cout << Color::GREY << "patch_base = 0x" << std::hex << value
                  << std::dec << Color::RST << '\n';
    } else {
        throw std::runtime_error("unknown field '" + field + "'");
    }
}

std::vector<std::string> SetCommand::complete(const CLIContext* /*ctx*/, const Args& args) const {
    static const std::vector<std::string> fields = {"bin_base", "patch_base"};

    if (args.empty())
        return fields;

    if (args.size() == 1) {
        std::vector<std::string> result;
        for (const auto& f : fields)
            if (f.rfind(args[0], 0) == 0)
                result.push_back(f);
        return result;
    }
    return {};
}
