#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class HandlerBin {
public:
    HandlerBin() = default;
    HandlerBin(std::vector<uint8_t>                         bytes,
               std::unordered_map<std::string, uint64_t>    offsets) :
               _bytes(std::move(bytes))
             , _offsets(std::move(offsets)) {}

    size_t                      size()  const { return _bytes.size(); }
    const std::vector<uint8_t>& bytes() const { return _bytes; }

    uint64_t handler_addr(const std::string& sym, uint64_t base) const {
        auto it = _offsets.find(sym);
        if (it == _offsets.end())
            throw std::runtime_error("HandlerBin: symbol not found: " + sym);

        return it->second + base;
    }

private:
    std::vector<uint8_t>                      _bytes;
    std::unordered_map<std::string, uint64_t> _offsets;
};
