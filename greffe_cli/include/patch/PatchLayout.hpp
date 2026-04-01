#pragma once

#include <cstdint>
#include <vector>
#include "PatchPlan.hpp"
#include "ProjectInfo.hpp"
#include "TargetManager.hpp"


class SharedStub {
    public:
        SharedStub(std::shared_ptr<IArchStubs> stub, uint64_t current_offset) : 
                   _name(stub->name())
                ,  _offset(stub->align_offset(current_offset)) {
            // _bytecode = generate_shstu
        };

        const std::string_view &        name()     const { return (_name); };
        uint64_t                        offset()   const { return (_offset); };
        const std::vector<uint8_t>     &bytecode() const { return (_bytecode); };

    private:
        const std::string_view &    _name;     // stub-related name
        std::vector<uint8_t>        _bytecode; // instructions
        uint64_t                    _offset; 
};

class PatchLayout {
    public:
        PatchLayout(const ProjectInfo& pinfo, TargetManager& targets);

        void create_patch_entry(PatchPlan *plan);
    private:
        const SharedStub *get_shared_stub(std::shared_ptr<IArchStubs> stub);
        const SharedStub *create_shared_stub(std::shared_ptr<IArchStubs> stub);

        const ProjectInfo&                _pinfo;
        uint64_t                          _patch_offset = 0;
        const std::vector<PatchPlan>&     _patch_plans;
        std::vector<SharedStub>           _shared_stubs;
};