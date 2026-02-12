// Copyright (c) 2025 — 2026 Ian Torres <iantorres@outlook.com>.
// All rights reserved.

#include <gtest/gtest.h>
#include <array>
#include <cstddef>
#include <cstring>
#include <engine/protocol.hpp>
#include <engine/protocol/detail/message_data.hpp>
#include <boost/contract.hpp>

using engine::protocol::message;
using engine::protocol::detail::message_data;

void set_error_handler() {
#ifndef BOOST_CONTRACT_NO_CHECKS
    set_precondition_failure(
    set_postcondition_failure(
        set_invariant_failure(
            boost::contract::set_old_failure(
                [](const boost::contract::from where) {
                    if (where == boost::contract::from_destructor) {
                        // Never throw from destructors.
                        std::clog << "ignored destructor contract failure" << std::endl;
                    } else {
                        throw; // Re-throw the assertion_failure exception.
                    }
                }
            ))));

    boost::contract::set_except_failure(
        [](boost::contract::from) {
            throw;
        }
    );
    boost::contract::set_check_failure(
        [] {
            throw;
        }
    );
#endif
}

TEST(message, can_be_identified) {
    alignas(4) std::array<std::byte, 24> _buffer = {
        std::byte{0xb9}, std::byte{0xf3}, std::byte{0x7d}, std::byte{0xa5},
        std::byte{0x26}, std::byte{0xd1}, std::byte{0x4d}, std::byte{0x87},
        std::byte{0x9e}, std::byte{0xd3}, std::byte{0xb8}, std::byte{0x0b},
        std::byte{0x88}, std::byte{0x65}, std::byte{0xb3}, std::byte{0x4b},
        std::byte{0x01}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, // action
        std::byte{0x00}, std::byte{0x00}, // param_count
        std::byte{0x00}, std::byte{0x00} // padding
    };

    const message _msg(_buffer);
    const auto _id = _msg.get_id();

    ASSERT_EQ(_id.size(), 16u);
    for (std::size_t _i = 0; _i < 16; ++_i)
        ASSERT_EQ(_id[_i], _buffer[_i]);
}

TEST(message, reads_action) {
    alignas(4) std::array<std::byte, 24> _buffer{};
    constexpr std::uint32_t _action = 0xAABBCCDD;
    std::memcpy(_buffer.data() + 16, &_action, sizeof(_action));

    const message _msg(_buffer);
    ASSERT_EQ(_msg.get_action(), _action);
}

TEST(message_data, reads_param_count) {
    alignas(4) std::array<std::byte, 36> _buffer{};
    constexpr std::uint16_t _count = 3;
    std::memcpy(_buffer.data() + 20, &_count, sizeof(_count));

    const message_data _data(_buffer, true);
    ASSERT_EQ(_data.get_param_count(), _count);
}

TEST(message_data, reads_param_sizes) {
    alignas(4) std::array<std::byte, 64> _buffer{};
    constexpr std::uint16_t _count = 3;
    constexpr std::uint32_t _sizes[3] = {10, 20, 30};
    std::memcpy(_buffer.data() + 20, &_count, sizeof(_count));
    std::memcpy(_buffer.data() + 24, _sizes, sizeof(_sizes));

    const message_data _data(_buffer);
    const auto *_msg_sizes = _data.get_param_sizes();
    ASSERT_NE(_msg_sizes, nullptr);
    ASSERT_EQ(_msg_sizes[0], 10u);
    ASSERT_EQ(_msg_sizes[1], 20u);
    ASSERT_EQ(_msg_sizes[2], 30u);
}

