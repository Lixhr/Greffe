#include "SharedStub.hpp"
#include <vector>

SharedStub::SharedStub(std::shared_ptr<IArchStubs> stub) : _name(stub->name()) {}

std::string_view SharedStub::name() const { return (_name); }

