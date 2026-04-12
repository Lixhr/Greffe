#pragma once
#include "../IArchStubs.hpp"

class ThumbStubs final : public IArchStubs {
    public:
        void                 save_ctx(GumThumbWriter *w)             override;
        void                 restore_ctx(GumThumbWriter *w)          override;
        std::vector<uint8_t> branch(ea_t from, ea_t to)      override;
        std::vector<uint8_t> call  (ea_t from, ea_t to)      override;
        std::vector<uint8_t> build_shared_stub(ea_t at)          override;

        std::vector<uint8_t> trampoline_init(ea_t at,
                                             ea_t      shstub_addr,
                                             uint8_t  **ptr_array)   override;

        std::vector<uint8_t> relocate_and_branch_back(
                                    const std::vector<ContextEntry>& instrs,
                                    ea_t                             dest_addr,
                                    ea_t                             branch_to) override;
        // assumes the handler distance is < 16Mb
        std::string_view     name()            const override;
        uint8_t              instr_alignment() const override { return 4; }
        uint8_t              sizeof_ptr()      const override { return 4; }
        void                 write_ptr(uint8_t* dst, ea_t addr) const override;

};
