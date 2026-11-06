#pragma once
#include <cstdint>

namespace HeadlessX11 {
    // Must run before ScopedJuceInitialiser_GUI opens any display. No-op off Linux.
    void install() noexcept;
    bool isInstalled() noexcept;
    std::uint32_t swallowedErrorCount() noexcept;
    unsigned char lastErrorCode() noexcept;
    unsigned char lastRequestCode() noexcept;
}
