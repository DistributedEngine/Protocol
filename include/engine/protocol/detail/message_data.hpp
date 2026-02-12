// Copyright (c) 2025 — 2026 Ian Torres <iantorres@outlook.com>.
// All rights reserved.

#ifndef ENGINE_PROTOCOL_DETAIL_MESSAGE_DATA_HPP
#define ENGINE_PROTOCOL_DETAIL_MESSAGE_DATA_HPP

#include <span>
#include <cstddef>
#include <cstdint>
#include <array>
#include <boost/contract.hpp>

namespace engine::protocol::detail {

    /**
     * @brief Internal data layer for protocol messages.
     *
     * This class handles the low-level parsing, offset calculation, and contract
     * enforcement for message buffers. By separating this from the main message
     * class, we keep the public API clean while maintaining robust internal logic.
     */
    class message_data
#ifndef BOOST_CONTRACT_NO_CHECKS
        : private boost::contract::constructor_precondition<message_data>
#endif
    {
    public:
        /**
         * Protocol Layout Constants:
         * - UUID: 16 bytes starting at offset 0.
         * - Action: 4 bytes starting at offset 16.
         * - Param Count: 2 bytes starting at offset 20.
         * - Padding: 2 bytes starting at offset 22 (for 4-byte alignment).
         * - Param Sizes: Array of 4-byte integers starting at offset 24.
         */
        static constexpr std::size_t offset_uuid = 0;
        static constexpr std::size_t offset_action = 16;
        static constexpr std::size_t offset_param_count = 20;
        static constexpr std::size_t offset_params = 24;
        static constexpr std::size_t max_params = 8;

        /**
         * @brief Pointer to the message buffer.
         */
        const std::byte *buffer;

        /**
         * @brief Size of the message buffer.
         */
        std::size_t buffer_size;

        /**
         * @brief Cached number of parameters in the message.
         */
        std::uint16_t cached_param_count = 0;

        /**
         * @brief Pointer to the parameter sizes array within the buffer.
         */
        const std::uint32_t *cached_param_sizes = nullptr;

        /**
         * @brief Offsets of each parameter within the buffer.
         *
         * Precomputed to avoid repetitive recalculation of offsets when accessing parameters.
         */
        std::array<std::size_t, max_params> param_offsets{};

        /**
         * @brief Constructs message_data and performs protocol validation.
         *
         * @param buffer_span The byte span representing the message buffer.
         * @param precompute Whether to precompute parameter offsets.
         */
        explicit message_data(const std::span<const std::byte> buffer_span, const bool precompute = false)
            :
#ifndef BOOST_CONTRACT_NO_CHECKS
            boost::contract::constructor_precondition<message_data>([&] {
                // The protocol requires 4-byte alignment for efficient memory access and to meet
                // hardware requirements for multi-byte types.
                BOOST_CONTRACT_ASSERT(reinterpret_cast<std::uintptr_t>(buffer_span.data()) % 4 == 0);

                // The buffer must be large enough to contain at least the fixed-size header
                // up to the start of the parameters section (24 bytes).
                BOOST_CONTRACT_ASSERT(buffer_span.size() >= offset_params);

                const std::uint16_t count = *reinterpret_cast<const std::uint16_t *>(
                    buffer_span.data() + offset_param_count);

                // The buffer must be large enough to hold the fixed header plus the array of
                // parameter sizes (4 bytes each).
                BOOST_CONTRACT_ASSERT(buffer_span.size() >= offset_params + count * sizeof(std::uint32_t));

                // The number of parameters must not exceed the maximum allowed by the protocol
                // to prevent buffer overflow and maintain predictable memory usage.
                BOOST_CONTRACT_ASSERT(count <= max_params);
            }),
#endif
            buffer(buffer_span.data()),
            buffer_size(buffer_span.size()) {

            // Cache parameter count and the pointer to sizes for fast access.
            cached_param_count = *reinterpret_cast<const std::uint16_t *>(buffer + offset_param_count);
            cached_param_sizes = reinterpret_cast<const std::uint32_t *>(buffer + offset_params);

            if (precompute) {
                // Parameter data starts immediately after the parameter sizes array.
                // We calculate each parameter's start position by accumulating the sizes of preceding parameters.
                std::size_t current_offset = offset_params + cached_param_count * sizeof(std::uint32_t);

                for (std::uint16_t i = 0; i < cached_param_count; ++i) {
#ifndef BOOST_CONTRACT_NO_CHECKS
                    // Validation: Ensure that the parameter's data actually fits within the provided buffer.
                    BOOST_CONTRACT_ASSERT(current_offset + cached_param_sizes[i] <= buffer_size);
#endif
                    param_offsets[i] = current_offset;
                    current_offset += cached_param_sizes[i];
                }
            }
        }

        /**
         * @brief Retrieves the message UUID.
         */
        [[nodiscard]] std::span<const std::byte, 16> get_id() const {
#ifndef BOOST_CONTRACT_NO_CHECKS
            boost::contract::check c = boost::contract::public_function(this)
                    .precondition([&] {
                        // Ensure the buffer is large enough to contain the 16-byte UUID.
                        BOOST_CONTRACT_ASSERT(buffer_size >= offset_uuid + 16);
                    });
#endif
            return std::span<const std::byte, 16>(buffer + offset_uuid, 16);
        }

        /**
         * @brief Retrieves the message action code.
         */
        [[nodiscard]] std::uint32_t get_action() const {
#ifndef BOOST_CONTRACT_NO_CHECKS
            boost::contract::check c = boost::contract::public_function(this)
                    .precondition([&] {
                        // Ensure the buffer is large enough to contain the 4-byte action code.
                        BOOST_CONTRACT_ASSERT(buffer_size >= offset_action + sizeof(std::uint32_t));
                    });
#endif
            return *reinterpret_cast<const std::uint32_t *>(buffer + offset_action);
        }

        /**
         * @brief Retrieves a parameter by its index.
         */
        [[nodiscard]] std::span<const std::byte> get_parameter(const std::size_t index) const {
#ifndef BOOST_CONTRACT_NO_CHECKS
            boost::contract::check c = boost::contract::public_function(this)
                    .precondition([&] {
                        // Parameter index must be within the range of parameters present in the message.
                        BOOST_CONTRACT_ASSERT(index < cached_param_count);
                        // The offset for this parameter must have been precomputed.
                        // A non-zero offset indicates it was successfully calculated during construction.
                        BOOST_CONTRACT_ASSERT(param_offsets[index] != 0);
                        // Final safety check: ensure the parameter data is still within buffer bounds.
                        BOOST_CONTRACT_ASSERT(buffer_size >= param_offsets[index] + cached_param_sizes[index]);
                    });
#endif
            return {buffer + param_offsets[index], cached_param_sizes[index]};
        }

        /**
         * @brief Internal access to parameter count.
         */
        [[nodiscard]] std::uint16_t get_param_count() const noexcept {
            return cached_param_count;
        }

        /**
         * @brief Internal access to parameter sizes array.
         */
        [[nodiscard]] const std::uint32_t *get_param_sizes() const noexcept {
            return cached_param_sizes;
        }

        /**
         * @brief Internal access to the start of parameter data.
         */
        [[nodiscard]] const std::byte *get_params_data() const {
#ifndef BOOST_CONTRACT_NO_CHECKS
            boost::contract::check c = boost::contract::public_function(this)
                    .precondition([&] {
                        if (cached_param_count > 0) {
                            BOOST_CONTRACT_ASSERT(param_offsets[0] != 0);
                        }
                    });
#endif
            return cached_param_count == 0 ? nullptr : buffer + param_offsets[0];
        }
    };

} // namespace engine::protocol::detail

#endif // ENGINE_PROTOCOL_DETAIL_MESSAGE_DATA_HPP
