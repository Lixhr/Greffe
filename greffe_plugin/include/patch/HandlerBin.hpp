#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include "PatchLayoutEntry.hpp"
#include "funcs.hpp"
#include "name.hpp"

class HandlerBin : public PatchLayoutEntry {
    public:
        HandlerBin() : PatchLayoutEntry(PLEType::entry_handlerbin) {}
        ~HandlerBin() {
            if (!_addr) return;
            for (const auto& [sym, offset] : _offsets) {
                ea_t addr = (offset + _addr) & ~static_cast<ea_t>(1);
                del_func(addr);
                del_global_name(addr);
            }
        }

        HandlerBin(std::vector<uint8_t>                         bytes,
                std::unordered_map<std::string, uint64_t>       offsets) :
                PatchLayoutEntry(PLEType::entry_handlerbin), _offsets(std::move(offsets)) { _bytes = std::move(bytes); }

        size_t size() const { return _bytes.size(); }

        ea_t handler_addr(const std::string& sym) const {
            auto it = _offsets.find(sym);
            if (it == _offsets.end())
                throw std::runtime_error("HandlerBin: symbol not found: " + sym);

            return it->second + _addr;
        }

    private:
        std::unordered_map<std::string, uint64_t> _offsets;
};
