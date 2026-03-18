#pragma once

#include <string>
#include <vector>
#include <cstddef>

#include "CLIDispatcher.hpp"

class CompletionRegistry {
    public:
        static CompletionRegistry& instance();

        void install(const CLIDispatcher& dispatcher);

        CompletionRegistry(const CompletionRegistry&)            = delete;
        CompletionRegistry& operator=(const CompletionRegistry&) = delete;

    private:
        CompletionRegistry() = default;

        static char** readline_completion(const char* text, int start, int end);
        static char* completion_generator(const char* text, int state);

        const CLIDispatcher*     _dispatcher = nullptr;
        std::vector<std::string> _candidates;
        std::size_t              _idx = 0;
};
