// Copyright (c) 2025 — 2026 Ian Torres <iantorres@outlook.com>.
// All rights reserved.

#include <benchmark/benchmark.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <engine/protocol.hpp>

using engine::protocol::request;

static std::array<std::byte, 64> make_buffer() {
    std::array<std::byte, 64> _buffer{};

    constexpr std::uint32_t _action = 0xAABBCCDD;
    constexpr std::uint16_t _count  = 3;
    constexpr std::uint32_t _sizes[3] = {4, 4, 4};

    std::memcpy(_buffer.data() + 16, &_action, sizeof(_action));
    std::memcpy(_buffer.data() + 20, &_count, sizeof(_count));
    std::memcpy(_buffer.data() + 24, _sizes, sizeof(_sizes));

    _buffer[36] = std::byte{0x01};
    _buffer[37] = std::byte{0x02};
    _buffer[38] = std::byte{0x03};
    _buffer[39] = std::byte{0x04};

    _buffer[40] = std::byte{0x05};
    _buffer[41] = std::byte{0x06};
    _buffer[42] = std::byte{0x07};
    _buffer[43] = std::byte{0x08};

    _buffer[44] = std::byte{0x09};
    _buffer[45] = std::byte{0x0A};
    _buffer[46] = std::byte{0x0B};
    _buffer[47] = std::byte{0x0C};

    return _buffer;
}

static void BM_request_get_id(benchmark::State& state) {
    auto _buffer = make_buffer();
    const request _req(_buffer);

    for (auto _ : state) {
        benchmark::DoNotOptimize(_req.get_id());
    }
}
BENCHMARK(BM_request_get_id);

static void BM_request_get_action(benchmark::State& state) {
    auto _buffer = make_buffer();
    const request _req(_buffer);

    for (auto _ : state) {
        benchmark::DoNotOptimize(_req.get_action());
    }
}
BENCHMARK(BM_request_get_action);

static void BM_request_get_param_count(benchmark::State& state) {
    auto _buffer = make_buffer();
    const request _req(_buffer);

    for (auto _ : state) {
        benchmark::DoNotOptimize(_req.get_param_count());
    }
}
BENCHMARK(BM_request_get_param_count);

static void BM_request_get_param_sizes_ptr(benchmark::State& state) {
    auto _buffer = make_buffer();
    const request _req(_buffer);

    for (auto _ : state) {
        benchmark::DoNotOptimize(_req.get_param_sizes());
    }
}
BENCHMARK(BM_request_get_param_sizes_ptr);

static void BM_request_get_param_0(benchmark::State& state) {
    auto _buffer = make_buffer();
    const request _req(_buffer);

    for (auto _ : state) {
        benchmark::DoNotOptimize(_req.get_param(0));
    }
}
BENCHMARK(BM_request_get_param_0);

static void BM_request_get_param_2(benchmark::State& state) {
    auto _buffer = make_buffer();
    const request _req(_buffer);

    for (auto _ : state) {
        benchmark::DoNotOptimize(_req.get_param(2));
    }
}
BENCHMARK(BM_request_get_param_2);

static void BM_request_construct(benchmark::State& state) {
    auto _buffer = make_buffer();

    for (auto _ : state) {
        benchmark::DoNotOptimize(request(_buffer));
    }
}
BENCHMARK(BM_request_construct);

static void BM_request_construct_precomputed(benchmark::State& state) {
    auto _buffer = make_buffer();

    for (auto _ : state) {
        benchmark::DoNotOptimize(request(_buffer, true));
    }
}
BENCHMARK(BM_request_construct_precomputed);
