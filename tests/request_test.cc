// Copyright (c) 2025 — 2026 Ian Torres <iantorres@outlook.com>.
// All rights reserved.

#include <gtest/gtest.h>
#include <array>
#include <cstddef>
#include <cstring>
#include <engine/protocol.hpp>

using engine::protocol::request;

TEST(request, can_be_identified) {
    std::array<std::byte, 24> _buffer = {
        std::byte{0xb9}, std::byte{0xf3}, std::byte{0x7d}, std::byte{0xa5},
        std::byte{0x26}, std::byte{0xd1}, std::byte{0x4d}, std::byte{0x87},
        std::byte{0x9e}, std::byte{0xd3}, std::byte{0xb8}, std::byte{0x0b},
        std::byte{0x88}, std::byte{0x65}, std::byte{0xb3}, std::byte{0x4b},
        std::byte{0x01}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, // action
        std::byte{0x00}, std::byte{0x00},                                     // param_count
        std::byte{0x00}, std::byte{0x00}                                      // padding
    };

    const request _req(_buffer);
    const auto _id = _req.get_id();

    ASSERT_EQ(_id.size(), 16u);
    for (std::size_t _i = 0; _i < 16; ++_i)
        ASSERT_EQ(_id[_i], _buffer[_i]);
}

TEST(request, reads_action) {
    std::array<std::byte, 24> _buffer{};
    constexpr std::uint32_t _action = 0xAABBCCDD;
    std::memcpy(_buffer.data() + 16, &_action, sizeof(_action));

    const request _req(_buffer);
    ASSERT_EQ(_req.get_action(), _action);
}

TEST(request, reads_param_count) {
    std::array<std::byte, 24> _buffer{};
    constexpr std::uint16_t _count = 3;
    std::memcpy(_buffer.data() + 20, &_count, sizeof(_count));

    const request _req(_buffer);
    ASSERT_EQ(_req.get_param_count(), _count);
}

TEST(request, reads_params_and_data) {
    std::array<std::byte, 24 + 12 + 6> _buffer{};
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

    const request _req(_buffer, true);

    const auto _p0 = _req.get_param(0);
    const auto _p1 = _req.get_param(1);
    const auto _p2 = _req.get_param(2);

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
