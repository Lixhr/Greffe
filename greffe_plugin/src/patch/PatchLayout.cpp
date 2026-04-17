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

static std::string hex(ea_t v) {
    std::ostringstream ss;
    ss << "0x" << std::hex << v;
    return ss.str();
}

PatchLayout::PatchLayout(ProjectInfo& pinfo)
    : _pinfo(pinfo)
    , _regions(pinfo.getRegionsSet())
{}

template <typename T, PLEType Type>
static std::vector<T*> filter_entries(const std::vector<unique_ple_t>& entries) {
    std::vector<T*> result;
    for (const auto& e : entries)
        if (e->type() == Type)
            result.push_back(static_cast<T*>(e.get()));
    return result;
}

const std::vector<PatchPlan*>   PatchLayout::patch_plans() const { return filter_entries<PatchPlan,  entry_plan>      (_entries); }
const std::vector<SharedStub*>  PatchLayout::shstubs()     const { return filter_entries<SharedStub, entry_shstub>    (_entries); }
const std::vector<PatchBranch*> PatchLayout::branches()    const { return filter_entries<PatchBranch,entry_branch>    (_entries); }
const std::vector<HandlerBin*>  PatchLayout::handlers()    const { return filter_entries<HandlerBin, entry_handlerbin>(_entries); }

bool PatchLayout::overlaps_any(ea_t s, ea_t e) const {
    (void)s;
    (void)e;
    // for (const auto& entry : g_ctx->layout._entries) {
    //     ea_t es = entry.addr();
    //     ea_t ee = es + static_cast<ea_t>(entry.bytes().size());
    //     if (s < ee && e > es)
    //         return true;
    // }
    return false;
}

const SharedStub *PatchLayout::get_shstub(PatchPlan *plan) {
    auto* it = entry_find_if([&](PatchLayoutEntry& e) {
        if (e.type() == entry_shstub)
            return static_cast<const SharedStub&>(e).name() == plan->stubs->name();
        return false;
    }); 

    return static_cast<const SharedStub *>(it);
}

const SharedStub *PatchLayout::create_shstub(PatchPlan *plan) {
    // Build at a dummy address to determine size
    // then allocate at the real address.
    auto probe = plan->stubs->build_shared_stub(0);
    ea_t addr  = _regions.alloc(plan->stubs->instr_alignment(), probe.size());

    SharedStub* new_stub = static_cast<SharedStub*>(
        add_entry(std::make_unique<SharedStub>(plan->stubs, addr))
    );
    return new_stub;
}

PatchLayoutEntry* PatchLayout::add_entry(unique_ple_t entry) {
    ea_t addr = entry->addr();
    ea_t end  = addr + static_cast<ea_t>(entry->bytes().size());

    auto pos = std::lower_bound(_entries.begin(), _entries.end(), addr,
        [](const std::unique_ptr<PatchLayoutEntry>& e, ea_t val) {
            return e->addr() < val;
        });

    if (pos != _entries.begin()) {
        const auto& prev = *std::prev(pos);
        ea_t prev_end = prev->addr() + static_cast<ea_t>(prev->bytes().size());
        if (prev_end > addr)
            throw std::runtime_error(
                "Entry at " + hex(addr) +
                " overlaps existing entry at " + hex(prev->addr()));
    }

    if (pos != _entries.end()) {
        if (end > (*pos)->addr())
            throw std::runtime_error(
                "Entry at " + hex(addr) +
                " overlaps existing entry at " + hex((*pos)->addr()));
    }

    return _entries.insert(pos, std::move(entry))->get();
}

void PatchLayout::create_patch_entry(PatchPlan *plan) {
    const SharedStub *shstub = get_shstub(plan);
    if (!shstub)
        shstub = create_shstub(plan);

    // Build the trampoline, retrying in the next region if it doesn't fit.
    _regions.select_closest(plan->ea);

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

    std::vector<uint8_t>     branch_bytes = branch.bytes();
    add_entry(std::make_unique<PatchBranch>(std::move(branch)));

    _regions.refresh_all_data_items();
}

void PatchLayout::place_handler_bin() {
    HandlerBin bin = HandlerCompiler::build(g_ctx->layout.patch_plans(),
                                            g_ctx->pinfo);

    ea_t addr = _regions.alloc(0x10, static_cast<ea_t>(bin.size()));
    bin.set_addr(addr);
    add_entry(std::make_unique<HandlerBin>(std::move(bin)));
}

