#include <ida.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include "utils.hpp"
#include <vector>
#include <algorithm>

struct ColoredRange {
    ea_t      start;
    ea_t      end;
    bgcolor_t color;
};

static std::vector<ColoredRange> s_ranges;

struct RangeColorHooks : public event_listener_t {
    ssize_t idaapi on_event(ssize_t code, va_list va) override {
        if (code == processor_t::ev_get_bg_color) {
            bgcolor_t *color = va_arg(va, bgcolor_t *);
            ea_t       ea    = va_arg(va, ea_t);
            const ColoredRange *best = nullptr;
            for (const auto& r : s_ranges) {
                if (ea >= r.start && ea < r.end) {
                    if (!best || (r.end - r.start) < (best->end - best->start))
                        best = &r;
                }
            }
            if (best) {
                *color = best->color;
                return 1;
            }
        }
        return 0;
    }
};

extern plugin_t PLUGIN;

static RangeColorHooks s_hooks;
static bool            s_hooked = false;

void set_range_color(ea_t start, ea_t end, bgcolor_t color)
{
    s_ranges.erase(
        std::remove_if(s_ranges.begin(), s_ranges.end(),
            [&](const ColoredRange& r) { return r.start == start && r.end == end; }),
        s_ranges.end());

    if (color != DEFCOLOR)
        s_ranges.push_back({start, end, color});

    if (!s_ranges.empty() && !s_hooked) {
        hook_event_listener(HT_IDP, &s_hooks, &PLUGIN);
        s_hooked = true;
    } else if (s_ranges.empty() && s_hooked) {
        unhook_event_listener(HT_IDP, &s_hooks);
        s_hooked = false;
    }

    request_refresh(IWID_DISASM);
}

void clear_all_range_colors()
{
    s_ranges.clear();
    if (s_hooked) {
        unhook_event_listener(HT_IDP, &s_hooks);
        s_hooked = false;
    }
    request_refresh(IWID_DISASM);
}