#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ICommand.hpp"
#include "CLIDispatcher.hpp"

class CompositeCommand final : public ICommand {
public:
    CompositeCommand(std::string name, std::string description);

    std::string_view name()        const override;
    std::string_view description() const override;

    void add(std::unique_ptr<ICommand> sub);

    void execute(CLIContext& ctx, const Args& args) override;

    std::vector<std::string> complete(const Args& args) const override;

    CLIDispatcher&       dispatcher();
    const CLIDispatcher& dispatcher() const;

private:
    void print_usage() const;

    std::string   _name;
    std::string   _description;
    CLIDispatcher _dispatcher;
};