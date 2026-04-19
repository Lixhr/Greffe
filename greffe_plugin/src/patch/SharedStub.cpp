#include "SharedStub.hpp"

SharedStub::SharedStub(std::shared_ptr<IArchStubs> s, ea_t addr) : PatchLayoutEntry(PLEType::entry_shstub) {
    stubs  = std::move(s);
    _name  = stubs->name();
    _addr  = addr;
    _bytes = stubs->build_shared_stub(addr);
}

std::string SharedStub::name() const { return _name; }
