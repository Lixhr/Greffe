#pragma once

#include "ICommand.hpp"
#include "CLIDispatcher.hpp"

class HelpCommand final : public ICommand {
    public:
        explicit HelpCommand(const CLIDispatcher& dispatcher);

        std::string_view name()        const override;
        std::string_view description() const override;

        void execute(CLIContext& ctx, const Args& args) override;

    private:
        static void print_dispatcher(const CLIDispatcher& dispatcher,
                                     const std::string&   indent,
                                     const std::string&   prefix);

        const CLIDispatcher& _dispatcher;
};

class QuitCommand final : public ICommand {
    public:
        std::string_view name()        const override;
        std::string_view description() const override;
    
        void execute(CLIContext& ctx, const Args& args) override;
};