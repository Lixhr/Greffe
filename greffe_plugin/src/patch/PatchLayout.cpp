#include "PatchLayout.hpp"
#include "TrampolineBuilder.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>

PatchLayout::PatchLayout(const ProjectInfo& pinfo, TargetManager& targets)
    : _pinfo(pinfo)
    , _patch_plans(targets.plans())
    , _cursor(pinfo.getRegions())
{}

const std::vector<PatchPlan>&   PatchLayout::patch_plans() const { return _patch_plans; }
std::vector<PatchPlan>&         PatchLayout::patch_plans()       { return _patch_plans; }
const std::vector<SharedStub>&  PatchLayout::shstubs()     const { return _shstubs;     }
const std::vector<PatchBranch>& PatchLayout::branches()    const { return _branches;    }

const SharedStub *PatchLayout::get_shstub(PatchPlan *plan) {
    auto it = std::find_if(_shstubs.begin(), _shstubs.end(),
        [plan](const SharedStub& s) { return s.name() == plan->stubs->name(); });
    return it != _shstubs.end() ? &*it : nullptr;
}

const SharedStub *PatchLayout::create_shstub(PatchPlan *plan) {
    // Build at a dummy address to determine size
    // then allocate at the real address.
    auto probe = plan->stubs->build_shared_stub(0);
    ea_t addr  = _cursor.alloc(plan->stubs->instr_alignment(), probe.size());

    _shstubs.push_back(SharedStub(plan->stubs, addr));
    return &_shstubs.back();
}

void PatchLayout::insert_branch(PatchBranch branch) {
    auto cmp = [](const PatchBranch& a, const PatchBranch& b) {
        return a.addr() < b.addr();
    };

    auto pos = std::lower_bound(_branches.begin(), _branches.end(), branch, cmp);

    auto hex = [](ea_t v) {
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
    const SharedStub *shstub = get_shstub(plan);
    if (!shstub)
        shstub = create_shstub(plan);

    // Build the trampoline, retrying in the next region if it doesn't fit.
    // branch_to_trampoline depends on plan->addr() so it's inside the loop too.
    PatchBranch branch{0, {}};
    for (;;) {
        _cursor.align(plan->stubs->instr_alignment());
        plan->set_addr(_cursor.current_addr());
        plan->bytes().clear();

        branch = TrampolineBuilder::branch_to_trampoline(*plan);
        TrampolineBuilder::init_trampoline(*plan, *shstub);
        TrampolineBuilder::relocate_and_branch_back(*plan);

        if (_cursor.fits(plan->bytes().size())) {
            _cursor.advance(plan->bytes().size());
            break;
        }
        _cursor.next_region();
    }

    insert_branch(std::move(branch));
}

void PatchLayout::rebuild() {
    _cursor.reset();
    _shstubs.clear();
    _branches.clear();
    for (auto& plan : _patch_plans)
        create_patch_entry(&plan);
}

void PatchLayout::place_handler_bin(HandlerBin& bin) {
    ea_t addr = _cursor.alloc(0x10, static_cast<ea_t>(bin.size()));
    bin.set_addr(addr);
}
