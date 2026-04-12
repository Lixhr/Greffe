#include "Target.hpp"


Target::Target(std::string name, ea_t ea, ea_t end_ea)
    : _name(std::move(name))
    , _ea(ea)
    , _end_ea(end_ea) {}

const std::string& Target::name()   const { return _name; }
ea_t               Target::ea()     const { return _ea; }
ea_t               Target::end_ea() const { return _end_ea; }

