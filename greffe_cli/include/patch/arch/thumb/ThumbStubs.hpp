#pragma once
#include "../IArchStubs.hpp"

class ThumbStubs final : public IArchStubs {
    public:
        std::vector<uint8_t> save_ctx   (uint64_t at) override;
        std::vector<uint8_t> restore_ctx(uint64_t at) override;
        std::vector<uint8_t> branch         (uint64_t from, uint64_t to) override;
        std::vector<uint8_t> call           (uint64_t from, uint64_t to) override;
        std::vector<uint8_t> trampoline_init(uint64_t at, 
                                             uint64_t handler_addr,
                                             uint32_t id);
        // assumes the handler distance is < 16Mb
        size_t               branch_placeholder_size() const override { return 4; }
        std::string_view     name()          const override;
        uint8_t              alignment()     const override { return 4; }
};
