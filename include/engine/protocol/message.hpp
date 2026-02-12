// Copyright (c) 2025 — 2026 Ian Torres <iantorres@outlook.com>.
// All rights reserved.

#ifndef ENGINE_PROTOCOL_MESSAGE_HPP
#define ENGINE_PROTOCOL_MESSAGE_HPP

#include <span>
#include <cstddef>
#include <cstdint>
#include <engine/protocol/detail/message_data.hpp>

namespace engine::protocol {
    /**
     * @brief Represents a protocol message.
     *
     * This class provides a high-level interface to access message data.
     * It delegates the low-level parsing and validation to the internal
     * message_data class.
     */
    class message {
        /**
         * @brief Internal data layer that handles buffer management and validation.
         */
        detail::message_data data_;

    public:
        /**
         * @brief Constructs a message object from a buffer.
         *
         * @param buffer The byte span representing the message buffer.
         * @param precompute Whether to precompute parameter offsets for faster access.
         */
        explicit message(const std::span<const std::byte> buffer, const bool precompute = false)
            : data_(buffer, precompute) {}

        /**
         * @brief Gets the unique identifier of the message.
         *
         * @return A span containing the 16-byte UUID.
         */
        [[nodiscard]] std::span<const std::byte, 16> get_id() const {
            return data_.get_id();
        }

        /**
         * @brief Gets the action code of the message.
         *
         * @return The 32-bit action code.
         */
        [[nodiscard]] std::uint32_t get_action() const {
            return data_.get_action();
        }

        /**
         * @brief Gets a specific parameter by index.
         *
         * @param index The index of the parameter to retrieve.
         * @return A span representing the parameter data.
         */
        [[nodiscard]] std::span<const std::byte> get_parameter(const std::size_t index) const {
            return data_.get_parameter(index);
        }
    };
} // namespace engine::protocol

#endif // ENGINE_PROTOCOL_MESSAGE_HPP
