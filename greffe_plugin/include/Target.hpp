#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "ida.hpp"

struct ContextEntry {
    ea_t                 ea;
    std::vector<uint8_t> raw;
    std::string          mode;
    bool        is_xref_target = false;
};

class Target {
    public:
        Target(std::string name, ea_t ea, ea_t end_ea);

        const std::string& name()   const;
        ea_t               ea()     const;
        ea_t               end_ea() const;

    private:
        std::string _name;
        ea_t        _ea;
        ea_t        _end_ea;
};