#include "PatchLayout.hpp"
#include "TrampolineBuilder.hpp"
#include "patch/arch/StubsFactory.hpp"
#include <ida.hpp>
#include <bytes.hpp>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include "utils.hpp"
#include "funcs.hpp"
#include "name.hpp"
#include "HandlerCompiler.hpp"
#include "GreffeCTX.hpp"
#include "db.hpp"

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

const std::vector<SharedStub*>  PatchLayout::shstubs()     const { return filter_entries<SharedStub, entry_shstub>    (_entries); }
const std::vector<HandlerBin*>  PatchLayout::handlers()    const { return filter_entries<HandlerBin, entry_handlerbin>(_entries); }

const std::vector<PatchBranch*> PatchLayout::branches()    const { return filter_entries<PatchBranch,entry_branch>    (_entries); }
const std::vector<PatchPlan*>   PatchLayout::patch_plans() const { return filter_entries<PatchPlan,  entry_plan>      (_entries); }


bool PatchLayout::overlaps_vec(const std::vector<unique_ple_t>& vec, ea_t s, ea_t e) const {
    for (const auto& entry : vec) {
        ea_t es = entry->ea();
        ea_t ee = entry->end_ea();
        if (s < ee && e > es)
            return true;
    }
    return false;
}

bool PatchLayout::overlaps_any(ea_t s, ea_t e) const {
    return overlaps_vec(_entries, s, e) || overlaps_vec(_queue, s, e);
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
    auto probe = plan->stubs->build_shared_stub(0);
    ea_t addr  = _regions.alloc_best_fit(plan->stubs->instr_alignment(), probe.size());

    SharedStub* new_stub = static_cast<SharedStub*>(
        queue_entry(std::make_unique<SharedStub>(plan->stubs, addr))
    );
    return new_stub;
}

static void check_collision(const std::vector<unique_ple_t>& vec, ea_t addr, ea_t end) {
    auto pos = std::lower_bound(vec.begin(), vec.end(), addr,
        [](const unique_ple_t& e, ea_t val) { return e->ea() < val; });

    if (pos != vec.begin()) {
        const auto& prev = *std::prev(pos);
        if (prev->end_ea() > addr)
            throw std::runtime_error(
                "Entry at " + hex(addr) +
                " overlaps existing entry at " + hex(prev->ea()));
    }

    if (pos != vec.end()) {
        if (end > (*pos)->ea())
            throw std::runtime_error(
                "Entry at " + hex(addr) +
                " overlaps existing entry at " + hex((*pos)->ea()));
    }
}

PatchLayoutEntry* PatchLayout::queue_entry(unique_ple_t entry) {
    ea_t addr = entry->ea();

    check_collision(_entries, addr, entry->end_ea());
    check_collision(_queue,   addr, entry->end_ea());

    auto pos = std::lower_bound(_queue.begin(), _queue.end(), addr,
        [](const unique_ple_t& e, ea_t val) { return e->ea() < val; });

    return _queue.insert(pos, std::move(entry))->get();
}

void PatchLayout::sort_queue_by_type() {
    std::stable_sort(_queue.begin(), _queue.end(),
        [](const unique_ple_t& a, const unique_ple_t& b) {
            return a->type() < b->type();
        });
}

void PatchLayout::commit() {
    for (auto& e : _queue) {
        ea_t addr = e->ea();
        auto pos = std::lower_bound(_entries.begin(), _entries.end(), addr,
            [](const unique_ple_t& x, ea_t val) { return x->ea() < val; });
        _entries.insert(pos, std::move(e));
    }

    _queue.clear();

    for (auto& r : _regions.regions_mutable())
        r.clear_region();
}

void PatchLayout::rollback() {
    for (const auto& e : _queue)
        if (e->type() != PLEType::entry_branch)
            _regions.reclaim(e->ea(), e->end_ea());
    _queue.clear();
}

