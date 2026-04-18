#include <ida.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include "utils.hpp"
#include "GreffeCTX.hpp"
#include "PatchPlan.hpp"

extern plugin_t PLUGIN;

static bgcolor_t color_for_entry(PatchLayoutEntry& e, ea_t ea) {
    if (e.type() == PLEType::entry_plan) {
        PatchPlan& plan = static_cast<PatchPlan&>(e);
        size_t sizeof_ptr = e.stubs->sizeof_ptr();
        if (ea >= plan.handler_ptr_addr && ea < plan.handler_ptr_addr + sizeof_ptr)
            return Color::HANDLER_CODE;
        return Color::TARGET;
    }
    switch (e.type()) {
        case PLEType::entry_branch:     return Color::TARGET;
        case PLEType::entry_handlerbin: return Color::HANDLER_CODE;
        case PLEType::entry_shstub:     return Color::PATCHED;
        default:                        return DEFCOLOR;
    }
}

struct RangeColorHooks : public event_listener_t {
    ssize_t idaapi on_event(ssize_t code, va_list va) override {
        if (code != processor_t::ev_get_bg_color || !g_ctx) return 0;
        bgcolor_t *color = va_arg(va, bgcolor_t *);
        ea_t       ea    = va_arg(va, ea_t);

        auto *entry = g_ctx->layout.entry_find_if([ea](PatchLayoutEntry& e) {
            return ea >= e.ea() && ea < e.end_ea();
        });
        if (entry) {
            *color = color_for_entry(*entry, ea);
            return 1;
        }

        for (const auto& r : g_ctx->pinfo.getRegionsSet().regions())
            if (r.contains(ea)) { *color = Color::PATCH_REGION; return 1; }

        return 0;
    }
};

static RangeColorHooks s_hooks;

void init_color_hook() { hook_event_listener(HT_IDP, &s_hooks, &PLUGIN); }
void term_color_hook() { unhook_event_listener(HT_IDP, &s_hooks); }
