#include "PatchLayout.hpp"
#include "TrampolineBuilder.hpp"
#include <ida.hpp>
#include <bytes.hpp>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include "utils.hpp"
#include "funcs.hpp"
#include "name.hpp"
#include "HandlerCompiler.hpp"
#include "GreffeCTX.hpp"

PatchLayout::PatchLayout(ProjectInfo& pinfo)
    : _pinfo(pinfo)
    , _regions(pinfo.getRegionsSet())
{}

const std::vector<PatchPlan>&   PatchLayout::patch_plans() const { return _patch_plans; }
std::vector<PatchPlan>&         PatchLayout::patch_plans()       { return _patch_plans; }
const std::vector<SharedStub>&  PatchLayout::shstubs()     const { return _shstubs;     }
const std::vector<PatchBranch>& PatchLayout::branches()    const { return _branches;    }
const HandlerBin&               PatchLayout::handler()     const { return _handlerbin; }

bool PatchLayout::overlaps_any(ea_t s, ea_t e) const {
    auto check = [s, e](const auto& entries) {
        for (const auto& entry : entries) {
            ea_t es = entry.addr();
            ea_t ee = es + static_cast<ea_t>(entry.bytes().size());
            if (s < ee && e > es)
                return true;
        }
        return false;
    };
    return check(_branches) || check(_patch_plans) || check(_shstubs);
}

const SharedStub *PatchLayout::get_shstub(PatchPlan *plan) {
    auto it = std::find_if(_shstubs.begin(), _shstubs.end(),
        [plan](const SharedStub& s) { return s.name() == plan->stubs->name(); });
    return it != _shstubs.end() ? &*it : nullptr;
}

const SharedStub *PatchLayout::create_shstub(PatchPlan *plan) {
    // Build at a dummy address to determine size
    // then allocate at the real address.
    auto probe = plan->stubs->build_shared_stub(0);
    ea_t addr  = _regions.alloc(plan->stubs->instr_alignment(), probe.size());

    _shstubs.push_back(SharedStub(plan->stubs, addr));

    SharedStub& shstub = _shstubs.back();

    // write_code_patch(shstub.addr(), shstub.bytes().data(), shstub.bytes().size(), Color::PATCHED);
    // set_name(shstub.addr(), ("shstub_" + std::string(shstub.name())).c_str());

    return &shstub;
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

    auto overlaps = [](const PatchBranch& a, const PatchBranch& b) {
        return a.trampoline_ret_addr > b.addr() &&
               b.trampoline_ret_addr > a.addr();
    };

    if (pos != _branches.end() && pos->addr() == branch.addr())
        throw std::runtime_error(
            "Branch already exists at " + hex(branch.addr()));

    if (pos != _branches.begin() && overlaps(*std::prev(pos), branch))
        throw std::runtime_error(
            "Branch at " + hex(branch.addr()) +
            " overlaps with existing branch at " + hex(std::prev(pos)->addr()));

    if (pos != _branches.end() && overlaps(branch, *pos))
        throw std::runtime_error(
            "Branch at " + hex(branch.addr()) +
            " overlaps with existing branch at " + hex(pos->addr()));

    _branches.insert(pos, std::move(branch));
}

void PatchLayout::create_patch_entry(PatchPlan *plan) {
    const SharedStub *shstub = get_shstub(plan);
    if (!shstub)
        shstub = create_shstub(plan);

    // Build the trampoline, retrying in the next region if it doesn't fit.
    _regions.select_closest(plan->target.ea());

    PatchBranch branch{0, {}, plan->trampoline_ret_addr};
    for (;;) {
        _regions.align(plan->stubs->instr_alignment());
        plan->set_addr(_regions.current_addr());

        branch = TrampolineBuilder::branch_to_trampoline(*plan);

        {
            ea_t bstart = branch.addr();
            ea_t bend   = bstart + branch.bytes().size();
            if (_regions.overlaps_any(bstart, bend)) {
                std::ostringstream ss;
                ss << "Branch at 0x" << std::hex << bstart
                   << " overlaps an existing patch region";
                throw std::runtime_error(ss.str());
            }
        }

        TrampolineBuilder::init_trampoline(*plan, *shstub);
        TrampolineBuilder::relocate_and_branch_back(*plan);

        if (_regions.fits(plan->bytes().size())) {
            _regions.advance(plan->bytes().size());
            break;
        }
        _regions.next_region();
    }

    // ea_t                     branch_addr  = branch.addr();
    std::vector<uint8_t>     branch_bytes = branch.bytes();
    insert_branch(std::move(branch));

    // write_code_patch(plan->addr(), plan->bytes().data(), plan->bytes().size(), Color::PATCHED);
    // set_name(plan->addr(), plan->target.name().c_str());

    // write_code_patch(branch_addr,  branch_bytes.data(),  branch_bytes.size(),  Color::RELOCATED);

    _regions.refresh_all_data_items();
}

void PatchLayout::place_handler_bin() {
    _handlerbin = HandlerCompiler::build(g_ctx->layout.patch_plans(),
                                         g_ctx->pinfo);

    ea_t addr = _regions.alloc(0x10, static_cast<ea_t>(_handlerbin.size()));
    _handlerbin.set_addr(addr);
    // set_code_region(bin.addr(), bin.addr() + bin.size());
    // write_code_patch(bin.addr(), bin.bytes().data(), bin.bytes().size(), Color::HANDLER_CODE);
}