void PatchLayout::create_patch_entry(PatchPlan *plan) {
    const SharedStub *shstub = get_shstub(plan);
    if (!shstub)
        shstub = create_shstub(plan);

    uint8_t alignment = plan->stubs->instr_alignment();

    // best-fit: smallest region first.
    const auto& regions = _regions.regions();
    std::vector<size_t> order(regions.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        return regions[a].size() < regions[b].size();
    });

    for (size_t idx : order) {
        const PatchRegion& r = regions[idx];
        ea_t candidate = r.base;
        if (alignment > 1)
            candidate = (candidate + alignment - 1) & ~static_cast<ea_t>(alignment - 1);
        if (candidate >= r.end)
            continue;

        plan->set_addr(candidate);
        PatchBranch branch = TrampolineBuilder::branch_to_trampoline(*plan);
        TrampolineBuilder::init_trampoline(*plan, *shstub);
        TrampolineBuilder::relocate_and_branch_back(*plan);
        uint64_t size = plan->bytes().size();

        if (candidate + size > r.end) {
            plan->bytes().clear();
            plan->relocd_instr.clear();
            continue;
        }

        if (_regions.overlaps_any(branch.ea(), branch.end_ea())) {
            std::ostringstream ss;
            ss << "Branch at 0x" << std::hex << branch.ea()
               << " overlaps an existing patch region";
            throw std::runtime_error(ss.str());
        }

        queue_entry(std::make_unique<PatchBranch>(std::move(branch)));
        _regions.commit_alloc(candidate, size);
        return;
    }

    throw std::runtime_error("PatchRegionSet: all patch regions exhausted");
}

void PatchLayout::free_entry(ea_t start, ea_t end) {
    _regions.reclaim(start, end);
}

void PatchLayout::revert_entries(const std::vector<std::pair<ea_t, ea_t>>& ranges) {
    for (auto& [start, end] : ranges)
        patch_revert_range(start, end);
}

void PatchLayout::free_handler_bin() {
    for (PatchPlan *plan : patch_plans()) {
        if (!plan->handler_addr) continue;
        size_t n = plan->stubs->sizeof_ptr();
        std::vector<uint8_t> zeroes(n, 0);
        uint8_t *slot = plan->bytes().data() + (plan->handler_ptr_addr - plan->ea());
        std::fill(slot, slot + n, 0);
        write_data_patch(plan->handler_ptr_addr, zeroes.data(), n);
        plan->handler_addr = 0;
    }
    for (HandlerBin *bin : handlers()) {
        std::vector<uint8_t> zeroes(bin->size(), 0);
        write_data_patch(bin->ea(), zeroes.data(), bin->size());
    }
    free_if([](PatchLayoutEntry& e){ return e.type() == PLEType::entry_handlerbin; });
}

HandlerBin *PatchLayout::place_handler_bin() {
    HandlerBin bin = HandlerCompiler::build(g_ctx->layout.patch_plans(),
                                            g_ctx->pinfo);

    ea_t addr = _regions.alloc_largest(0x10, static_cast<ea_t>(bin.size()));
    bin.set_addr(addr);
    bin.rebase(addr);

    return static_cast<HandlerBin*>(
           queue_entry(std::make_unique<HandlerBin>(std::move(bin))));
}

static std::shared_ptr<IArchStubs>
resolve_stubs(const DB_PatchLayout *idx,
              const uint8_t *plans_raw, size_t plans_sz,
              ProjectInfo &pinfo)
{
    for (size_t i = 0; i < idx->n_entries; i++)
        if (idx->entries[i].type == entry_branch)
            return StubsFactory::create(pinfo.getArchKeyAt(idx->entries[i].base));

    if (plans_raw && plans_sz >= sizeof(DB_PatchPlan_meta)) {
        const auto *first = reinterpret_cast<const DB_PatchPlan_meta *>(plans_raw);
        return StubsFactory::create(pinfo.getArchKeyAt(first->target_ea));
    }
    return nullptr;
}

static unique_ple_t
load_plan(ea_t base, size_t sz,
          const uint8_t *&cursor, const uint8_t *end,
          std::shared_ptr<IArchStubs> stubs, ProjectInfo &pinfo)
{
    if (!cursor || cursor + sizeof(DB_PatchPlan_meta) > end)
        return nullptr;
    const auto *meta = reinterpret_cast<const DB_PatchPlan_meta *>(cursor);
    if (cursor + sizeof(DB_PatchPlan_meta) + meta->name_len > end)
        return nullptr;

    if (!stubs)
        stubs = StubsFactory::create(pinfo.getArchKeyAt(meta->target_ea));

    auto plan = std::make_unique<PatchPlan>(
        base, sz,
        std::string(meta->name, meta->name_len),
        meta->target_ea, meta->target_end_ea,
        meta->handler_ptr_addr,
        std::move(stubs));

    cursor += sizeof(DB_PatchPlan_meta) + meta->name_len;
    return plan;
}

