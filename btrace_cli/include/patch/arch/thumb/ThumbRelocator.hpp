#pragma once
#include "../IRelocator.hpp"

class ThumbRelocator final : public IRelocator {
public:
    RelocatedCode    relocate(const std::vector<uint8_t>& input_bytes,
                              size_t                       n_bytes,
                              uint64_t                     original_ea,
                              uint64_t                     trampoline_addr,
                              const std::vector<uint8_t>&  trailer) override;
    bool             is_branch(const std::vector<uint8_t>& bytes,
                               uint64_t                     ea) const override;
    std::string_view name() const override;
};
