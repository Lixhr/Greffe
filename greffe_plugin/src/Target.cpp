#include "Target.hpp"


Target::Target(std::string name, uint64_t ea, uint64_t end_ea)
    : _name(std::move(name))
    , _ea(ea)
    , _end_ea(end_ea) {}

const std::string& Target::name()   const { return _name; }
uint64_t           Target::ea()     const { return _ea; }
uint64_t           Target::end_ea() const { return _end_ea; }

