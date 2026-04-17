#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include "PatchLayoutEntry.hpp"

class HandlerBin : public PatchLayoutEntry {
    public:
        HandlerBin() : PatchLayoutEntry(PLEType::entry_handlerbin) {}
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
