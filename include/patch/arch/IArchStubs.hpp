#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include "ida.hpp"

// Frida writers always emit literal pool words in LE regardless of target endianness.
// Call after resize(written) to byte-swap every 4-byte pool word in-place.
inline void fix_be_pool(std::vector<uint8_t>& buf, size_t pool_start) {
    for (size_t i = pool_start; i + 4 <= buf.size(); i += 4) {
        std::swap(buf[i],     buf[i + 3]);
        std::swap(buf[i + 1], buf[i + 2]);
    }
}

struct ContextEntry;

class IArchStubs {
    public:
        virtual ~IArchStubs() = default;

        virtual std::vector<uint8_t> branch(ea_t from, ea_t to) = 0;
        virtual std::vector<uint8_t> call  (ea_t from, ea_t to) = 0;
        virtual std::vector<uint8_t> build_shared_stub(ea_t at) = 0;
        virtual std::vector<uint8_t> trampoline_init(ea_t    at,
                                                     ea_t    shstub_addr,
                                                     uint8_t **ptr_array) = 0;
        virtual std::vector<uint8_t> relocate_and_branch_back(
                                            const std::vector<ContextEntry>& instrs,
                                            ea_t                             dest_addr,
                                            ea_t                             branch_to) = 0;

        // Override both together when an arch has instructions that form semantic groups (IT blocks, ..).
        virtual bool                 at_reloc_boundary(const std::vector<ContextEntry>&) const { return true; }
        virtual std::vector<uint8_t> nop_bytes()       const { return {}; }

        virtual std::string      name()            const = 0;
        virtual uint8_t          instr_alignment() const = 0;

        uint8_t sizeof_ptr() const { return static_cast<uint8_t>(_bits / 8); }

        void write_ptr(uint8_t* dst, ea_t addr) const {
            const uint8_t n = sizeof_ptr();
            if (_endianness == "le") {
                for (uint8_t i = 0; i < n; ++i)
                    dst[i] = static_cast<uint8_t>(addr >> (8 * i));
            } else {
                for (uint8_t i = 0; i < n; ++i)
                    dst[i] = static_cast<uint8_t>(addr >> (8 * (n - 1 - i)));
            }
        }

        uint64_t align_offset(uint64_t offset) const {
            const uint8_t a = instr_alignment();
            return (offset + a - 1) & ~static_cast<uint64_t>(a - 1);
        }

    protected:
        bool is_big_endian() const { return _endianness != "le"; }

        IArchStubs(int bits, std::string endianness)
            : _bits(bits), _endianness(std::move(endianness)) {}

    private:
        int         _bits;
        std::string _endianness;
};
