// Copyright (c) 2025 — 2026 Ian Torres <iantorres@outlook.com>.
// All rights reserved.

#ifndef ENGINE_PROTOCOL_MESSAGE_HPP
#define ENGINE_PROTOCOL_MESSAGE_HPP

#include <span>
#include <cstddef>
#include <cstdint>
#include <array>
#include <stdexcept>
#include <boost/contract.hpp>

namespace engine::protocol {

    /**
     * @brief Exception thrown when a buffer is not properly aligned.
     */
    class alignment_error : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    /**
     * @brief Represents a protocol message.
     *
     * This class provides methods to access different parts of a message buffer.
     * The buffer must be 4-byte aligned and have a minimum size of 24 bytes.
     */
    class message {
        /**
         * @brief Pointer to the message buffer.
         */
        const std::byte *buffer_;

        /**
         * @brief Size of the message buffer.
         */
        std::size_t buffer_size_;

        static constexpr std::size_t offset_uuid_ = 0;
        static constexpr std::size_t offset_action_ = 16;
        static constexpr std::size_t offset_param_count_ = 20;
        static constexpr std::size_t offset_params_ = 24;
        static constexpr std::size_t max_params_ = 8;

        /**
         * @brief Cached number of parameters in the message.
         */
        std::uint16_t cached_param_count_ = 0;

        /**
         * @brief Pointer to the parameter sizes array within the buffer.
         */
        const std::uint32_t *cached_param_sizes_ = nullptr;

        /**
         * @brief Offsets of each parameter within the buffer.
         */
        std::array<std::size_t, max_params_> param_offsets_{};

    public:
        /**
         * @brief Constructs a message object from a buffer.
         *
         * @param buffer The byte span representing the message buffer.
         * @param precompute Whether to precompute parameter offsets.
         * @throws alignment_error If the buffer is not 4-byte aligned.
         *
         * @pre buffer.size() >= 24
         * @pre reinterpret_cast<std::uintptr_t>(buffer.data()) % 4 == 0
         * @pre buffer.size() >= 24 + (param_count * 4)
         * @pre param_count <= max_params_
         */
        explicit message(const std::span<const std::byte> buffer, const bool precompute = false)
            : buffer_(buffer.data()), buffer_size_(buffer.size()) {
            boost::contract::check c = boost::contract::constructor(this)
                .precondition([&] {
                    if (reinterpret_cast<std::uintptr_t>(buffer.data()) % 4 != 0) {
                        throw alignment_error("Buffer must be 4-byte aligned");
                    }
                    BOOST_CONTRACT_ASSERT(buffer.size() >= offset_params_);
                    const std::uint16_t _count = *reinterpret_cast<const std::uint16_t *>(buffer.data() + offset_param_count_);
                    BOOST_CONTRACT_ASSERT(_count <= max_params_);
                    BOOST_CONTRACT_ASSERT(buffer.size() >= offset_params_ + _count * sizeof(std::uint32_t));
                });

            cached_param_count_ = *reinterpret_cast<const std::uint16_t *>(buffer_ + offset_param_count_);
            cached_param_sizes_ = reinterpret_cast<const std::uint32_t *>(buffer_ + offset_params_);

            if (precompute) {
                std::size_t _offset = offset_params_ + cached_param_count_ * sizeof(std::uint32_t);
                for (std::uint16_t _i = 0; _i < cached_param_count_; ++_i) {
                    BOOST_CONTRACT_ASSERT(_offset + cached_param_sizes_[_i] <= buffer_size_);
                    param_offsets_[_i] = _offset;
                    _offset += cached_param_sizes_[_i];
                }
            }
        }

        /**
         * @brief Gets the unique identifier of the message.
         *
         * @return A span containing the 16-byte UUID.
         */
        [[nodiscard]] std::span<const std::byte, 16> get_id() const {
            boost::contract::check c = boost::contract::public_function(this)
                .precondition([&] {
                    BOOST_CONTRACT_ASSERT(buffer_size_ >= offset_uuid_ + 16);
                });
            return std::span<const std::byte, 16>(buffer_ + offset_uuid_, 16);
        }

        /**
         * @brief Gets the action code of the message.
         *
         * @return The 32-bit action code.
         */
        [[nodiscard]] std::uint32_t get_action() const {
            boost::contract::check c = boost::contract::public_function(this)
                .precondition([&] {
                    BOOST_CONTRACT_ASSERT(buffer_size_ >= offset_action_ + sizeof(std::uint32_t));
                });
            return *reinterpret_cast<const std::uint32_t *>(buffer_ + offset_action_);
        }

        /**
         * @brief Gets the number of parameters in the message.
         *
         * @return The parameter count.
         */
        [[nodiscard]] std::uint16_t get_param_count() const noexcept {
            return cached_param_count_;
        }

        /**
         * @brief Gets the pointer to the array of parameter sizes.
         *
         * @return A pointer to the uint32_t array of sizes.
         */
        [[nodiscard]] const std::uint32_t *get_param_sizes() const noexcept {
            return cached_param_sizes_;
        }

        /**
         * @brief Gets a specific parameter by index.
         *
         * @param index The index of the parameter to retrieve.
         * @return A span representing the parameter data.
         *
         * @pre index < get_param_count()
         * @pre param_offsets_[index] != 0 (implies precompute was true or offsets were set)
         */
        [[nodiscard]] std::span<const std::byte> get_param(const std::size_t index) const {
            boost::contract::check c = boost::contract::public_function(this)
                .precondition([&] {
                    BOOST_CONTRACT_ASSERT(index < cached_param_count_);
                    BOOST_CONTRACT_ASSERT(param_offsets_[index] != 0);
                    BOOST_CONTRACT_ASSERT(buffer_size_ >= param_offsets_[index] + cached_param_sizes_[index]);
                });
            return {buffer_ + param_offsets_[index], cached_param_sizes_[index]};
        }

        /**
         * @brief Gets a pointer to the start of the parameters data.
         *
         * @return A pointer to the start of the parameters data, or nullptr if there are no parameters.
         */
        [[nodiscard]] const std::byte *get_params_data() const {
            boost::contract::check c = boost::contract::public_function(this)
                .precondition([&] {
                    if (cached_param_count_ > 0) {
                        BOOST_CONTRACT_ASSERT(param_offsets_[0] != 0);
                    }
                });
            return cached_param_count_ == 0 ? nullptr : buffer_ + param_offsets_[0];
        }
    };
} // namespace engine::protocol

#endif // ENGINE_PROTOCOL_MESSAGE_HPP
