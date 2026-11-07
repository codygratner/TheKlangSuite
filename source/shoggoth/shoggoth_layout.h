#pragma once

#include <cstdint>
#include <cstddef>
#include <array>

namespace Shoggoth {

inline constexpr int kMaxPartials     = 2048;
inline constexpr int kMax1987Partials = 126;                     // 63 in S1, 63 in S2
inline constexpr int kLanes           = 8;                       // AVX2 8x float32
inline constexpr int kBlocks          = kMaxPartials / kLanes;   // 256 vector blocks
inline constexpr int kSub             = 16;                      // samples per subblock
inline constexpr int kGuard           = 8;                       // 1 vector ghost cells on each side
inline constexpr int kGuarded         = kGuard + kMaxPartials + kGuard; // 2064 elements
inline constexpr int kMaxVoices       = 64;
inline constexpr int kNumDHGEnv       = 4;                       // 1987 DHG harmonic envelope groups
inline constexpr int kNumEnvStages    = 6;

// 100% portable C++20: raw float arrays with 64-byte cache alignment.
// Vectorization is performed via function-local intrinsics or simd8 wrappers.
struct alignas(64) ShoggothPartialsSoA {
    // --- HOT: touched every sample in vector inner loop ---
    alignas(64) float re  [kMaxPartials];   // Rotator state cos(theta_k)
    alignas(64) float im  [kMaxPartials];   // Rotator state sin(theta_k) (quadrature output tap)
    alignas(64) float wRe [kMaxPartials];   // Folded step g * cos(omega_k)
    alignas(64) float wIm [kMaxPartials];   // Folded step g * sin(omega_k)
    alignas(64) float amp [kMaxPartials];   // Current linear amplitude
    alignas(64) float dAmp[kMaxPartials];   // Per-sample linear delta (fallback for fast onsets)

    // --- WARM: touched once per 16-sample subblock ---
    alignas(64) float baseHarmonic[kMaxPartials]; // Master static timbre recipe for this voice note (0..1 linear)
    alignas(64) float ampTgt[kMaxPartials]; // Pre-cull target linear amplitude
    alignas(64) float ratio [kMaxPartials]; // Harmonic or inharmonic frequency ratio (f_k / f_0)
    alignas(64) float dOmega[kMaxPartials]; // Kuramoto frequency correction delta-omega_k [rad/sample]

    // --- Kuramoto Stencil Field with Ghost Cells (psi = detune phase, kAmp = coupling weight) ---
    alignas(64) float psi [kGuarded];       // Ghost cells at [0..7] and [2056..2063] initialized to 0
    alignas(64) float kAmp[kGuarded];       // Ghost weights = 0 -> branchless boundary handling

    // --- Harmonic Envelope Routing ---
    alignas(64) uint8_t envAssign[kMaxPartials]; // 0..3 -> DHG 1..4
};

static_assert(alignof(ShoggothPartialsSoA) == 64);

struct EnvStage {
    float rate{0.0f};  // 0..99
    float level{0.0f}; // 0..99
};

struct DHGEnvelopeData {
    EnvStage stages[kNumEnvStages];
    bool loop34{false};
};

struct EnvRuntimeState {
    float currentLevel{0.0f};
    float targetLevel{0.0f};
    float rateDelta{0.0f};
    int   stageIndex{0};
    bool  isFinished{false};
    bool  isReleased{false};
};

struct AhdsrState {
    enum class Phase : uint8_t { Idle, Attack, Hold, Decay, Sustain, Release };
    Phase phase{Phase::Idle};
    float currentLevel{0.0f};
    float holdTimerSec{0.0f};
};

struct BiquadState { // Transposed Direct Form II
    float b0{1.0f}, b1{0.0f}, b2{0.0f}, a1{0.0f}, a2{0.0f};
    float z1{0.0f}, z2{0.0f}, _pad{0.0f};
};

enum class VoicePhase : uint8_t { Idle, Attack, Sustain, Release, Steal };

struct alignas(64) ShoggothVoiceState {
    // --- Hot: Scheduler & Allocator ---
    float     f0Hz{440.0f};
    float     f0Tgt{440.0f};
    float     velocity{0.8f};
    float     voiceGain{1.0f};
    uint32_t  activePartials{126};
    uint32_t  activeBlocks{16}; // ceil(activePartials / 8)
    uint64_t  ageSamples{0};
    uint32_t  noteId{0};
    uint8_t   midiNote{69};
    VoicePhase phase{VoicePhase::Idle};
    bool      modernMode{true};
    uint8_t   _pad0[21]{0};

    // --- Modulation & Envelopes ---
    alignas(64) DHGEnvelopeData  dhgData[kNumDHGEnv];
    alignas(64) EnvRuntimeState  dhgRuntime[kNumDHGEnv];
    alignas(64) EnvRuntimeState  ddaRuntime;
    alignas(64) EnvRuntimeState  ddfRuntime;
    alignas(64) AhdsrState       ampAhdsr;
    alignas(64) AhdsrState       filterAhdsr;
    alignas(64) BiquadState      ddf; // Vintage post-filter biquad (L/R)
    BiquadState ddfR;

    float masterPhase{0.0f};
    float kuramotoK{0.0f};
    float pitchPunchOffset{0.0f};
    float pitchPunchDecay{0.0f};

    // --- The Partial Bank (SoA) ---
    alignas(64) ShoggothPartialsSoA p;
};

static_assert(alignof(ShoggothVoiceState) == 64);

} // namespace Shoggoth
