#include "PatchLayout.hpp"

PatchLayout::PatchLayout(const ProjectInfo& pinfo, TargetManager& targets) :
                         _pinfo(pinfo),
                         _patch_plans(targets.plans()),
                         _patch_offset(pinfo.getPatchBase() - pinfo.getBinBase())
                        {}

void PatchLayout::create_patch_entry(PatchPlan *plan) {
    // const SharedStub shared = *get_shared_stub(plan->stubs->name());
    // if (!shared)
    //     shared = 
}

// const SharedStub *PatchLayout::create_shared_stub(std::shared_ptr<IArchStubs> stub) {
//     SharedStub()
//     _shared_stubs.push_back(SharedStub())
// }

const SharedStub *PatchLayout::get_shared_stub(std::shared_ptr<IArchStubs> stub) {
    auto it = std::find_if(_shared_stubs.begin(), _shared_stubs.end(),
        [&stub](const SharedStub& shstb) { return shstb.name() == stub->name(); });

    if (it != _shared_stubs.end())
        return (&(*it));

    // shared stub currently not created
    SharedStub new_stub(stub, _patch_offset);
    _patch_offset = new_stub.offset() + new_stub.bytecode().size();

    _shared_stubs.push_back(new_stub);
    return (&_shared_stubs.back());
}