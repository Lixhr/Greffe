#pragma once
#include "IArchStubs.hpp"

class ThumbStubs final : public IArchStubs {
public:
    std::vector<uint8_t> save_ctx   (uint64_t at) override;
    std::vector<uint8_t> restore_ctx(uint64_t at) override;
    std::vector<uint8_t> branch     (uint64_t from, uint64_t to) override;
    std::vector<uint8_t> call       (uint64_t from, uint64_t to) override;
    std::string_view     name()      const override;
};
