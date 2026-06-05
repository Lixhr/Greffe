#pragma once

#include <cstring>
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

        HandlerBin(std::vector<uint8_t>                   bytes,
                std::unordered_map<std::string, uint64_t> offsets,
                std::vector<uint32_t>                     rel_offsets = {}) :
                PatchLayoutEntry(PLEType::entry_handlerbin),
                _offsets(std::move(offsets)),
                _rel_offsets(std::move(rel_offsets)) { _bytes = std::move(bytes); }

        size_t size() const { return _bytes.size(); }

        void rebase(ea_t base) {
            for (uint32_t off : _rel_offsets) {
                if (off + 4 > _bytes.size()) continue;
                uint32_t word;
                memcpy(&word, _bytes.data() + off, 4);
                word += static_cast<uint32_t>(base);
                memcpy(_bytes.data() + off, &word, 4);
            }
        }

        ea_t handler_addr(const std::string& sym) const {
            auto it = _offsets.find(sym);
            if (it == _offsets.end())
                throw std::runtime_error("HandlerBin: symbol not found: " + sym);

            return it->second + _addr;
        }

    private:
        std::unordered_map<std::string, uint64_t> _offsets;
        std::vector<uint32_t>                     _rel_offsets;
};
