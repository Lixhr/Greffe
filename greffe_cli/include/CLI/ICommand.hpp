#pragma once
 
#include <string>
#include <string_view>
#include <vector>
 
#include "CLIContext.hpp"
 
class ICommand {
    public:
        virtual ~ICommand() = default;
    
        virtual std::string_view name()        const = 0;
        virtual std::string_view description() const = 0;
    
        virtual void execute(CLIContext& ctx, const Args&) = 0;
    
        virtual std::vector<std::string> complete(const CLIContext *ctx, const Args&) const ;
};
 