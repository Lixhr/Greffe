#include "PatchLayout.hpp"
#include "TrampolineBuilder.hpp"

PatchLayout::PatchLayout(const ProjectInfo& pinfo, TargetManager& targets) :
                         _pinfo(pinfo)
                       , _patch_plans(targets.plans())
                       , _patch_offset(0)
                        {}

const std::vector<PatchPlan> & PatchLayout::patch_plans() const { return (_patch_plans);}
const std::vector<SharedStub>& PatchLayout::shstubs()     const { return (_shstubs);}

uint64_t PatchLayout::offset_to_addr(uint64_t offset)     const {
    return (offset + _pinfo.getPatchBase());
}


#include <iostream>
void PatchLayout::set_trampoline_addr(PatchPlan* plan) {

    _patch_offset = plan->stubs->align_offset(_patch_offset);

    plan->trampoline_addr = _patch_offset + _pinfo.getPatchBase();
}

const SharedStub *PatchLayout::get_shstub(PatchPlan *plan) {
    auto it = std::find_if(_shstubs.begin(), _shstubs.end(),
        [&plan](const SharedStub& shstub) { 
            return (shstub.name() == plan->stubs->name()); 
        });

    if (it != _shstubs.end())
        return (&(*it));
    return (NULL);
}

const SharedStub *PatchLayout::create_shstub(PatchPlan *plan) {
    uint64_t offset = plan->stubs->align_offset(_patch_offset);

    _shstubs.push_back(SharedStub(plan->stubs,
                                  offset, 
                                  _pinfo.getPatchBase() + offset));

    SharedStub &new_shstub = _shstubs.back();
    return (&new_shstub);
}


void PatchLayout::create_patch_entry(PatchPlan *plan) {
    // get the shared-stubs
    const SharedStub *shstub = get_shstub(plan);
    if (!shstub) {
        shstub = create_shstub(plan);
        _patch_offset += shstub->bytes().size();
    }


    set_trampoline_addr(plan);
    _patch_offset += TrampolineBuilder::init_trampoline(*plan, *shstub);
    // _patch_offset += 

    TrampolineBuilder::branch_to_trampoline(*plan);
}
