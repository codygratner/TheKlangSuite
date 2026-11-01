#pragma once

#include <cstdint>
#include <cstddef>
#include <array>

namespace tkui {

/**
 * @enum GameBoyButton
 * @brief Standard 8-button Game Boy / Dirtywave M8 physical key assignment.
 */
enum class GameBoyButton : uint8_t {
    Up      = 1 << 0, // 0x01
    Down    = 1 << 1, // 0x02
    Left    = 1 << 2, // 0x04
    Right   = 1 << 3, // 0x08
    A       = 1 << 4, // 0x10 (Action / Play)
    B       = 1 << 5, // 0x20 (Back / Cut)
    Select  = 1 << 6, // 0x40 (Option)
    Start   = 1 << 7  // 0x80 (Edit / Shift)
};

constexpr uint8_t operator|(GameBoyButton a, GameBoyButton b) noexcept {
    return static_cast<uint8_t>(a) | static_cast<uint8_t>(b);
}

/**
 * @brief Pure 1kHz Debounce Integrator & 8-Button Matrix State Machine.
 *
 * ⚠️ HARDWARE PROTECTION INVARIANT:
 * All physical button circuits connected to Teensy 4.1 (NXP i.MX RT1062) MUST
 * operate strictly at 3.3V logic levels with internal or external pull-ups.
 * The i.MX RT1062 GPIO pins are NOT 5V tolerant. Direct connection to vintage 5V
 * Game Boy DMG buses without bidirectional level shifters will cause permanent silicon destruction.
 *
 * This state machine runs deterministically at 1 kHz (1ms sampling intervals) and uses
 * saturating integrator counters per button to reject contact bounce while ensuring
 * zero latency on confirmed transitions.
 */
template <uint8_t DebounceThreshold = 5>
class GameBoyMatrix {
    static_assert(DebounceThreshold >= 1 && DebounceThreshold <= 64,
                  "DebounceThreshold must be between 1 and 64 ms.");

public:
    static constexpr size_t kNumButtons = 8;
    static constexpr uint8_t kThreshold = DebounceThreshold;

    GameBoyMatrix() noexcept {
        reset();
    }

    /**
     * @brief Resets all integrator counters and state bitmasks to zero.
     */
    void reset() noexcept {
        integrators_.fill(0);
        current_state_ = 0;
        just_pressed_ = 0;
        just_released_ = 0;
    }

    /**
     * @brief Advances the debounce state machine by 1 tick (1 ms at 1 kHz).
     *
     * @param raw_input 8-bit input bitmask where 1 indicates button depressed.
     *                  For active-low pull-up circuits (where pressed = 0V),
     *                  pass ~raw_pins.
     */
    void tick(uint8_t raw_input) noexcept {
        uint8_t prev_state = current_state_;
        uint8_t new_state = current_state_;

        for (size_t i = 0; i < kNumButtons; ++i) {
            const uint8_t mask = static_cast<uint8_t>(1 << i);
            const bool raw_is_high = (raw_input & mask) != 0;

            if (raw_is_high) {
                if (integrators_[i] < kThreshold) {
                    ++integrators_[i];
                    if (integrators_[i] == kThreshold) {
                        new_state |= mask; // Transition to PRESSED
                    }
                }
            } else {
                if (integrators_[i] > 0) {
                    --integrators_[i];
                    if (integrators_[i] == 0) {
                        new_state &= ~mask; // Transition to RELEASED
                    }
                }
            }
        }

        current_state_ = new_state;
        just_pressed_ = static_cast<uint8_t>((~prev_state) & current_state_);
        just_released_ = static_cast<uint8_t>(prev_state & (~current_state_));
    }

    /**
     * @brief Convenience update for active-low hardware GPIO pins.
     * @param active_low_pins Raw GPIO register (0 = pressed, 1 = unpressed).
     */
    void tick_active_low(uint8_t active_low_pins) noexcept {
        tick(static_cast<uint8_t>(~active_low_pins));
    }

    [[nodiscard]] bool is_down(GameBoyButton btn) const noexcept {
        return (current_state_ & static_cast<uint8_t>(btn)) != 0;
    }

    [[nodiscard]] bool was_pressed(GameBoyButton btn) const noexcept {
        return (just_pressed_ & static_cast<uint8_t>(btn)) != 0;
    }

    [[nodiscard]] bool was_released(GameBoyButton btn) const noexcept {
        return (just_released_ & static_cast<uint8_t>(btn)) != 0;
    }

    [[nodiscard]] uint8_t state() const noexcept {
        return current_state_;
    }

    [[nodiscard]] uint8_t pressed_edges() const noexcept {
        return just_pressed_;
    }

    [[nodiscard]] uint8_t released_edges() const noexcept {
        return just_released_;
    }

    [[nodiscard]] uint8_t integrator(size_t button_idx) const noexcept {
        return (button_idx < kNumButtons) ? integrators_[button_idx] : 0;
    }

private:
    std::array<uint8_t, kNumButtons> integrators_{};
    uint8_t current_state_{0};
    uint8_t just_pressed_{0};
    uint8_t just_released_{0};
};

} // namespace tkui
