#include "PatchLayout.hpp"
#include "TrampolineBuilder.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <iomanip>

PatchLayout::PatchLayout(const ProjectInfo& pinfo, TargetManager& targets) :
                         _pinfo(pinfo)
                       , _patch_plans(targets.plans())
                       , _patch_offset(pinfo.getBits(), pinfo.getPatchBase())
                        {}

const std::vector<PatchPlan> & PatchLayout::patch_plans() const { return _patch_plans; }
const std::vector<SharedStub>& PatchLayout::shstubs()     const { return _shstubs;     }
const std::vector<PatchBranch>& PatchLayout::branches()   const { return _branches;    }
uint64_t                       PatchLayout::current_offset() const { return _patch_offset; }

uint64_t PatchLayout::offset_to_addr(uint64_t offset)     const {
    return offset + _pinfo.getPatchBase();
}


void PatchLayout::set_trampoline_addr(PatchPlan* plan) {

    _patch_offset = plan->stubs->align_offset(_patch_offset);

    plan->set_addr(offset_to_addr(_patch_offset));
}

const SharedStub *PatchLayout::get_shstub(PatchPlan *plan) {
    auto it = std::find_if(_shstubs.begin(), _shstubs.end(),
        [plan](const SharedStub& shstub) {
            return shstub.name() == plan->stubs->name();
        });

    if (it != _shstubs.end())
        return &*it;
    return nullptr;
}

const SharedStub *PatchLayout::create_shstub(PatchPlan *plan) {
    uint64_t offset = plan->stubs->align_offset(_patch_offset);

    _shstubs.push_back(SharedStub(plan->stubs,
                                  offset, 
                                  offset_to_addr(offset)));

    SharedStub &new_shstub = _shstubs.back();
    return &new_shstub;
}

void PatchLayout::insert_branch(PatchBranch branch) {
    auto cmp = [](const PatchBranch& a, const PatchBranch& b) {
        return a.addr() < b.addr();
    };

    auto pos = std::lower_bound(_branches.begin(), _branches.end(), branch, cmp);

    auto hex = [](uint64_t v) {
        std::ostringstream ss;
        ss << "0x" << std::hex << v;
        return ss.str();
    };

    if (pos != _branches.begin()) {
        const auto& pred = *std::prev(pos);
        if (pred.addr() + pred.bytes().size() > branch.addr())
            throw std::runtime_error(
                "Branch at " + hex(branch.addr()) +
                " overlaps with existing branch at " + hex(pred.addr()));
    }

    if (pos != _branches.end()) {
        if (branch.addr() + branch.bytes().size() > pos->addr())
            throw std::runtime_error(
                "Branch at " + hex(branch.addr()) +
                " overlaps with existing branch at " + hex(pos->addr()));
    }

    _branches.insert(pos, std::move(branch));
}

void PatchLayout::create_patch_entry(PatchPlan *plan) {
    // get the shared-stubs
    const SharedStub *shstub = get_shstub(plan);
    if (!shstub) {
        shstub = create_shstub(plan);
        _patch_offset += shstub->bytes().size();
    }

    set_trampoline_addr(plan);
    insert_branch(TrampolineBuilder::branch_to_trampoline(*plan));
    _patch_offset += TrampolineBuilder::init_trampoline(*plan, *shstub);
    _patch_offset += TrampolineBuilder::relocate_and_branch_back(*plan);
}
