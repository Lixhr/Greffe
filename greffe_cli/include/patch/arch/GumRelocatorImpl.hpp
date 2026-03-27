#pragma once

#include "IRelocator.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

struct cs_insn;

template <typename T>
static size_t gum_writer_written(const typename T::Writer& w) {
    return reinterpret_cast<const uint8_t*>(w.code)
         - reinterpret_cast<const uint8_t*>(w.base);
}

template <typename T>
static size_t read_instructions(typename T::Relocator& rl, size_t n_bytes) {
    size_t n_insns = 0;
    const cs_insn* insn = nullptr;
    while (T::relocator_read_one(&rl, &insn) != 0) {
        ++n_insns;
        size_t consumed = static_cast<size_t>(rl.input_cur - rl.input_start);
        if (consumed >= n_bytes || T::relocator_eoi(&rl))
            break;
    }
    return n_insns;
}

template <typename T>
static void write_instructions(typename T::Relocator& rl) {
    while (T::relocator_write_one(&rl)) {}
}

template <typename T>
static void append_trailer(typename T::Writer& writer, const std::vector<uint8_t>& trailer) {
    if (!trailer.empty())
        T::writer_put_bytes(&writer, trailer.data(), trailer.size());
}


template <typename T>
bool is_branch_impl(const std::vector<uint8_t>& bytes, uint64_t ea) {
    if (bytes.empty()) return false;

    uint8_t buf[8] = {};
    typename T::Writer writer;
    T::writer_init(&writer, buf);

    typename T::Relocator rl;
    T::relocator_init(&rl, bytes.data(), &writer);
    rl.input_pc = static_cast<decltype(rl.input_pc)>(ea);

    const cs_insn* insn = nullptr;
    T::relocator_read_one(&rl, &insn);

    bool result = static_cast<bool>(rl.eob);
    T::relocator_clear(&rl);
    T::writer_clear(&writer);
    return result;
}

template <typename T>
RelocatedCode relocate_impl(const std::vector<uint8_t>& input_bytes,
                            size_t   n_bytes,
                            uint64_t original_ea,
                            uint64_t trampoline_addr,
                            const std::vector<uint8_t>& trailer) {
    if (input_bytes.empty() || n_bytes == 0)
        throw std::runtime_error(std::string(T::relocator_name) + ": empty input");

    std::vector<uint8_t> output(512, 0);

    typename T::Writer writer;
    T::writer_init(&writer, output.data());
    writer.pc = static_cast<decltype(writer.pc)>(trampoline_addr);

    typename T::Relocator rl;
    T::relocator_init(&rl, input_bytes.data(), &writer);
    rl.input_pc = static_cast<decltype(rl.input_pc)>(original_ea);

    const size_t n_insns = read_instructions<T>(rl, n_bytes);
    if (n_insns == 0)
        throw std::runtime_error(std::string(T::relocator_name) + ": failed to read any instruction");

    write_instructions<T>(rl);

    const size_t insns_size = gum_writer_written<T>(writer);

    append_trailer<T>(writer, trailer);
    T::writer_flush(&writer);

    const size_t written = gum_writer_written<T>(writer);

    T::relocator_clear(&rl);
    T::writer_clear(&writer);

    const bool ends_with_branch = static_cast<bool>(rl.eob);
    output.resize(written);
    return { std::move(output), insns_size, n_insns, ends_with_branch };
}