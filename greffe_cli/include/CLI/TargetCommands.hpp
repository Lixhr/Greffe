#pragma once

#include "ICommand.hpp"
#include "ProjectInfo.hpp"
#include "Target.hpp"

void create_handler_stub(const Target& t, const ProjectInfo& pinfo);

class AddCommand final : public ICommand {
    public:
        std::string_view name()        const override;
        std::string_view description() const override;
        void execute(CLIContext& ctx, const Args& args) override;
};

class DelCommand final : public ICommand {
    public:
        std::string_view name()        const override;
        std::string_view description() const override;
        void execute(CLIContext& ctx, const Args& args) override;
        std::vector<std::string> complete(const CLIContext *ctx, const Args& args) const override;
};

class ListCommand final : public ICommand {
    public:
        std::string_view name()        const override;
        std::string_view description() const override;
        void execute(CLIContext& ctx, const Args& args) override;
};

class SaveCommand final : public ICommand {
    public:
        std::string_view name()        const override;
        std::string_view description() const override;
        void execute(CLIContext& ctx, const Args& args) override;
};

class PatchCommand final : public ICommand {
    public:
        std::string_view name()        const override;
        std::string_view description() const override;
        void execute(CLIContext& ctx, const Args& args) override;
};

class SetCommand final : public ICommand {
    public:
        std::string_view name()        const override;
        std::string_view description() const override;
        void execute(CLIContext& ctx, const Args& args) override;
        std::vector<std::string> complete(const CLIContext *ctx, const Args& args) const override;
};
