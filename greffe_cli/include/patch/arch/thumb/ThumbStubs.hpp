#pragma once
#include "../IArchStubs.hpp"

class ThumbStubs final : public IArchStubs {
    public:
        void                 save_ctx(GumThumbWriter *w)             override;
        void                 restore_ctx(GumThumbWriter *w)          override;
        std::vector<uint8_t> branch(uint64_t from, uint64_t to)      override;
        std::vector<uint8_t> call  (uint64_t from, uint64_t to)      override;
        std::vector<uint8_t> build_shared_stub(uint64_t at)          override;

        std::vector<uint8_t> trampoline_init(uint64_t at,
                                            uint64_t shstub_addr,
                                            uint8_t  **ptr_array)   override;

        std::vector<uint8_t> relocate(const ContextEntry& instr,
                                    uint64_t            dest_addr) override;
        // assumes the handler distance is < 16Mb
        std::string_view     name()            const override;
        uint8_t              instr_alignment() const override { return 4; }
        uint8_t              sizeof_ptr()      const override { return 4; }

};
