#pragma once

#include "patch/arch/IArchStubs.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <ostream>

struct ContextEntry {
    uint64_t    ea;
    std::string raw;
    std::string mode;
    bool        is_xref_target = false;
};

class Target {
    public:
        Target(std::string name, uint64_t ea, uint64_t end_ea,
               std::vector<ContextEntry> context,
               std::shared_ptr<IArchStubs> stubs);

        const std::string&               name()    const;
        uint64_t                         ea()      const;
        uint64_t                         end_ea()  const;
        const std::vector<ContextEntry>& context() const;
        IArchStubs&                      stubs()   const;

    private:
        std::string                 _name;
        uint64_t                    _ea;
        uint64_t                    _end_ea;
        std::vector<ContextEntry>   _context;
        std::shared_ptr<IArchStubs> _stubs;
};

struct TargetView {
    const Target& target;
    int           bits;
};

std::ostream& operator<<(std::ostream& os, const TargetView& v);