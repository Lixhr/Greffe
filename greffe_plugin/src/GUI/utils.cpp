#include "utils.hpp"
#include "ua.hpp"
#include "bytes.hpp"
#include <lines.hpp>
#include "offset.hpp"
#include "PatchLayout.hpp"
#include "name.hpp"

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

void write_data_patch(ea_t addr, const uint8_t *bytes, size_t size, bgcolor_t color) {
    del_items(addr, DELIT_SIMPLE, size);
    patch_bytes(addr, bytes, size);
    set_range_color(addr, addr + size, color);
}

void commit_gui(PatchLayout &layout) {
    layout.sort_queue_by_type();

    layout.foreach_queue([](PatchLayoutEntry& e) {
        write_code_patch(e.ea(), e.bytes().data(), e.end_ea());

        bgcolor_t color = 0;

        switch (e.type()) {
            case PLEType::entry_branch:     color = Color::TARGET;       break; 
            case PLEType::entry_handlerbin: color = Color::HANDLER_CODE; break;

            case PLEType::entry_shstub: {    color = Color::PATCHED;
                SharedStub &stub = static_cast<SharedStub&>(e);
                set_name(stub.ea(), (stub.name() + "_shstub").c_str());
                break; 
            }

            case PLEType::entry_plan: {     color = Color::TARGET;
                PatchPlan &plan = static_cast<PatchPlan&>(e);
                set_name(plan.ea(), plan.name.c_str());
                set_range_color(e.ea(), e.end_ea(), color);

                // bzero the handler's pointer
                size_t sizeof_ptr = e.stubs->sizeof_ptr();
                del_items(plan.handler_ptr_addr, DELIT_SIMPLE, sizeof_ptr);
                set_range_color(plan.handler_ptr_addr, 
                                plan.handler_ptr_addr + sizeof_ptr,
                                Color::HANDLER_CODE);
                op_plain_offset(plan.handler_ptr_addr, 0, 0);
                return;
            }
        }
        set_range_color(e.ea(), e.end_ea(), color);
    });
}