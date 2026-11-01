#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

#include "dsp/spsc_ring.hpp"
#include "ui/gameboy_matrix.hpp"

namespace test_embedded {

inline void test_spsc_basic() {
    std::cout << "  [RUN] SPSC Basic FIFO & Capacity Test..." << std::endl;
    tkdsp::SpscRing<int, 8> ring;
    assert(ring.capacity() == 8);
    assert(ring.empty());
    assert(!ring.full());
    assert(ring.size() == 0);

    // Push 8 items (capacity limit)
    for (int i = 0; i < 8; ++i) {
        bool ok = ring.try_push(i * 10);
        assert(ok);
    }
    assert(ring.full());
    assert(ring.size() == 8);

    // Push 9th should fail
    bool overflow = ring.try_push(999);
    assert(!overflow);

    // Pop and verify FIFO order
    for (int i = 0; i < 8; ++i) {
        int val = -1;
        bool ok = ring.try_pop(val);
        assert(ok);
        assert(val == i * 10);
    }
    assert(ring.empty());
    assert(ring.size() == 0);

    // Pop from empty should fail
    int dummy = 0;
    assert(!ring.try_pop(dummy));
    std::cout << "  [PASS] SPSC Basic FIFO & Capacity passed." << std::endl;
}

inline void test_spsc_wraparound_torture() {
    std::cout << "  [RUN] SPSC 32-Bit Integer Overflow Wraparound Torture Test..." << std::endl;
    // Seed index at UINT32_MAX - 1000 to force multiple 32-bit integer wraparounds
    const uint32_t seed = UINT32_MAX - 1000;
    tkdsp::SpscRing<uint64_t, 16> ring(seed);

    constexpr uint64_t kTotalIterations = 10000;
    for (uint64_t i = 0; i < kTotalIterations; ++i) {
        // Push 4 items
        for (int p = 0; p < 4; ++p) {
            uint64_t val = (i * 4) + p;
            bool ok = ring.try_push(val);
            assert(ok);
        }
        // Pop 4 items
        for (int p = 0; p < 4; ++p) {
            uint64_t expected = (i * 4) + p;
            uint64_t val = 0;
            bool ok = ring.try_pop(val);
            assert(ok);
            assert(val == expected);
        }
    }
    assert(ring.empty());
    std::cout << "  [PASS] SPSC 32-Bit Overflow Wraparound passed (10,000 cycles across boundary)." << std::endl;
}

inline void test_spsc_multithreaded_stress() {
    std::cout << "  [RUN] SPSC Multi-Threaded Producer/Consumer Stress Test (100,000 items)..." << std::endl;
    constexpr size_t kItems = 100000;
    tkdsp::SpscRing<size_t, 512> ring;
    std::atomic<bool> producer_done{false};

    // Producer Thread
    std::thread producer([&]() {
        for (size_t i = 0; i < kItems; ++i) {
            while (!ring.try_push(i)) {
                std::this_thread::yield();
            }
        }
        producer_done.store(true, std::memory_order_release);
    });

    // Consumer Thread
    std::vector<size_t> received;
    received.reserve(kItems);

    std::thread consumer([&]() {
        while (!producer_done.load(std::memory_order_acquire) || !ring.empty()) {
            size_t val = 0;
            if (ring.try_pop(val)) {
                received.push_back(val);
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    assert(received.size() == kItems);
    for (size_t i = 0; i < kItems; ++i) {
        assert(received[i] == i);
    }
    std::cout << "  [PASS] SPSC Multi-Threaded Stress passed (100,000 items, zero loss/reorder)." << std::endl;
}

inline void test_gameboy_matrix_bounce_rejection() {
    std::cout << "  [RUN] Game Boy Matrix Contact Bounce Rejection Test..." << std::endl;
    tkui::GameBoyMatrix<5> matrix; // 5ms threshold

    // Simulate 2ms bouncing glitch on Button A (0x10)
    matrix.tick(0x10); // tick 1
    assert(!matrix.is_down(tkui::GameBoyButton::A));
    matrix.tick(0x10); // tick 2
    assert(!matrix.is_down(tkui::GameBoyButton::A));
    matrix.tick(0x00); // glitch back to low (contact bounce)
    assert(matrix.integrator(4) == 1);
    matrix.tick(0x00); // back to 0
    assert(matrix.integrator(4) == 0);
    assert(!matrix.is_down(tkui::GameBoyButton::A));

    // Now continuous 5ms press
    for (int t = 0; t < 4; ++t) {
        matrix.tick(0x10);
        assert(!matrix.is_down(tkui::GameBoyButton::A));
    }
    matrix.tick(0x10); // 5th tick -> confirm!
    assert(matrix.is_down(tkui::GameBoyButton::A));
    assert(matrix.was_pressed(tkui::GameBoyButton::A));

    // Next tick: still down, but was_pressed edge cleared
    matrix.tick(0x10);
    assert(matrix.is_down(tkui::GameBoyButton::A));
    assert(!matrix.was_pressed(tkui::GameBoyButton::A));

    std::cout << "  [PASS] Game Boy Matrix Contact Bounce Rejection passed." << std::endl;
}

inline void test_gameboy_matrix_exhaustive_transitions() {
    std::cout << "  [RUN] Game Boy Matrix Exhaustive 65,536 Transition Test..." << std::endl;
    tkui::GameBoyMatrix<5> matrix;

    for (uint32_t from_state = 0; from_state < 256; ++from_state) {
        // Force matrix to stable from_state
        for (int t = 0; t < 5; ++t) {
            matrix.tick(static_cast<uint8_t>(from_state));
        }
        assert(matrix.state() == static_cast<uint8_t>(from_state));

        for (uint32_t to_state = 0; to_state < 256; ++to_state) {
            // Apply to_state for 5 ticks
            for (int t = 0; t < 4; ++t) {
                matrix.tick(static_cast<uint8_t>(to_state));
            }
            matrix.tick(static_cast<uint8_t>(to_state)); // 5th tick confirms

            assert(matrix.state() == static_cast<uint8_t>(to_state));
            const uint8_t expected_pressed = static_cast<uint8_t>((~from_state) & to_state);
            const uint8_t expected_released = static_cast<uint8_t>(from_state & (~to_state));
            assert(matrix.pressed_edges() == expected_pressed);
            assert(matrix.released_edges() == expected_released);

            // Reset back to from_state for next to_state
            for (int t = 0; t < 5; ++t) {
                matrix.tick(static_cast<uint8_t>(from_state));
            }
        }
    }
    std::cout << "  [PASS] Game Boy Matrix Exhaustive 65,536 Transitions passed (100% verified)." << std::endl;
}

inline void test_gameboy_matrix_exhaustive_release_permutations() {
    std::cout << "  [RUN] Game Boy Matrix Exhaustive 40,320 Release Permutations Test..." << std::endl;
    tkui::GameBoyMatrix<5> matrix;
    std::array<uint8_t, 8> order = {0, 1, 2, 3, 4, 5, 6, 7};
    uint32_t permutation_count = 0;

    do {
        // 1. Establish stable ALL PRESSED state (0xFF)
        for (int t = 0; t < 5; ++t) {
            matrix.tick(0xFF);
        }
        assert(matrix.state() == 0xFF);

        // 2. Release buttons sequentially in this permutation's order
        uint8_t current_pins = 0xFF;
        for (size_t step = 0; step < 8; ++step) {
            uint8_t btn_to_release = order[step];
            current_pins &= static_cast<uint8_t>(~(1 << btn_to_release));

            for (int t = 0; t < 5; ++t) {
                matrix.tick(current_pins);
            }
            assert(matrix.state() == current_pins);
            assert(matrix.was_released(static_cast<tkui::GameBoyButton>(1 << btn_to_release)));
        }
        assert(matrix.state() == 0x00);
        ++permutation_count;
    } while (std::next_permutation(order.begin(), order.end()));

    assert(permutation_count == 40320);
    std::cout << "  [PASS] Game Boy Matrix Exhaustive 40,320 Permutations passed (0 ghost keys, 0 stuck keys)." << std::endl;
}

inline void run_all_embedded_concurrency_tests() {
    std::cout << "\n=== EXECUTING EMBEDDED CONCURRENCY & HAL TEST SUITE ===" << std::endl;
    test_spsc_basic();
    test_spsc_wraparound_torture();
    test_spsc_multithreaded_stress();
    test_gameboy_matrix_bounce_rejection();
    test_gameboy_matrix_exhaustive_transitions();
    test_gameboy_matrix_exhaustive_release_permutations();
    std::cout << ">>> EMBEDDED CONCURRENCY & HAL SUITE: ALL TESTS PASSED <<<\n" << std::endl;
}

} // namespace test_embedded