TEST(message, reads_params_and_data) {
    alignas(4) std::array<std::byte, 24 + 12 + 6> _buffer{};
    constexpr std::uint16_t _count = 3;
    constexpr std::uint32_t _sizes[3] = {1, 2, 3};

    std::memcpy(_buffer.data() + 20, &_count, sizeof(_count));
    std::memcpy(_buffer.data() + 24, _sizes, sizeof(_sizes));

    _buffer[36] = std::byte{0xAA};
    _buffer[37] = std::byte{0xBB};
    _buffer[38] = std::byte{0xCC};
    _buffer[39] = std::byte{0xDD};
    _buffer[40] = std::byte{0xEE};
    _buffer[41] = std::byte{0xFF};

    const message _msg(_buffer, true);

    const auto _p0 = _msg.get_parameter(0);
    const auto _p1 = _msg.get_parameter(1);
    const auto _p2 = _msg.get_parameter(2);

    ASSERT_EQ(_p0.size(), 1u);
    ASSERT_EQ(_p1.size(), 2u);
    ASSERT_EQ(_p2.size(), 3u);

    ASSERT_EQ(_p0[0], std::byte{0xAA});
    ASSERT_EQ(_p1[0], std::byte{0xBB});
    ASSERT_EQ(_p1[1], std::byte{0xCC});
    ASSERT_EQ(_p2[0], std::byte{0xDD});
    ASSERT_EQ(_p2[1], std::byte{0xEE});
    ASSERT_EQ(_p2[2], std::byte{0xFF});
}

TEST(message_data, get_params_data_returns_correct_pointer) {
    alignas(4) std::array<std::byte, 64> _buffer{};
    constexpr std::uint16_t _count = 1;
    constexpr std::uint32_t _size = 4;
    std::memcpy(_buffer.data() + 20, &_count, sizeof(_count));
    std::memcpy(_buffer.data() + 24, &_size, sizeof(_size));

    const message_data _data(_buffer, true);
    ASSERT_EQ(_data.get_params_data(), _buffer.data() + 24 + 4);
}

TEST(message_data, get_params_data_returns_nullptr_for_zero_params) {
    alignas(4) std::array<std::byte, 24> _buffer{};
    const message_data _data(_buffer, true);
    ASSERT_EQ(_data.get_params_data(), nullptr);
}

#ifndef BOOST_CONTRACT_NO_CHECKS
TEST(message, throws_on_unaligned_buffer) {
    set_error_handler();

    std::array<std::byte, 64> _buffer_raw{};
    // Find an unaligned pointer
    std::byte *_ptr = _buffer_raw.data();
    if (reinterpret_cast<std::uintptr_t>(_ptr) % 4 == 0) {
        _ptr += 1;
    }
    std::span<const std::byte> _unaligned_span(_ptr, 24);

    ASSERT_ANY_THROW(message _msg(_unaligned_span));
}

TEST(message, throws_on_small_buffer) {
    set_error_handler();
    alignas(4) std::array<std::byte, 10> _small_buffer{};
    ASSERT_ANY_THROW(message _msg(_small_buffer));
}

TEST(message, throws_on_out_of_bounds_param) {
    set_error_handler();
    alignas(4) std::array<std::byte, 64> _buffer{};
    constexpr std::uint16_t _count = 1;
    std::memcpy(_buffer.data() + 20, &_count, sizeof(_count));

    const message _msg(_buffer, true);
    ASSERT_ANY_THROW(_msg.get_parameter(1));
}

TEST(message, throws_on_too_many_params) {
    set_error_handler();
    alignas(4) std::array<std::byte, 128> _buffer{};
    constexpr std::uint16_t _count = 9; // Max is 8
    std::memcpy(_buffer.data() + 20, &_count, sizeof(_count));

    ASSERT_ANY_THROW(message _msg(_buffer));
}

TEST(message, throws_on_missing_precompute) {
    set_error_handler();
    alignas(4) std::array<std::byte, 64> _buffer{};
    constexpr std::uint16_t _count = 1;
    std::memcpy(_buffer.data() + 20, &_count, sizeof(_count));

    const message _msg(_buffer, false); // No precompute
    ASSERT_ANY_THROW(_msg.get_parameter(0));
}
#endif
