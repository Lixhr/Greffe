#pragma once

#include "ICommand.hpp"

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
