#include "utils.hpp"
#include "ua.hpp"
#include "bytes.hpp"
#include <lines.hpp>
#include "offset.hpp"
#include "PatchLayout.hpp"
#include "name.hpp"
#include "funcs.hpp"

void set_code_region(ea_t start, ea_t end) {
    if (start == end)
        return;

    while (start < end) {
        int size = create_insn(start);
        if (size == 0) start++;
        else           start += size;
    }
}

void write_code_patch(ea_t ea, const uint8_t *bytes, ea_t end_ea) {
    size_t size = end_ea - ea;

    del_items(ea, DELIT_SIMPLE, size);
    patch_bytes(ea, bytes, size);
    set_code_region(ea, end_ea);
}

void write_data_patch(ea_t addr, const uint8_t *bytes, size_t size) {
    del_items(addr, DELIT_SIMPLE, size);
    patch_bytes(addr, bytes, size);
}

void commit_gui(PatchLayout &layout) {
    layout.sort_queue_by_type();

    layout.foreach_queue([](PatchLayoutEntry& e) {
        write_code_patch(e.ea(), e.bytes().data(), e.end_ea());

        switch (e.type()) {
            case PLEType::entry_shstub: {
                SharedStub &stub = static_cast<SharedStub&>(e);
                set_name(stub.ea(), (stub.name() + "_shstub").c_str());
                break;
            }
            case PLEType::entry_plan: {
                PatchPlan &plan = static_cast<PatchPlan&>(e);
                set_name(plan.ea(), plan.name.c_str());
                size_t sizeof_ptr = e.stubs->sizeof_ptr();
                del_items(plan.handler_ptr_addr, DELIT_SIMPLE, sizeof_ptr);
                op_plain_offset(plan.handler_ptr_addr, 0, 0);
                break;
            }
            default: break;
        }
    });

    // plans are already in _entries: committed at add
    // handler_addr is define on patch
    for (PatchPlan *plan : layout.patch_plans()) {
        if (plan->handler_addr) {
            add_func(plan->handler_addr);
            set_name(plan->handler_addr & ~1, (plan->name + "_handler").c_str());
        }
    }

    request_refresh(IWID_DISASM);
}