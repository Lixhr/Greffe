#include "SharedStub.hpp"
#include <vector>

SharedStub::SharedStub(std::shared_ptr<IArchStubs> s, uint64_t initial_offset) {
    stubs = std::move(s);
    set_offset(stubs->align_offset(initial_offset));
    _name = stubs->name();
}

std::string_view            SharedStub::name() const { return (_name); }
const std::vector<uint8_t>  SharedStub::bytes() const { return (_bytes); }

uint64_t                    SharedStub::end() const { 
    return (_offset + _bytes.size()); 
}
