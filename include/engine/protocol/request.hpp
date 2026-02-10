// Copyright (c) 2025 — 2026 Ian Torres <iantorres@outlook.com>.
// All rights reserved.

#ifndef ENGINE_PROTOCOL_REQUEST_HPP
#define ENGINE_PROTOCOL_REQUEST_HPP

#include <span>
#include <cstddef>
#include <cstdint>
#include <array>

namespace engine::protocol {
    class request {
        const std::byte *buffer_;
        std::size_t buffer_size_;

        static constexpr std::size_t offset_uuid_ = 0;
        static constexpr std::size_t offset_action_ = 16;
        static constexpr std::size_t offset_param_count_ = 20;
        static constexpr std::size_t offset_params_ = 24;
        static constexpr std::size_t max_params_ = 8;

        std::uint16_t cached_param_count_ = 0;
        const std::uint32_t *cached_param_sizes_ = nullptr;
        std::array<std::size_t, max_params_> param_offsets_{};

    public:
        explicit request(const std::span<const std::byte> buffer, const bool precompute = false)
            : buffer_(buffer.data()), buffer_size_(buffer.size()) {
            cached_param_count_ = *reinterpret_cast<const std::uint16_t *>(buffer_ + offset_param_count_);
            cached_param_sizes_ = reinterpret_cast<const std::uint32_t *>(buffer_ + offset_params_);

            if (precompute) {
                std::size_t _offset = offset_params_ + cached_param_count_ * sizeof(std::uint32_t);
                for (std::uint16_t _i = 0; _i < cached_param_count_; ++_i) {
                    param_offsets_[_i] = _offset;
                    _offset += cached_param_sizes_[_i];
                }
            }
        }

        [[nodiscard]] std::span<const std::byte, 16> get_id() const noexcept {
            return std::span<const std::byte, 16>(buffer_ + offset_uuid_, 16);
        }

        [[nodiscard]] std::uint32_t get_action() const noexcept {
            return *reinterpret_cast<const std::uint32_t *>(buffer_ + offset_action_);
        }

        [[nodiscard]] std::uint16_t get_param_count() const noexcept {
            return cached_param_count_;
        }

        [[nodiscard]] const std::uint32_t *get_param_sizes() const noexcept {
            return cached_param_sizes_;
        }

        [[nodiscard]] std::span<const std::byte> get_param(const std::size_t index) const noexcept {
            return {buffer_ + param_offsets_[index], cached_param_sizes_[index]};
        }

        [[nodiscard]] const std::byte *get_params_data() const noexcept {
            return cached_param_count_ == 0 ? nullptr : buffer_ + param_offsets_[0];
        }
    };
} // namespace engine::protocol

#endif // ENGINE_PROTOCOL_REQUEST_HPP
