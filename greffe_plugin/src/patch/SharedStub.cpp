#include "SharedStub.hpp"

SharedStub::SharedStub(std::shared_ptr<IArchStubs> s, ea_t addr) {
    stubs  = std::move(s);
    _name  = stubs->name();
    _addr  = addr;
    _bytes = stubs->build_shared_stub(addr);
}

std::string_view SharedStub::name() const { return _name; }
