#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

#if defined(__cpp_lib_hardware_interference_size)
    using std::hardware_destructive_interference_size;
#else
    // Standard L1 cache line size for x86_64 and ARM Cortex-M7
    constexpr size_t hardware_destructive_interference_size = 64;
#endif

namespace tkdsp {

/**
 * @brief Lock-Free Single-Producer Single-Consumer (SPSC) Ring Buffer.
 * 
 * Implements a wait-free, bounded FIFO queue using the Rigtorp pattern.
 * Separates producer and consumer atomic indices across independent 64-byte cache lines
 * to eliminate false sharing. Enforces acquire/release memory ordering (compiling to
 * DMB ISH on ARM Cortex-M7 DTCM) to guarantee hard real-time safety.
 *
 * @tparam T Value type to store. Must be nothrow destructible.
 * @tparam Capacity Buffer size. Must be an exact power of two.
 */
template <typename T, size_t Capacity>
class SpscRing {
    static_assert((Capacity >= 2) && ((Capacity & (Capacity - 1)) == 0),
                  "Capacity must be an exact power of two (>= 2).");
    static_assert(std::atomic<uint32_t>::is_always_lock_free,
                  "Atomic uint32_t must be lock-free on target architecture.");
    static_assert(std::is_nothrow_destructible_v<T>,
                  "Element type T must be nothrow destructible.");

public:
    static constexpr size_t kCapacity = Capacity;
    static constexpr uint32_t kMask = static_cast<uint32_t>(Capacity - 1);

    SpscRing() noexcept
        : write_idx_(0),
          cached_read_idx_(0),
          read_idx_(0),
          cached_write_idx_(0) {}

    // Special constructor for testing 32-bit wraparound boundaries
    explicit SpscRing(uint32_t initial_index_seed) noexcept
        : write_idx_(initial_index_seed),
          cached_read_idx_(initial_index_seed),
          read_idx_(initial_index_seed),
          cached_write_idx_(initial_index_seed) {}

    ~SpscRing() noexcept {
        reset();
    }

    // Non-copyable, non-movable for concurrency isolation
    SpscRing(const SpscRing&) = delete;
    SpscRing& operator=(const SpscRing&) = delete;
    SpscRing(SpscRing&&) = delete;
    SpscRing& operator=(SpscRing&&) = delete;

    /**
     * @brief Pushes an element by copy. (Producer Thread Only)
     */
    bool try_push(const T& item) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        const uint32_t write = write_idx_.load(std::memory_order_relaxed);
        if (write - cached_read_idx_ >= kCapacity) {
            // Re-sample consumer index with acquire barrier
            cached_read_idx_ = read_idx_.load(std::memory_order_acquire);
            if (write - cached_read_idx_ >= kCapacity) {
                return false; // Queue full
            }
        }

        // Construct element in slot
        new (&storage_[write & kMask].storage) T(item);

        // Publish to consumer with release barrier (dmb ish on Cortex-M7)
        write_idx_.store(write + 1, std::memory_order_release);
        return true;
    }

    /**
     * @brief Pushes an element by move. (Producer Thread Only)
     */
    bool try_push(T&& item) noexcept(std::is_nothrow_move_constructible_v<T>) {
        const uint32_t write = write_idx_.load(std::memory_order_relaxed);
        if (write - cached_read_idx_ >= kCapacity) {
            cached_read_idx_ = read_idx_.load(std::memory_order_acquire);
            if (write - cached_read_idx_ >= kCapacity) {
                return false;
            }
        }

        new (&storage_[write & kMask].storage) T(std::move(item));
        write_idx_.store(write + 1, std::memory_order_release);
        return true;
    }

    /**
     * @brief Pops an element into the destination reference. (Consumer Thread Only)
     */
    bool try_pop(T& item) noexcept(std::is_nothrow_move_assignable_v<T>) {
        const uint32_t read = read_idx_.load(std::memory_order_relaxed);
        if (read == cached_write_idx_) {
            // Re-sample producer index with acquire barrier
            cached_write_idx_ = write_idx_.load(std::memory_order_acquire);
            if (read == cached_write_idx_) {
                return false; // Queue empty
            }
        }

        T* slot = reinterpret_cast<T*>(&storage_[read & kMask].storage);
        item = std::move(*slot);
        slot->~T();

        // Release slot back to producer
        read_idx_.store(read + 1, std::memory_order_release);
        return true;
    }

    /**
     * @brief Approximate current element count.
     */
    [[nodiscard]] size_t size() const noexcept {
        const uint32_t write = write_idx_.load(std::memory_order_relaxed);
        const uint32_t read = read_idx_.load(std::memory_order_relaxed);
        return static_cast<size_t>(write - read);
    }

    [[nodiscard]] constexpr size_t capacity() const noexcept {
        return Capacity;
    }

    [[nodiscard]] bool empty() const noexcept {
        return size() == 0;
    }

    [[nodiscard]] bool full() const noexcept {
        return size() >= Capacity;
    }

    /**
     * @brief Drains all remaining elements. (Not thread-safe; call during quiescence).
     */
    void reset() noexcept {
        uint32_t read = read_idx_.load(std::memory_order_relaxed);
        const uint32_t write = write_idx_.load(std::memory_order_relaxed);
        while (read != write) {
            T* slot = reinterpret_cast<T*>(&storage_[read & kMask].storage);
            slot->~T();
            ++read;
        }
        read_idx_.store(0, std::memory_order_relaxed);
        write_idx_.store(0, std::memory_order_relaxed);
        cached_read_idx_ = 0;
        cached_write_idx_ = 0;
    }

private:
    struct Slot {
        alignas(alignof(T)) std::byte storage[sizeof(T)];
    };

    // Producer data: Cache line 0
    alignas(hardware_destructive_interference_size) std::atomic<uint32_t> write_idx_{0};
    uint32_t cached_read_idx_{0};

    // Consumer data: Cache line 1
    alignas(hardware_destructive_interference_size) std::atomic<uint32_t> read_idx_{0};
    uint32_t cached_write_idx_{0};

    // Buffer storage: Cache line 2+
    alignas(hardware_destructive_interference_size) Slot storage_[Capacity];
};

} // namespace tkdsp
