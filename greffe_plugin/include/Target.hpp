#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <ostream>

struct ContextEntry {
    uint64_t             ea;
    std::vector<uint8_t> raw;
    std::string          mode;
    bool        is_xref_target = false;
};

class Target {
    public:
        Target(std::string name, uint64_t ea, uint64_t end_ea);

        const std::string& name()   const;
        uint64_t           ea()     const;
        uint64_t           end_ea() const;

    private:
        std::string _name;
        uint64_t    _ea;
        uint64_t    _end_ea;
};

struct TargetView {
    const Target& target;
    int           bits;
};

std::ostream& operator<<(std::ostream& os, const TargetView& v);