static void save_layout_index(netnode &node, const std::vector<PatchLayoutEntry *> &entries)
{
    size_t n        = entries.size();
    size_t alloc_sz = sizeof(DB_PatchLayout) + n * sizeof(DB_PatchLayout::DB_PatchLayout_entry);
    auto  *blob     = static_cast<DB_PatchLayout *>(malloc(alloc_sz));
    if (!blob) return;

    blob->n_entries = n;
    for (size_t i = 0; i < n; i++) {
        blob->entries[i].base = entries[i]->ea();
        blob->entries[i].end  = entries[i]->end_ea();
        blob->entries[i].type = entries[i]->type();
    }
    node.setblob(blob, alloc_sz, 0, DB_IDs::PatchLayout_entry);
    free(blob);
}

static void save_plan_metadata(netnode &node, const std::vector<PatchPlan *> &plans)
{
    size_t total = 0;
    for (const PatchPlan *p : plans)
        total += sizeof(DB_PatchPlan_meta) + p->name.size();

    auto *buf = static_cast<uint8_t *>(malloc(total));
    if (!buf) return;

    uint8_t *cursor = buf;
    for (const PatchPlan *p : plans) {
        auto *meta             = reinterpret_cast<DB_PatchPlan_meta *>(cursor);
        meta->target_ea        = p->target_ea;
        meta->target_end_ea    = p->target_end_ea;
        meta->handler_ptr_addr = p->handler_ptr_addr;
        meta->name_len         = static_cast<uint32_t>(p->name.size());
        memcpy(meta->name, p->name.data(), p->name.size());
        cursor += sizeof(DB_PatchPlan_meta) + p->name.size();
    }
    node.setblob(buf, total, 0, DB_IDs::PatchPlans_entry);
    free(buf);
}

void PatchLayout::load_from_db(netnode &node) {
    _regions.load_from_db(node);

    size_t main_sz = 0;
    auto  *main    = static_cast<DB_PatchLayout *>(node.getblob(nullptr, &main_sz, 0, DB_IDs::PatchLayout_entry));
    if (!main || main_sz < sizeof(DB_PatchLayout)) { qfree(main); return; }

    size_t plans_sz = 0;
    auto  *plans    = static_cast<uint8_t *>(node.getblob(nullptr, &plans_sz, 0, DB_IDs::PatchPlans_entry));

    auto stubs = resolve_stubs(main, plans, plans_sz, _pinfo);

    const uint8_t *cursor = plans;
    const uint8_t *end    = plans ? plans + plans_sz : nullptr;

    std::vector<unique_ple_t> loaded;
    for (size_t i = 0; i < main->n_entries; i++) {
        const auto& e  = main->entries[i];
        size_t      sz = static_cast<size_t>(e.end - e.base);

        switch (e.type) {
        case entry_branch: {
            std::vector<uint8_t> bytes(sz);
            get_bytes(bytes.data(), sz, e.base);
            loaded.push_back(std::make_unique<PatchBranch>(e.base, std::move(bytes), 0));
            break;
        }
        case entry_shstub:
            if (stubs)
                loaded.push_back(std::make_unique<SharedStub>(stubs, e.base));
            break;
        case entry_plan:
            if (auto p = load_plan(e.base, sz, cursor, end, stubs, _pinfo))
                loaded.push_back(std::move(p));
            break;
        case entry_handlerbin:
            break;
        }
    }

    std::sort(loaded.begin(), loaded.end(),
        [](const unique_ple_t& a, const unique_ple_t& b){ return a->ea() < b->ea(); });
    _entries = std::move(loaded);

    qfree(main);
    qfree(plans);
}

void PatchLayout::save_db(netnode &node) {
    _regions.save_db(node);

    std::vector<PatchLayoutEntry *> to_save;
    for (const auto& e : _entries)
        if (e->type() != entry_handlerbin)
            to_save.push_back(e.get());

    if (to_save.empty()) return;

    save_layout_index(node, to_save);

    auto plans = patch_plans();
    if (!plans.empty())
        save_plan_metadata(node, plans);
}