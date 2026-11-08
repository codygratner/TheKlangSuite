#pragma once

#include <array>

namespace Nyarlat {

// Represents one corner in the 8-vertex 3D parameter cube
struct ZCubeCorner {
    struct Section {
        float pole_rho;   // 0.0 .. 0.995 (strictly < 1.0)
        float pole_theta; // 0.0 .. pi (frequency in radians)
        float zero_rho;   // 0.0 .. 1.0
        float zero_theta; // 0.0 .. pi
    };
    std::array<Section, 7> sections;
    float gain_db = 0.0f; // Passband gain correction
};

} // namespace Nyarlat
