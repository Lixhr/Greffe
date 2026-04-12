#pragma once

#include <cstdint>
#include <string_view>
#include <vector>
#include <gum/arch-arm/gumthumbwriter.h>

struct ContextEntry;

class IArchStubs {
    public:
        virtual ~IArchStubs() = default;

        virtual void                 save_ctx(GumThumbWriter *w) = 0;
        virtual void                 restore_ctx(GumThumbWriter *w) = 0;
        virtual std::vector<uint8_t> branch(uint64_t from, uint64_t to) = 0;
        virtual std::vector<uint8_t> call  (uint64_t from, uint64_t to) = 0;
        virtual std::vector<uint8_t> build_shared_stub(uint64_t at) = 0;
        virtual std::vector<uint8_t> trampoline_init(uint64_t at,
                                                    uint64_t shstub_addr,
                                                    uint8_t  **ptr_array) = 0;
        virtual std::vector<uint8_t> relocate_and_branch_back(
                                            const std::vector<ContextEntry>& instrs,
                                            uint64_t                         dest_addr,
                                            uint64_t                         branch_to) = 0;

        virtual std::string_view name()            const = 0;
        virtual uint8_t          instr_alignment() const = 0;
        virtual uint8_t          sizeof_ptr()      const = 0;
        virtual void             write_ptr(uint8_t* dst, uint64_t addr) const = 0;

        uint64_t align_offset(uint64_t offset) const {
            const uint8_t a = instr_alignment();
            return (offset + a - 1) & ~static_cast<uint64_t>(a - 1);
        }
};
