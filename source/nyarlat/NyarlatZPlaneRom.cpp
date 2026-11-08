// ==============================================================================
// NyarlatZPlaneRom.cpp — Static ROM Tables for 288 Z-Plane Setups
// ==============================================================================
#include "NyarlatZPlaneRom.h"

namespace Nyarlat {

static const std::array<const char*, 8> sBankNames = {{
    "VOWELS & FORMANTS",
    "ACOUSTIC BODIES",
    "COMB CLUSTERS",
    "PHASERS & FLANGERS",
    "CLASSIC SYNTH",
    "TOPOLOGICAL MORPHS",
    "NONLINEAR & EDGY",
    "COSMIC & CHAOS"
}};

const char* ZPlaneRom::getBankName(int bankIndex) noexcept {
    if (bankIndex >= 0 && bankIndex < 8) return sBankNames[bankIndex];
    return "UNKNOWN";
}

static const std::array<ZPlaneModelDef, 288>& getStaticRom() noexcept {
    static const auto rom = []() {
        std::array<ZPlaneModelDef, 288> defs;
        int idx = 0;
        // [001] 001 Vocal Ah-Ee
        defs[0].name = "001 Vocal Ah-Ee";
        defs[0].category = "VOWEL";
        defs[0].bankIndex = 0;
        defs[0].activeStages = 5;
        defs[0].isPlanar4 = false;
        defs[0].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[0].corners[c].sections[s].pole_rho = rh;
                defs[0].corners[c].sections[s].pole_theta = th;
                defs[0].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[0].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [002] 002 Vocal Oo-Ah
        defs[1].name = "002 Vocal Oo-Ah";
        defs[1].category = "VOWEL";
        defs[1].bankIndex = 0;
        defs[1].activeStages = 5;
        defs[1].isPlanar4 = false;
        defs[1].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[1].corners[c].sections[s].pole_rho = rh;
                defs[1].corners[c].sections[s].pole_theta = th;
                defs[1].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[1].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [003] 003 Vocal Ee-Yi
        defs[2].name = "003 Vocal Ee-Yi";
        defs[2].category = "VOWEL";
        defs[2].bankIndex = 0;
        defs[2].activeStages = 5;
        defs[2].isPlanar4 = false;
        defs[2].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[2].corners[c].sections[s].pole_rho = rh;
                defs[2].corners[c].sections[s].pole_theta = th;
                defs[2].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[2].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [004] 004 Vocal Ah-Oh
        defs[3].name = "004 Vocal Ah-Oh";
        defs[3].category = "VOWEL";
        defs[3].bankIndex = 0;
        defs[3].activeStages = 5;
        defs[3].isPlanar4 = false;
        defs[3].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[3].corners[c].sections[s].pole_rho = rh;
                defs[3].corners[c].sections[s].pole_theta = th;
                defs[3].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[3].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [005] 005 Choir AA-EE
        defs[4].name = "005 Choir AA-EE";
        defs[4].category = "VOWEL";
        defs[4].bankIndex = 0;
        defs[4].activeStages = 6;
        defs[4].isPlanar4 = false;
        defs[4].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[4].corners[c].sections[s].pole_rho = rh;
                defs[4].corners[c].sections[s].pole_theta = th;
                defs[4].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[4].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [006] 006 Deep Throat
        defs[5].name = "006 Deep Throat";
        defs[5].category = "VOWEL";
        defs[5].bankIndex = 0;
        defs[5].activeStages = 6;
        defs[5].isPlanar4 = false;
        defs[5].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[5].corners[c].sections[s].pole_rho = rh;
                defs[5].corners[c].sections[s].pole_theta = th;
                defs[5].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[5].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [007] 007 Talking Box
        defs[6].name = "007 Talking Box";
        defs[6].category = "VOWEL";
        defs[6].bankIndex = 0;
        defs[6].activeStages = 6;
        defs[6].isPlanar4 = false;
        defs[6].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[6].corners[c].sections[s].pole_rho = rh;
                defs[6].corners[c].sections[s].pole_theta = th;
                defs[6].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[6].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [008] 008 Nasal Resonator
        defs[7].name = "008 Nasal Resonator";
        defs[7].category = "VOWEL";
        defs[7].bankIndex = 0;
        defs[7].activeStages = 6;
        defs[7].isPlanar4 = false;
        defs[7].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[7].corners[c].sections[s].pole_rho = rh;
                defs[7].corners[c].sections[s].pole_theta = th;
                defs[7].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[7].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [009] 009 Bass Formants
        defs[8].name = "009 Bass Formants";
        defs[8].category = "VOWEL";
        defs[8].bankIndex = 0;
        defs[8].activeStages = 6;
        defs[8].isPlanar4 = false;
        defs[8].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[8].corners[c].sections[s].pole_rho = rh;
                defs[8].corners[c].sections[s].pole_theta = th;
                defs[8].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[8].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [010] 010 Soprano Morph
        defs[9].name = "010 Soprano Morph";
        defs[9].category = "VOWEL";
        defs[9].bankIndex = 0;
        defs[9].activeStages = 6;
        defs[9].isPlanar4 = false;
        defs[9].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[9].corners[c].sections[s].pole_rho = rh;
                defs[9].corners[c].sections[s].pole_theta = th;
                defs[9].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[9].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [011] 011 Choral Blend
        defs[10].name = "011 Choral Blend";
        defs[10].category = "VOWEL";
        defs[10].bankIndex = 0;
        defs[10].activeStages = 6;
        defs[10].isPlanar4 = false;
        defs[10].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[10].corners[c].sections[s].pole_rho = rh;
                defs[10].corners[c].sections[s].pole_theta = th;
                defs[10].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[10].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [012] 012 Vocal Cry
        defs[11].name = "012 Vocal Cry";
        defs[11].category = "VOWEL";
        defs[11].bankIndex = 0;
        defs[11].activeStages = 5;
        defs[11].isPlanar4 = false;
        defs[11].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[11].corners[c].sections[s].pole_rho = rh;
                defs[11].corners[c].sections[s].pole_theta = th;
                defs[11].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[11].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [013] 013 Diphthong Morph
        defs[12].name = "013 Diphthong Morph";
        defs[12].category = "VOWEL";
        defs[12].bankIndex = 0;
        defs[12].activeStages = 6;
        defs[12].isPlanar4 = false;
        defs[12].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[12].corners[c].sections[s].pole_rho = rh;
                defs[12].corners[c].sections[s].pole_theta = th;
                defs[12].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[12].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [014] 014 Whisper Vocal
        defs[13].name = "014 Whisper Vocal";
        defs[13].category = "VOWEL";
        defs[13].bankIndex = 0;
        defs[13].activeStages = 5;
        defs[13].isPlanar4 = false;
        defs[13].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[13].corners[c].sections[s].pole_rho = rh;
                defs[13].corners[c].sections[s].pole_theta = th;
                defs[13].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[13].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [015] 015 Vocal Swell
        defs[14].name = "015 Vocal Swell";
        defs[14].category = "VOWEL";
        defs[14].bankIndex = 0;
        defs[14].activeStages = 5;
        defs[14].isPlanar4 = false;
        defs[14].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[14].corners[c].sections[s].pole_rho = rh;
                defs[14].corners[c].sections[s].pole_theta = th;
                defs[14].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[14].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [016] 016 Jaw Talker
        defs[15].name = "016 Jaw Talker";
        defs[15].category = "VOWEL";
        defs[15].bankIndex = 0;
        defs[15].activeStages = 6;
        defs[15].isPlanar4 = false;
        defs[15].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[15].corners[c].sections[s].pole_rho = rh;
                defs[15].corners[c].sections[s].pole_theta = th;
                defs[15].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[15].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [017] 017 Formant Glide
        defs[16].name = "017 Formant Glide";
        defs[16].category = "VOWEL";
        defs[16].bankIndex = 0;
        defs[16].activeStages = 6;
        defs[16].isPlanar4 = false;
        defs[16].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[16].corners[c].sections[s].pole_rho = rh;
                defs[16].corners[c].sections[s].pole_theta = th;
                defs[16].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[16].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [018] 018 Ooh-Wah
        defs[17].name = "018 Ooh-Wah";
        defs[17].category = "VOWEL";
        defs[17].bankIndex = 0;
        defs[17].activeStages = 6;
        defs[17].isPlanar4 = false;
        defs[17].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[17].corners[c].sections[s].pole_rho = rh;
                defs[17].corners[c].sections[s].pole_theta = th;
                defs[17].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[17].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [019] 019 Aah-Ooh
        defs[18].name = "019 Aah-Ooh";
        defs[18].category = "VOWEL";
        defs[18].bankIndex = 0;
        defs[18].activeStages = 6;
        defs[18].isPlanar4 = false;
        defs[18].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[18].corners[c].sections[s].pole_rho = rh;
                defs[18].corners[c].sections[s].pole_theta = th;
                defs[18].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[18].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [020] 020 Vocal Sweep 1
        defs[19].name = "020 Vocal Sweep 1";
        defs[19].category = "VOWEL";
        defs[19].bankIndex = 0;
        defs[19].activeStages = 5;
        defs[19].isPlanar4 = false;
        defs[19].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[19].corners[c].sections[s].pole_rho = rh;
                defs[19].corners[c].sections[s].pole_theta = th;
                defs[19].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[19].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [021] 021 Vocal Sweep 2
        defs[20].name = "021 Vocal Sweep 2";
        defs[20].category = "VOWEL";
        defs[20].bankIndex = 0;
        defs[20].activeStages = 5;
        defs[20].isPlanar4 = false;
        defs[20].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[20].corners[c].sections[s].pole_rho = rh;
                defs[20].corners[c].sections[s].pole_theta = th;
                defs[20].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[20].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [022] 022 Choral Pad
        defs[21].name = "022 Choral Pad";
        defs[21].category = "VOWEL";
        defs[21].bankIndex = 0;
        defs[21].activeStages = 6;
        defs[21].isPlanar4 = false;
        defs[21].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[21].corners[c].sections[s].pole_rho = rh;
                defs[21].corners[c].sections[s].pole_theta = th;
                defs[21].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[21].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [023] 023 Throat Sing
        defs[22].name = "023 Throat Sing";
        defs[22].category = "VOWEL";
        defs[22].bankIndex = 0;
        defs[22].activeStages = 6;
        defs[22].isPlanar4 = false;
        defs[22].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[22].corners[c].sections[s].pole_rho = rh;
                defs[22].corners[c].sections[s].pole_theta = th;
                defs[22].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[22].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [024] 024 Pharyngeal
        defs[23].name = "024 Pharyngeal";
        defs[23].category = "VOWEL";
        defs[23].bankIndex = 0;
        defs[23].activeStages = 6;
        defs[23].isPlanar4 = false;
        defs[23].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[23].corners[c].sections[s].pole_rho = rh;
                defs[23].corners[c].sections[s].pole_theta = th;
                defs[23].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[23].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [025] 025 Palatal Morph
        defs[24].name = "025 Palatal Morph";
        defs[24].category = "VOWEL";
        defs[24].bankIndex = 0;
        defs[24].activeStages = 6;
        defs[24].isPlanar4 = false;
        defs[24].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[24].corners[c].sections[s].pole_rho = rh;
                defs[24].corners[c].sections[s].pole_theta = th;
                defs[24].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[24].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [026] 026 Velar Sweep
        defs[25].name = "026 Velar Sweep";
        defs[25].category = "VOWEL";
        defs[25].bankIndex = 0;
        defs[25].activeStages = 6;
        defs[25].isPlanar4 = false;
        defs[25].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[25].corners[c].sections[s].pole_rho = rh;
                defs[25].corners[c].sections[s].pole_theta = th;
                defs[25].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[25].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [027] 027 Labial Formant
        defs[26].name = "027 Labial Formant";
        defs[26].category = "VOWEL";
        defs[26].bankIndex = 0;
        defs[26].activeStages = 6;
        defs[26].isPlanar4 = false;
        defs[26].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[26].corners[c].sections[s].pole_rho = rh;
                defs[26].corners[c].sections[s].pole_theta = th;
                defs[26].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[26].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [028] 028 Guttural Res
        defs[27].name = "028 Guttural Res";
        defs[27].category = "VOWEL";
        defs[27].bankIndex = 0;
        defs[27].activeStages = 6;
        defs[27].isPlanar4 = false;
        defs[27].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[27].corners[c].sections[s].pole_rho = rh;
                defs[27].corners[c].sections[s].pole_theta = th;
                defs[27].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[27].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [029] 029 Vocal Drone
        defs[28].name = "029 Vocal Drone";
        defs[28].category = "VOWEL";
        defs[28].bankIndex = 0;
        defs[28].activeStages = 5;
        defs[28].isPlanar4 = false;
        defs[28].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[28].corners[c].sections[s].pole_rho = rh;
                defs[28].corners[c].sections[s].pole_theta = th;
                defs[28].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[28].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [030] 030 Morphing Monk
        defs[29].name = "030 Morphing Monk";
        defs[29].category = "VOWEL";
        defs[29].bankIndex = 0;
        defs[29].activeStages = 6;
        defs[29].isPlanar4 = false;
        defs[29].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[29].corners[c].sections[s].pole_rho = rh;
                defs[29].corners[c].sections[s].pole_theta = th;
                defs[29].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[29].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [031] 031 Baritone Vox
        defs[30].name = "031 Baritone Vox";
        defs[30].category = "VOWEL";
        defs[30].bankIndex = 0;
        defs[30].activeStages = 6;
        defs[30].isPlanar4 = false;
        defs[30].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[30].corners[c].sections[s].pole_rho = rh;
                defs[30].corners[c].sections[s].pole_theta = th;
                defs[30].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[30].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [032] 032 Tenor Formant
        defs[31].name = "032 Tenor Formant";
        defs[31].category = "VOWEL";
        defs[31].bankIndex = 0;
        defs[31].activeStages = 6;
        defs[31].isPlanar4 = false;
        defs[31].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[31].corners[c].sections[s].pole_rho = rh;
                defs[31].corners[c].sections[s].pole_theta = th;
                defs[31].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[31].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [033] 033 Vocal Lead
        defs[32].name = "033 Vocal Lead";
        defs[32].category = "VOWEL";
        defs[32].bankIndex = 0;
        defs[32].activeStages = 5;
        defs[32].isPlanar4 = false;
        defs[32].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[32].corners[c].sections[s].pole_rho = rh;
                defs[32].corners[c].sections[s].pole_theta = th;
                defs[32].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[32].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [034] 034 Resonant Talker
        defs[33].name = "034 Resonant Talker";
        defs[33].category = "VOWEL";
        defs[33].bankIndex = 0;
        defs[33].activeStages = 6;
        defs[33].isPlanar4 = false;
        defs[33].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[33].corners[c].sections[s].pole_rho = rh;
                defs[33].corners[c].sections[s].pole_theta = th;
                defs[33].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[33].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [035] 035 Formant Matrix
        defs[34].name = "035 Formant Matrix";
        defs[34].category = "VOWEL";
        defs[34].bankIndex = 0;
        defs[34].activeStages = 6;
        defs[34].isPlanar4 = false;
        defs[34].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[34].corners[c].sections[s].pole_rho = rh;
                defs[34].corners[c].sections[s].pole_theta = th;
                defs[34].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[34].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [036] 036 Acoustic Vowel
        defs[35].name = "036 Acoustic Vowel";
        defs[35].category = "VOWEL";
        defs[35].bankIndex = 0;
        defs[35].activeStages = 6;
        defs[35].isPlanar4 = false;
        defs[35].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[35].corners[c].sections[s].pole_rho = rh;
                defs[35].corners[c].sections[s].pole_theta = th;
                defs[35].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[35].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [037] 037 Cello Body
        defs[36].name = "037 Cello Body";
        defs[36].category = "BODY";
        defs[36].bankIndex = 1;
        defs[36].activeStages = 6;
        defs[36].isPlanar4 = false;
        defs[36].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[36].corners[c].sections[s].pole_rho = rh;
                defs[36].corners[c].sections[s].pole_theta = th;
                defs[36].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[36].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [038] 038 Wood Cavity
        defs[37].name = "038 Wood Cavity";
        defs[37].category = "BODY";
        defs[37].bankIndex = 1;
        defs[37].activeStages = 6;
        defs[37].isPlanar4 = false;
        defs[37].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[37].corners[c].sections[s].pole_rho = rh;
                defs[37].corners[c].sections[s].pole_theta = th;
                defs[37].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[37].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [039] 039 Ceramic Horn
        defs[38].name = "039 Ceramic Horn";
        defs[38].category = "BODY";
        defs[38].bankIndex = 1;
        defs[38].activeStages = 6;
        defs[38].isPlanar4 = false;
        defs[38].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[38].corners[c].sections[s].pole_rho = rh;
                defs[38].corners[c].sections[s].pole_theta = th;
                defs[38].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[38].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [040] 040 Bell Shell
        defs[39].name = "040 Bell Shell";
        defs[39].category = "BODY";
        defs[39].bankIndex = 1;
        defs[39].activeStages = 6;
        defs[39].isPlanar4 = false;
        defs[39].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[39].corners[c].sections[s].pole_rho = rh;
                defs[39].corners[c].sections[s].pole_theta = th;
                defs[39].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[39].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [041] 041 Metallic Plate
        defs[40].name = "041 Metallic Plate";
        defs[40].category = "BODY";
        defs[40].bankIndex = 1;
        defs[40].activeStages = 6;
        defs[40].isPlanar4 = false;
        defs[40].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[40].corners[c].sections[s].pole_rho = rh;
                defs[40].corners[c].sections[s].pole_theta = th;
                defs[40].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[40].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [042] 042 Piano Body
        defs[41].name = "042 Piano Body";
        defs[41].category = "BODY";
        defs[41].bankIndex = 1;
        defs[41].activeStages = 6;
        defs[41].isPlanar4 = false;
        defs[41].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[41].corners[c].sections[s].pole_rho = rh;
                defs[41].corners[c].sections[s].pole_theta = th;
                defs[41].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[41].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [043] 043 Bass Cavity
        defs[42].name = "043 Bass Cavity";
        defs[42].category = "BODY";
        defs[42].bankIndex = 1;
        defs[42].activeStages = 6;
        defs[42].isPlanar4 = false;
        defs[42].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[42].corners[c].sections[s].pole_rho = rh;
                defs[42].corners[c].sections[s].pole_theta = th;
                defs[42].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[42].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [044] 044 Violin Body
        defs[43].name = "044 Violin Body";
        defs[43].category = "BODY";
        defs[43].bankIndex = 1;
        defs[43].activeStages = 6;
        defs[43].isPlanar4 = false;
        defs[43].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[43].corners[c].sections[s].pole_rho = rh;
                defs[43].corners[c].sections[s].pole_theta = th;
                defs[43].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[43].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [045] 045 String Box
        defs[44].name = "045 String Box";
        defs[44].category = "BODY";
        defs[44].bankIndex = 1;
        defs[44].activeStages = 6;
        defs[44].isPlanar4 = false;
        defs[44].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[44].corners[c].sections[s].pole_rho = rh;
                defs[44].corners[c].sections[s].pole_theta = th;
                defs[44].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[44].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [046] 046 Tuned Pipe
        defs[45].name = "046 Tuned Pipe";
        defs[45].category = "BODY";
        defs[45].bankIndex = 1;
        defs[45].activeStages = 6;
        defs[45].isPlanar4 = false;
        defs[45].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[45].corners[c].sections[s].pole_rho = rh;
                defs[45].corners[c].sections[s].pole_theta = th;
                defs[45].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[45].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [047] 047 Brass Chamber
        defs[46].name = "047 Brass Chamber";
        defs[46].category = "BODY";
        defs[46].bankIndex = 1;
        defs[46].activeStages = 6;
        defs[46].isPlanar4 = false;
        defs[46].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[46].corners[c].sections[s].pole_rho = rh;
                defs[46].corners[c].sections[s].pole_theta = th;
                defs[46].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[46].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [048] 048 Saxophone Bell
        defs[47].name = "048 Saxophone Bell";
        defs[47].category = "BODY";
        defs[47].bankIndex = 1;
        defs[47].activeStages = 6;
        defs[47].isPlanar4 = false;
        defs[47].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[47].corners[c].sections[s].pole_rho = rh;
                defs[47].corners[c].sections[s].pole_theta = th;
                defs[47].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[47].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [049] 049 Clarinet Tube
        defs[48].name = "049 Clarinet Tube";
        defs[48].category = "BODY";
        defs[48].bankIndex = 1;
        defs[48].activeStages = 6;
        defs[48].isPlanar4 = false;
        defs[48].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[48].corners[c].sections[s].pole_rho = rh;
                defs[48].corners[c].sections[s].pole_theta = th;
                defs[48].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[48].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [050] 050 Flute Airway
        defs[49].name = "050 Flute Airway";
        defs[49].category = "BODY";
        defs[49].bankIndex = 1;
        defs[49].activeStages = 6;
        defs[49].isPlanar4 = false;
        defs[49].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[49].corners[c].sections[s].pole_rho = rh;
                defs[49].corners[c].sections[s].pole_theta = th;
                defs[49].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[49].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [051] 051 Membrane Drum
        defs[50].name = "051 Membrane Drum";
        defs[50].category = "BODY";
        defs[50].bankIndex = 1;
        defs[50].activeStages = 6;
        defs[50].isPlanar4 = false;
        defs[50].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[50].corners[c].sections[s].pole_rho = rh;
                defs[50].corners[c].sections[s].pole_theta = th;
                defs[50].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[50].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [052] 052 Gong Resonance
        defs[51].name = "052 Gong Resonance";
        defs[51].category = "BODY";
        defs[51].bankIndex = 1;
        defs[51].activeStages = 6;
        defs[51].isPlanar4 = false;
        defs[51].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[51].corners[c].sections[s].pole_rho = rh;
                defs[51].corners[c].sections[s].pole_theta = th;
                defs[51].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[51].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [053] 053 Chime Cluster
        defs[52].name = "053 Chime Cluster";
        defs[52].category = "BODY";
        defs[52].bankIndex = 1;
        defs[52].activeStages = 6;
        defs[52].isPlanar4 = false;
        defs[52].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[52].corners[c].sections[s].pole_rho = rh;
                defs[52].corners[c].sections[s].pole_theta = th;
                defs[52].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[52].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [054] 054 Resonant Wood
        defs[53].name = "054 Resonant Wood";
        defs[53].category = "BODY";
        defs[53].bankIndex = 1;
        defs[53].activeStages = 6;
        defs[53].isPlanar4 = false;
        defs[53].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[53].corners[c].sections[s].pole_rho = rh;
                defs[53].corners[c].sections[s].pole_theta = th;
                defs[53].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[53].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [055] 055 Acoustic Horn
        defs[54].name = "055 Acoustic Horn";
        defs[54].category = "BODY";
        defs[54].bankIndex = 1;
        defs[54].activeStages = 6;
        defs[54].isPlanar4 = false;
        defs[54].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[54].corners[c].sections[s].pole_rho = rh;
                defs[54].corners[c].sections[s].pole_theta = th;
                defs[54].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[54].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [056] 056 Steel Drum
        defs[55].name = "056 Steel Drum";
        defs[55].category = "BODY";
        defs[55].bankIndex = 1;
        defs[55].activeStages = 6;
        defs[55].isPlanar4 = false;
        defs[55].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[55].corners[c].sections[s].pole_rho = rh;
                defs[55].corners[c].sections[s].pole_theta = th;
                defs[55].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[55].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [057] 057 Hollow Log
        defs[56].name = "057 Hollow Log";
        defs[56].category = "BODY";
        defs[56].bankIndex = 1;
        defs[56].activeStages = 6;
        defs[56].isPlanar4 = false;
        defs[56].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[56].corners[c].sections[s].pole_rho = rh;
                defs[56].corners[c].sections[s].pole_theta = th;
                defs[56].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[56].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [058] 058 Glass Jar
        defs[57].name = "058 Glass Jar";
        defs[57].category = "BODY";
        defs[57].bankIndex = 1;
        defs[57].activeStages = 6;
        defs[57].isPlanar4 = false;
        defs[57].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[57].corners[c].sections[s].pole_rho = rh;
                defs[57].corners[c].sections[s].pole_theta = th;
                defs[57].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[57].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [059] 059 Bronze Cauldron
        defs[58].name = "059 Bronze Cauldron";
        defs[58].category = "BODY";
        defs[58].bankIndex = 1;
        defs[58].activeStages = 6;
        defs[58].isPlanar4 = false;
        defs[58].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[58].corners[c].sections[s].pole_rho = rh;
                defs[58].corners[c].sections[s].pole_theta = th;
                defs[58].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[58].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [060] 060 Silver Bell
        defs[59].name = "060 Silver Bell";
        defs[59].category = "BODY";
        defs[59].bankIndex = 1;
        defs[59].activeStages = 6;
        defs[59].isPlanar4 = false;
        defs[59].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[59].corners[c].sections[s].pole_rho = rh;
                defs[59].corners[c].sections[s].pole_theta = th;
                defs[59].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[59].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [061] 061 Acoustic Guitar
        defs[60].name = "061 Acoustic Guitar";
        defs[60].category = "BODY";
        defs[60].bankIndex = 1;
        defs[60].activeStages = 6;
        defs[60].isPlanar4 = false;
        defs[60].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[60].corners[c].sections[s].pole_rho = rh;
                defs[60].corners[c].sections[s].pole_theta = th;
                defs[60].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[60].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [062] 062 Double Bass Box
        defs[61].name = "062 Double Bass Box";
        defs[61].category = "BODY";
        defs[61].bankIndex = 1;
        defs[61].activeStages = 6;
        defs[61].isPlanar4 = false;
        defs[61].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[61].corners[c].sections[s].pole_rho = rh;
                defs[61].corners[c].sections[s].pole_theta = th;
                defs[61].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[61].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [063] 063 Marimba Bar
        defs[62].name = "063 Marimba Bar";
        defs[62].category = "BODY";
        defs[62].bankIndex = 1;
        defs[62].activeStages = 6;
        defs[62].isPlanar4 = false;
        defs[62].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[62].corners[c].sections[s].pole_rho = rh;
                defs[62].corners[c].sections[s].pole_theta = th;
                defs[62].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[62].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [064] 064 Vibraphone Tube
        defs[63].name = "064 Vibraphone Tube";
        defs[63].category = "BODY";
        defs[63].bankIndex = 1;
        defs[63].activeStages = 6;
        defs[63].isPlanar4 = false;
        defs[63].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[63].corners[c].sections[s].pole_rho = rh;
                defs[63].corners[c].sections[s].pole_theta = th;
                defs[63].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[63].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [065] 065 Timpani Bowl
        defs[64].name = "065 Timpani Bowl";
        defs[64].category = "BODY";
        defs[64].bankIndex = 1;
        defs[64].activeStages = 6;
        defs[64].isPlanar4 = false;
        defs[64].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[64].corners[c].sections[s].pole_rho = rh;
                defs[64].corners[c].sections[s].pole_theta = th;
                defs[64].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[64].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [066] 066 Snare Shell
        defs[65].name = "066 Snare Shell";
        defs[65].category = "BODY";
        defs[65].bankIndex = 1;
        defs[65].activeStages = 6;
        defs[65].isPlanar4 = false;
        defs[65].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[65].corners[c].sections[s].pole_rho = rh;
                defs[65].corners[c].sections[s].pole_theta = th;
                defs[65].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[65].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [067] 067 Cymbal Ring
        defs[66].name = "067 Cymbal Ring";
        defs[66].category = "BODY";
        defs[66].bankIndex = 1;
        defs[66].activeStages = 6;
        defs[66].isPlanar4 = false;
        defs[66].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[66].corners[c].sections[s].pole_rho = rh;
                defs[66].corners[c].sections[s].pole_theta = th;
                defs[66].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[66].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [068] 068 Tubular Bell
        defs[67].name = "068 Tubular Bell";
        defs[67].category = "BODY";
        defs[67].bankIndex = 1;
        defs[67].activeStages = 6;
        defs[67].isPlanar4 = false;
        defs[67].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[67].corners[c].sections[s].pole_rho = rh;
                defs[67].corners[c].sections[s].pole_theta = th;
                defs[67].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[67].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [069] 069 Temple Block
        defs[68].name = "069 Temple Block";
        defs[68].category = "BODY";
        defs[68].bankIndex = 1;
        defs[68].activeStages = 6;
        defs[68].isPlanar4 = false;
        defs[68].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[68].corners[c].sections[s].pole_rho = rh;
                defs[68].corners[c].sections[s].pole_theta = th;
                defs[68].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[68].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [070] 070 Kalimba Tine
        defs[69].name = "070 Kalimba Tine";
        defs[69].category = "BODY";
        defs[69].bankIndex = 1;
        defs[69].activeStages = 6;
        defs[69].isPlanar4 = false;
        defs[69].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[69].corners[c].sections[s].pole_rho = rh;
                defs[69].corners[c].sections[s].pole_theta = th;
                defs[69].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[69].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [071] 071 Music Box Comb
        defs[70].name = "071 Music Box Comb";
        defs[70].category = "BODY";
        defs[70].bankIndex = 1;
        defs[70].activeStages = 6;
        defs[70].isPlanar4 = false;
        defs[70].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[70].corners[c].sections[s].pole_rho = rh;
                defs[70].corners[c].sections[s].pole_theta = th;
                defs[70].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[70].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [072] 072 Wind Chime
        defs[71].name = "072 Wind Chime";
        defs[71].category = "BODY";
        defs[71].bankIndex = 1;
        defs[71].activeStages = 6;
        defs[71].isPlanar4 = false;
        defs[71].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[71].corners[c].sections[s].pole_rho = rh;
                defs[71].corners[c].sections[s].pole_theta = th;
                defs[71].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[71].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [073] 073 Harmonic Comb
        defs[72].name = "073 Harmonic Comb";
        defs[72].category = "COMB";
        defs[72].bankIndex = 2;
        defs[72].activeStages = 5;
        defs[72].isPlanar4 = false;
        defs[72].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[72].corners[c].sections[s].pole_rho = rh;
                defs[72].corners[c].sections[s].pole_theta = th;
                defs[72].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[72].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [074] 074 Dense Cluster
        defs[73].name = "074 Dense Cluster";
        defs[73].category = "COMB";
        defs[73].bankIndex = 2;
        defs[73].activeStages = 5;
        defs[73].isPlanar4 = false;
        defs[73].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[73].corners[c].sections[s].pole_rho = rh;
                defs[73].corners[c].sections[s].pole_theta = th;
                defs[73].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[73].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [075] 075 Choral Comb
        defs[74].name = "075 Choral Comb";
        defs[74].category = "COMB";
        defs[74].bankIndex = 2;
        defs[74].activeStages = 5;
        defs[74].isPlanar4 = false;
        defs[74].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[74].corners[c].sections[s].pole_rho = rh;
                defs[74].corners[c].sections[s].pole_theta = th;
                defs[74].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[74].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [076] 076 Deep Combs
        defs[75].name = "076 Deep Combs";
        defs[75].category = "COMB";
        defs[75].bankIndex = 2;
        defs[75].activeStages = 5;
        defs[75].isPlanar4 = false;
        defs[75].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[75].corners[c].sections[s].pole_rho = rh;
                defs[75].corners[c].sections[s].pole_theta = th;
                defs[75].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[75].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [077] 077 Ring Combs
        defs[76].name = "077 Ring Combs";
        defs[76].category = "COMB";
        defs[76].bankIndex = 2;
        defs[76].activeStages = 5;
        defs[76].isPlanar4 = false;
        defs[76].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[76].corners[c].sections[s].pole_rho = rh;
                defs[76].corners[c].sections[s].pole_theta = th;
                defs[76].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[76].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [078] 078 Microtonal Array
        defs[77].name = "078 Microtonal Array";
        defs[77].category = "COMB";
        defs[77].bankIndex = 2;
        defs[77].activeStages = 5;
        defs[77].isPlanar4 = false;
        defs[77].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[77].corners[c].sections[s].pole_rho = rh;
                defs[77].corners[c].sections[s].pole_theta = th;
                defs[77].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[77].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [079] 079 Wide Comb 1
        defs[78].name = "079 Wide Comb 1";
        defs[78].category = "COMB";
        defs[78].bankIndex = 2;
        defs[78].activeStages = 5;
        defs[78].isPlanar4 = false;
        defs[78].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[78].corners[c].sections[s].pole_rho = rh;
                defs[78].corners[c].sections[s].pole_theta = th;
                defs[78].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[78].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [080] 080 Wide Comb 2
        defs[79].name = "080 Wide Comb 2";
        defs[79].category = "COMB";
        defs[79].bankIndex = 2;
        defs[79].activeStages = 5;
        defs[79].isPlanar4 = false;
        defs[79].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[79].corners[c].sections[s].pole_rho = rh;
                defs[79].corners[c].sections[s].pole_theta = th;
                defs[79].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[79].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [081] 081 Narrow Resonator
        defs[80].name = "081 Narrow Resonator";
        defs[80].category = "COMB";
        defs[80].bankIndex = 2;
        defs[80].activeStages = 5;
        defs[80].isPlanar4 = false;
        defs[80].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[80].corners[c].sections[s].pole_rho = rh;
                defs[80].corners[c].sections[s].pole_theta = th;
                defs[80].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[80].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [082] 082 Flange Comb
        defs[81].name = "082 Flange Comb";
        defs[81].category = "COMB";
        defs[81].bankIndex = 2;
        defs[81].activeStages = 5;
        defs[81].isPlanar4 = false;
        defs[81].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[81].corners[c].sections[s].pole_rho = rh;
                defs[81].corners[c].sections[s].pole_theta = th;
                defs[81].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[81].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [083] 083 Odd Harmonics
        defs[82].name = "083 Odd Harmonics";
        defs[82].category = "COMB";
        defs[82].bankIndex = 2;
        defs[82].activeStages = 5;
        defs[82].isPlanar4 = false;
        defs[82].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[82].corners[c].sections[s].pole_rho = rh;
                defs[82].corners[c].sections[s].pole_theta = th;
                defs[82].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[82].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [084] 084 Even Harmonics
        defs[83].name = "084 Even Harmonics";
        defs[83].category = "COMB";
        defs[83].bankIndex = 2;
        defs[83].activeStages = 5;
        defs[83].isPlanar4 = false;
        defs[83].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[83].corners[c].sections[s].pole_rho = rh;
                defs[83].corners[c].sections[s].pole_theta = th;
                defs[83].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[83].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [085] 085 Slap Comb
        defs[84].name = "085 Slap Comb";
        defs[84].category = "COMB";
        defs[84].bankIndex = 2;
        defs[84].activeStages = 5;
        defs[84].isPlanar4 = false;
        defs[84].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[84].corners[c].sections[s].pole_rho = rh;
                defs[84].corners[c].sections[s].pole_theta = th;
                defs[84].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[84].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [086] 086 Stereo Comb L/R
        defs[85].name = "086 Stereo Comb L/R";
        defs[85].category = "COMB";
        defs[85].bankIndex = 2;
        defs[85].activeStages = 5;
        defs[85].isPlanar4 = false;
        defs[85].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[85].corners[c].sections[s].pole_rho = rh;
                defs[85].corners[c].sections[s].pole_theta = th;
                defs[85].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[85].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [087] 087 Phased Comb
        defs[86].name = "087 Phased Comb";
        defs[86].category = "COMB";
        defs[86].bankIndex = 2;
        defs[86].activeStages = 5;
        defs[86].isPlanar4 = false;
        defs[86].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[86].corners[c].sections[s].pole_rho = rh;
                defs[86].corners[c].sections[s].pole_theta = th;
                defs[86].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[86].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [088] 088 Decay Combs
        defs[87].name = "088 Decay Combs";
        defs[87].category = "COMB";
        defs[87].bankIndex = 2;
        defs[87].activeStages = 5;
        defs[87].isPlanar4 = false;
        defs[87].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[87].corners[c].sections[s].pole_rho = rh;
                defs[87].corners[c].sections[s].pole_theta = th;
                defs[87].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[87].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [089] 089 Flutter Comb
        defs[88].name = "089 Flutter Comb";
        defs[88].category = "COMB";
        defs[88].bankIndex = 2;
        defs[88].activeStages = 5;
        defs[88].isPlanar4 = false;
        defs[88].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[88].corners[c].sections[s].pole_rho = rh;
                defs[88].corners[c].sections[s].pole_theta = th;
                defs[88].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[88].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [090] 090 Spring Cluster
        defs[89].name = "090 Spring Cluster";
        defs[89].category = "COMB";
        defs[89].bankIndex = 2;
        defs[89].activeStages = 5;
        defs[89].isPlanar4 = false;
        defs[89].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[89].corners[c].sections[s].pole_rho = rh;
                defs[89].corners[c].sections[s].pole_theta = th;
                defs[89].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[89].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [091] 091 Metallic Comb
        defs[90].name = "091 Metallic Comb";
        defs[90].category = "COMB";
        defs[90].bankIndex = 2;
        defs[90].activeStages = 5;
        defs[90].isPlanar4 = false;
        defs[90].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[90].corners[c].sections[s].pole_rho = rh;
                defs[90].corners[c].sections[s].pole_theta = th;
                defs[90].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[90].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [092] 092 Resonant Lattice
        defs[91].name = "092 Resonant Lattice";
        defs[91].category = "COMB";
        defs[91].bankIndex = 2;
        defs[91].activeStages = 5;
        defs[91].isPlanar4 = false;
        defs[91].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[91].corners[c].sections[s].pole_rho = rh;
                defs[91].corners[c].sections[s].pole_theta = th;
                defs[91].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[91].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [093] 093 Comb Cascade
        defs[92].name = "093 Comb Cascade";
        defs[92].category = "COMB";
        defs[92].bankIndex = 2;
        defs[92].activeStages = 5;
        defs[92].isPlanar4 = false;
        defs[92].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[92].corners[c].sections[s].pole_rho = rh;
                defs[92].corners[c].sections[s].pole_theta = th;
                defs[92].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[92].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [094] 094 Diffused Comb
        defs[93].name = "094 Diffused Comb";
        defs[93].category = "COMB";
        defs[93].bankIndex = 2;
        defs[93].activeStages = 5;
        defs[93].isPlanar4 = false;
        defs[93].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[93].corners[c].sections[s].pole_rho = rh;
                defs[93].corners[c].sections[s].pole_theta = th;
                defs[93].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[93].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [095] 095 Echo Resonator
        defs[94].name = "095 Echo Resonator";
        defs[94].category = "COMB";
        defs[94].bankIndex = 2;
        defs[94].activeStages = 5;
        defs[94].isPlanar4 = false;
        defs[94].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[94].corners[c].sections[s].pole_rho = rh;
                defs[94].corners[c].sections[s].pole_theta = th;
                defs[94].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[94].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [096] 096 Tuned Combs
        defs[95].name = "096 Tuned Combs";
        defs[95].category = "COMB";
        defs[95].bankIndex = 2;
        defs[95].activeStages = 5;
        defs[95].isPlanar4 = false;
        defs[95].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[95].corners[c].sections[s].pole_rho = rh;
                defs[95].corners[c].sections[s].pole_theta = th;
                defs[95].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[95].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [097] 097 Inharmonic Comb
        defs[96].name = "097 Inharmonic Comb";
        defs[96].category = "COMB";
        defs[96].bankIndex = 2;
        defs[96].activeStages = 5;
        defs[96].isPlanar4 = false;
        defs[96].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[96].corners[c].sections[s].pole_rho = rh;
                defs[96].corners[c].sections[s].pole_theta = th;
                defs[96].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[96].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [098] 098 Prime Comb 1
        defs[97].name = "098 Prime Comb 1";
        defs[97].category = "COMB";
        defs[97].bankIndex = 2;
        defs[97].activeStages = 5;
        defs[97].isPlanar4 = false;
        defs[97].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[97].corners[c].sections[s].pole_rho = rh;
                defs[97].corners[c].sections[s].pole_theta = th;
                defs[97].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[97].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [099] 099 Prime Comb 2
        defs[98].name = "099 Prime Comb 2";
        defs[98].category = "COMB";
        defs[98].bankIndex = 2;
        defs[98].activeStages = 5;
        defs[98].isPlanar4 = false;
        defs[98].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[98].corners[c].sections[s].pole_rho = rh;
                defs[98].corners[c].sections[s].pole_theta = th;
                defs[98].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[98].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [100] 100 Space Comb
        defs[99].name = "100 Space Comb";
        defs[99].category = "COMB";
        defs[99].bankIndex = 2;
        defs[99].activeStages = 5;
        defs[99].isPlanar4 = false;
        defs[99].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[99].corners[c].sections[s].pole_rho = rh;
                defs[99].corners[c].sections[s].pole_theta = th;
                defs[99].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[99].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [101] 101 Prismatic Comb
        defs[100].name = "101 Prismatic Comb";
        defs[100].category = "COMB";
        defs[100].bankIndex = 2;
        defs[100].activeStages = 5;
        defs[100].isPlanar4 = false;
        defs[100].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[100].corners[c].sections[s].pole_rho = rh;
                defs[100].corners[c].sections[s].pole_theta = th;
                defs[100].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[100].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [102] 102 Shimmer Comb
        defs[101].name = "102 Shimmer Comb";
        defs[101].category = "COMB";
        defs[101].bankIndex = 2;
        defs[101].activeStages = 5;
        defs[101].isPlanar4 = false;
        defs[101].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[101].corners[c].sections[s].pole_rho = rh;
                defs[101].corners[c].sections[s].pole_theta = th;
                defs[101].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[101].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [103] 103 Feedback Comb
        defs[102].name = "103 Feedback Comb";
        defs[102].category = "COMB";
        defs[102].bankIndex = 2;
        defs[102].activeStages = 5;
        defs[102].isPlanar4 = false;
        defs[102].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[102].corners[c].sections[s].pole_rho = rh;
                defs[102].corners[c].sections[s].pole_theta = th;
                defs[102].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[102].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [104] 104 Granular Comb
        defs[103].name = "104 Granular Comb";
        defs[103].category = "COMB";
        defs[103].bankIndex = 2;
        defs[103].activeStages = 5;
        defs[103].isPlanar4 = false;
        defs[103].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[103].corners[c].sections[s].pole_rho = rh;
                defs[103].corners[c].sections[s].pole_theta = th;
                defs[103].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[103].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [105] 105 Tension Comb
        defs[104].name = "105 Tension Comb";
        defs[104].category = "COMB";
        defs[104].bankIndex = 2;
        defs[104].activeStages = 5;
        defs[104].isPlanar4 = false;
        defs[104].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[104].corners[c].sections[s].pole_rho = rh;
                defs[104].corners[c].sections[s].pole_theta = th;
                defs[104].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[104].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [106] 106 Stator Comb
        defs[105].name = "106 Stator Comb";
        defs[105].category = "COMB";
        defs[105].bankIndex = 2;
        defs[105].activeStages = 5;
        defs[105].isPlanar4 = false;
        defs[105].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[105].corners[c].sections[s].pole_rho = rh;
                defs[105].corners[c].sections[s].pole_theta = th;
                defs[105].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[105].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [107] 107 Hyper Comb
        defs[106].name = "107 Hyper Comb";
        defs[106].category = "COMB";
        defs[106].bankIndex = 2;
        defs[106].activeStages = 5;
        defs[106].isPlanar4 = false;
        defs[106].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[106].corners[c].sections[s].pole_rho = rh;
                defs[106].corners[c].sections[s].pole_theta = th;
                defs[106].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[106].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [108] 108 Infinite Comb
        defs[107].name = "108 Infinite Comb";
        defs[107].category = "COMB";
        defs[107].bankIndex = 2;
        defs[107].activeStages = 5;
        defs[107].isPlanar4 = false;
        defs[107].modelNorm = 0.32f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[107].corners[c].sections[s].pole_rho = rh;
                defs[107].corners[c].sections[s].pole_theta = th;
                defs[107].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[107].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [109] 109 6-Pole Notch
        defs[108].name = "109 6-Pole Notch";
        defs[108].category = "PHASER";
        defs[108].bankIndex = 3;
        defs[108].activeStages = 6;
        defs[108].isPlanar4 = false;
        defs[108].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[108].corners[c].sections[s].pole_rho = rh;
                defs[108].corners[c].sections[s].pole_theta = th;
                defs[108].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[108].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [110] 110 10-Pole Notch
        defs[109].name = "110 10-Pole Notch";
        defs[109].category = "PHASER";
        defs[109].bankIndex = 3;
        defs[109].activeStages = 6;
        defs[109].isPlanar4 = false;
        defs[109].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[109].corners[c].sections[s].pole_rho = rh;
                defs[109].corners[c].sections[s].pole_theta = th;
                defs[109].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[109].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [111] 111 14-Pole Phaser
        defs[110].name = "111 14-Pole Phaser";
        defs[110].category = "PHASER";
        defs[110].bankIndex = 3;
        defs[110].activeStages = 6;
        defs[110].isPlanar4 = false;
        defs[110].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[110].corners[c].sections[s].pole_rho = rh;
                defs[110].corners[c].sections[s].pole_theta = th;
                defs[110].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[110].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [112] 112 Jet Flanger
        defs[111].name = "112 Jet Flanger";
        defs[111].category = "PHASER";
        defs[111].bankIndex = 3;
        defs[111].activeStages = 6;
        defs[111].isPlanar4 = false;
        defs[111].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[111].corners[c].sections[s].pole_rho = rh;
                defs[111].corners[c].sections[s].pole_theta = th;
                defs[111].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[111].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [113] 113 Dual Sweeper
        defs[112].name = "113 Dual Sweeper";
        defs[112].category = "PHASER";
        defs[112].bankIndex = 3;
        defs[112].activeStages = 6;
        defs[112].isPlanar4 = false;
        defs[112].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[112].corners[c].sections[s].pole_rho = rh;
                defs[112].corners[c].sections[s].pole_theta = th;
                defs[112].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[112].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [114] 114 Barberpole Phase
        defs[113].name = "114 Barberpole Phase";
        defs[113].category = "PHASER";
        defs[113].bankIndex = 3;
        defs[113].activeStages = 6;
        defs[113].isPlanar4 = false;
        defs[113].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[113].corners[c].sections[s].pole_rho = rh;
                defs[113].corners[c].sections[s].pole_theta = th;
                defs[113].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[113].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [115] 115 Asym Notch 1
        defs[114].name = "115 Asym Notch 1";
        defs[114].category = "PHASER";
        defs[114].bankIndex = 3;
        defs[114].activeStages = 6;
        defs[114].isPlanar4 = false;
        defs[114].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[114].corners[c].sections[s].pole_rho = rh;
                defs[114].corners[c].sections[s].pole_theta = th;
                defs[114].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[114].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [116] 116 Asym Notch 2
        defs[115].name = "116 Asym Notch 2";
        defs[115].category = "PHASER";
        defs[115].bankIndex = 3;
        defs[115].activeStages = 6;
        defs[115].isPlanar4 = false;
        defs[115].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[115].corners[c].sections[s].pole_rho = rh;
                defs[115].corners[c].sections[s].pole_theta = th;
                defs[115].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[115].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [117] 117 Resonant Notch
        defs[116].name = "117 Resonant Notch";
        defs[116].category = "PHASER";
        defs[116].bankIndex = 3;
        defs[116].activeStages = 6;
        defs[116].isPlanar4 = false;
        defs[116].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[116].corners[c].sections[s].pole_rho = rh;
                defs[116].corners[c].sections[s].pole_theta = th;
                defs[116].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[116].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [118] 118 Cascade Flange
        defs[117].name = "118 Cascade Flange";
        defs[117].category = "PHASER";
        defs[117].bankIndex = 3;
        defs[117].activeStages = 6;
        defs[117].isPlanar4 = false;
        defs[117].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[117].corners[c].sections[s].pole_rho = rh;
                defs[117].corners[c].sections[s].pole_theta = th;
                defs[117].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[117].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [119] 119 Deep Swirl
        defs[118].name = "119 Deep Swirl";
        defs[118].category = "PHASER";
        defs[118].bankIndex = 3;
        defs[118].activeStages = 6;
        defs[118].isPlanar4 = false;
        defs[118].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[118].corners[c].sections[s].pole_rho = rh;
                defs[118].corners[c].sections[s].pole_theta = th;
                defs[118].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[118].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [120] 120 Liquid Phase
        defs[119].name = "120 Liquid Phase";
        defs[119].category = "PHASER";
        defs[119].bankIndex = 3;
        defs[119].activeStages = 6;
        defs[119].isPlanar4 = false;
        defs[119].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[119].corners[c].sections[s].pole_rho = rh;
                defs[119].corners[c].sections[s].pole_theta = th;
                defs[119].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[119].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [121] 121 Vapor Flange
        defs[120].name = "121 Vapor Flange";
        defs[120].category = "PHASER";
        defs[120].bankIndex = 3;
        defs[120].activeStages = 6;
        defs[120].isPlanar4 = false;
        defs[120].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[120].corners[c].sections[s].pole_rho = rh;
                defs[120].corners[c].sections[s].pole_theta = th;
                defs[120].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[120].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [122] 122 Stereo Phaser
        defs[121].name = "122 Stereo Phaser";
        defs[121].category = "PHASER";
        defs[121].bankIndex = 3;
        defs[121].activeStages = 6;
        defs[121].isPlanar4 = false;
        defs[121].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[121].corners[c].sections[s].pole_rho = rh;
                defs[121].corners[c].sections[s].pole_theta = th;
                defs[121].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[121].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [123] 123 Staggered Notch
        defs[122].name = "123 Staggered Notch";
        defs[122].category = "PHASER";
        defs[122].bankIndex = 3;
        defs[122].activeStages = 6;
        defs[122].isPlanar4 = false;
        defs[122].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[122].corners[c].sections[s].pole_rho = rh;
                defs[122].corners[c].sections[s].pole_theta = th;
                defs[122].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[122].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [124] 124 Twin Peaks
        defs[123].name = "124 Twin Peaks";
        defs[123].category = "PHASER";
        defs[123].bankIndex = 3;
        defs[123].activeStages = 6;
        defs[123].isPlanar4 = false;
        defs[123].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[123].corners[c].sections[s].pole_rho = rh;
                defs[123].corners[c].sections[s].pole_theta = th;
                defs[123].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[123].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [125] 125 Bipolar Phase
        defs[124].name = "125 Bipolar Phase";
        defs[124].category = "PHASER";
        defs[124].bankIndex = 3;
        defs[124].activeStages = 6;
        defs[124].isPlanar4 = false;
        defs[124].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[124].corners[c].sections[s].pole_rho = rh;
                defs[124].corners[c].sections[s].pole_theta = th;
                defs[124].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[124].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [126] 126 Comb Flange
        defs[125].name = "126 Comb Flange";
        defs[125].category = "PHASER";
        defs[125].bankIndex = 3;
        defs[125].activeStages = 6;
        defs[125].isPlanar4 = false;
        defs[125].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[125].corners[c].sections[s].pole_rho = rh;
                defs[125].corners[c].sections[s].pole_theta = th;
                defs[125].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[125].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [127] 127 Vintage Phase
        defs[126].name = "127 Vintage Phase";
        defs[126].category = "PHASER";
        defs[126].bankIndex = 3;
        defs[126].activeStages = 6;
        defs[126].isPlanar4 = false;
        defs[126].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[126].corners[c].sections[s].pole_rho = rh;
                defs[126].corners[c].sections[s].pole_theta = th;
                defs[126].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[126].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [128] 128 Optical Phaser
        defs[127].name = "128 Optical Phaser";
        defs[127].category = "PHASER";
        defs[127].bankIndex = 3;
        defs[127].activeStages = 6;
        defs[127].isPlanar4 = false;
        defs[127].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[127].corners[c].sections[s].pole_rho = rh;
                defs[127].corners[c].sections[s].pole_theta = th;
                defs[127].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[127].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [129] 129 Rotary Flange
        defs[128].name = "129 Rotary Flange";
        defs[128].category = "PHASER";
        defs[128].bankIndex = 3;
        defs[128].activeStages = 6;
        defs[128].isPlanar4 = false;
        defs[128].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[128].corners[c].sections[s].pole_rho = rh;
                defs[128].corners[c].sections[s].pole_theta = th;
                defs[128].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[128].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [130] 130 Chorus Phase
        defs[129].name = "130 Chorus Phase";
        defs[129].category = "PHASER";
        defs[129].bankIndex = 3;
        defs[129].activeStages = 6;
        defs[129].isPlanar4 = false;
        defs[129].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[129].corners[c].sections[s].pole_rho = rh;
                defs[129].corners[c].sections[s].pole_theta = th;
                defs[129].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[129].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [131] 131 Sky Sweep
        defs[130].name = "131 Sky Sweep";
        defs[130].category = "PHASER";
        defs[130].bankIndex = 3;
        defs[130].activeStages = 6;
        defs[130].isPlanar4 = false;
        defs[130].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[130].corners[c].sections[s].pole_rho = rh;
                defs[130].corners[c].sections[s].pole_theta = th;
                defs[130].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[130].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [132] 132 Zero Flange
        defs[131].name = "132 Zero Flange";
        defs[131].category = "PHASER";
        defs[131].bankIndex = 3;
        defs[131].activeStages = 6;
        defs[131].isPlanar4 = false;
        defs[131].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[131].corners[c].sections[s].pole_rho = rh;
                defs[131].corners[c].sections[s].pole_theta = th;
                defs[131].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[131].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [133] 133 Ring Phase
        defs[132].name = "133 Ring Phase";
        defs[132].category = "PHASER";
        defs[132].bankIndex = 3;
        defs[132].activeStages = 6;
        defs[132].isPlanar4 = false;
        defs[132].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[132].corners[c].sections[s].pole_rho = rh;
                defs[132].corners[c].sections[s].pole_theta = th;
                defs[132].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[132].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [134] 134 Turbine Flange
        defs[133].name = "134 Turbine Flange";
        defs[133].category = "PHASER";
        defs[133].bankIndex = 3;
        defs[133].activeStages = 6;
        defs[133].isPlanar4 = false;
        defs[133].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[133].corners[c].sections[s].pole_rho = rh;
                defs[133].corners[c].sections[s].pole_theta = th;
                defs[133].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[133].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [135] 135 Puddle Phase
        defs[134].name = "135 Puddle Phase";
        defs[134].category = "PHASER";
        defs[134].bankIndex = 3;
        defs[134].activeStages = 6;
        defs[134].isPlanar4 = false;
        defs[134].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[134].corners[c].sections[s].pole_rho = rh;
                defs[134].corners[c].sections[s].pole_theta = th;
                defs[134].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[134].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [136] 136 Mirror Notch
        defs[135].name = "136 Mirror Notch";
        defs[135].category = "PHASER";
        defs[135].bankIndex = 3;
        defs[135].activeStages = 6;
        defs[135].isPlanar4 = false;
        defs[135].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[135].corners[c].sections[s].pole_rho = rh;
                defs[135].corners[c].sections[s].pole_theta = th;
                defs[135].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[135].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [137] 137 Laser Sweep
        defs[136].name = "137 Laser Sweep";
        defs[136].category = "PHASER";
        defs[136].bankIndex = 3;
        defs[136].activeStages = 6;
        defs[136].isPlanar4 = false;
        defs[136].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[136].corners[c].sections[s].pole_rho = rh;
                defs[136].corners[c].sections[s].pole_theta = th;
                defs[136].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[136].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [138] 138 Phase Matrix
        defs[137].name = "138 Phase Matrix";
        defs[137].category = "PHASER";
        defs[137].bankIndex = 3;
        defs[137].activeStages = 6;
        defs[137].isPlanar4 = false;
        defs[137].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[137].corners[c].sections[s].pole_rho = rh;
                defs[137].corners[c].sections[s].pole_theta = th;
                defs[137].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[137].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [139] 139 Wide Swirl
        defs[138].name = "139 Wide Swirl";
        defs[138].category = "PHASER";
        defs[138].bankIndex = 3;
        defs[138].activeStages = 6;
        defs[138].isPlanar4 = false;
        defs[138].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[138].corners[c].sections[s].pole_rho = rh;
                defs[138].corners[c].sections[s].pole_theta = th;
                defs[138].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[138].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [140] 140 Multi-Stage Phase
        defs[139].name = "140 Multi-Stage Phase";
        defs[139].category = "PHASER";
        defs[139].bankIndex = 3;
        defs[139].activeStages = 6;
        defs[139].isPlanar4 = false;
        defs[139].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[139].corners[c].sections[s].pole_rho = rh;
                defs[139].corners[c].sections[s].pole_theta = th;
                defs[139].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[139].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [141] 141 Tape Flanger
        defs[140].name = "141 Tape Flanger";
        defs[140].category = "PHASER";
        defs[140].bankIndex = 3;
        defs[140].activeStages = 6;
        defs[140].isPlanar4 = false;
        defs[140].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[140].corners[c].sections[s].pole_rho = rh;
                defs[140].corners[c].sections[s].pole_theta = th;
                defs[140].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[140].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [142] 142 Bi-Phase
        defs[141].name = "142 Bi-Phase";
        defs[141].category = "PHASER";
        defs[141].bankIndex = 3;
        defs[141].activeStages = 6;
        defs[141].isPlanar4 = false;
        defs[141].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[141].corners[c].sections[s].pole_rho = rh;
                defs[141].corners[c].sections[s].pole_theta = th;
                defs[141].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[141].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [143] 143 Quantum Notch
        defs[142].name = "143 Quantum Notch";
        defs[142].category = "PHASER";
        defs[142].bankIndex = 3;
        defs[142].activeStages = 6;
        defs[142].isPlanar4 = false;
        defs[142].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[142].corners[c].sections[s].pole_rho = rh;
                defs[142].corners[c].sections[s].pole_theta = th;
                defs[142].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[142].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [144] 144 Vortex Sweep
        defs[143].name = "144 Vortex Sweep";
        defs[143].category = "PHASER";
        defs[143].bankIndex = 3;
        defs[143].activeStages = 6;
        defs[143].isPlanar4 = false;
        defs[143].modelNorm = 0.35f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[143].corners[c].sections[s].pole_rho = rh;
                defs[143].corners[c].sections[s].pole_theta = th;
                defs[143].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[143].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [145] 145 LP12 Smooth
        defs[144].name = "145 LP12 Smooth";
        defs[144].category = "CLASSIC";
        defs[144].bankIndex = 4;
        defs[144].activeStages = 2;
        defs[144].isPlanar4 = true;
        defs[144].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[144].corners[c].sections[s].pole_rho = rh;
                defs[144].corners[c].sections[s].pole_theta = th;
                defs[144].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[144].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [146] 146 LP24 Classic
        defs[145].name = "146 LP24 Classic";
        defs[145].category = "CLASSIC";
        defs[145].bankIndex = 4;
        defs[145].activeStages = 4;
        defs[145].isPlanar4 = true;
        defs[145].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[145].corners[c].sections[s].pole_rho = rh;
                defs[145].corners[c].sections[s].pole_theta = th;
                defs[145].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[145].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [147] 147 LP36 Steeper
        defs[146].name = "147 LP36 Steeper";
        defs[146].category = "CLASSIC";
        defs[146].bankIndex = 4;
        defs[146].activeStages = 6;
        defs[146].isPlanar4 = true;
        defs[146].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[146].corners[c].sections[s].pole_rho = rh;
                defs[146].corners[c].sections[s].pole_theta = th;
                defs[146].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[146].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [148] 148 HP12 Shallow
        defs[147].name = "148 HP12 Shallow";
        defs[147].category = "CLASSIC";
        defs[147].bankIndex = 4;
        defs[147].activeStages = 2;
        defs[147].isPlanar4 = true;
        defs[147].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[147].corners[c].sections[s].pole_rho = rh;
                defs[147].corners[c].sections[s].pole_theta = th;
                defs[147].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[147].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [149] 149 HP24 Deeper
        defs[148].name = "149 HP24 Deeper";
        defs[148].category = "CLASSIC";
        defs[148].bankIndex = 4;
        defs[148].activeStages = 4;
        defs[148].isPlanar4 = true;
        defs[148].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[148].corners[c].sections[s].pole_rho = rh;
                defs[148].corners[c].sections[s].pole_theta = th;
                defs[148].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[148].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [150] 150 BPF12 Wide
        defs[149].name = "150 BPF12 Wide";
        defs[149].category = "CLASSIC";
        defs[149].bankIndex = 4;
        defs[149].activeStages = 2;
        defs[149].isPlanar4 = true;
        defs[149].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[149].corners[c].sections[s].pole_rho = rh;
                defs[149].corners[c].sections[s].pole_theta = th;
                defs[149].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[149].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [151] 151 BPF24 Narrow
        defs[150].name = "151 BPF24 Narrow";
        defs[150].category = "CLASSIC";
        defs[150].bankIndex = 4;
        defs[150].activeStages = 4;
        defs[150].isPlanar4 = true;
        defs[150].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[150].corners[c].sections[s].pole_rho = rh;
                defs[150].corners[c].sections[s].pole_theta = th;
                defs[150].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[150].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [152] 152 Acid Squeeze
        defs[151].name = "152 Acid Squeeze";
        defs[151].category = "CLASSIC";
        defs[151].bankIndex = 4;
        defs[151].activeStages = 4;
        defs[151].isPlanar4 = true;
        defs[151].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[151].corners[c].sections[s].pole_rho = rh;
                defs[151].corners[c].sections[s].pole_theta = th;
                defs[151].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[151].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [153] 153 Moog 4-Pole
        defs[152].name = "153 Moog 4-Pole";
        defs[152].category = "CLASSIC";
        defs[152].bankIndex = 4;
        defs[152].activeStages = 4;
        defs[152].isPlanar4 = true;
        defs[152].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[152].corners[c].sections[s].pole_rho = rh;
                defs[152].corners[c].sections[s].pole_theta = th;
                defs[152].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[152].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [154] 154 Oberheim SEM
        defs[153].name = "154 Oberheim SEM";
        defs[153].category = "CLASSIC";
        defs[153].bankIndex = 4;
        defs[153].activeStages = 4;
        defs[153].isPlanar4 = true;
        defs[153].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[153].corners[c].sections[s].pole_rho = rh;
                defs[153].corners[c].sections[s].pole_theta = th;
                defs[153].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[153].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [155] 155 TB-303 Acid
        defs[154].name = "155 TB-303 Acid";
        defs[154].category = "CLASSIC";
        defs[154].bankIndex = 4;
        defs[154].activeStages = 4;
        defs[154].isPlanar4 = true;
        defs[154].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[154].corners[c].sections[s].pole_rho = rh;
                defs[154].corners[c].sections[s].pole_theta = th;
                defs[154].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[154].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [156] 156 MS-20 Korg
        defs[155].name = "156 MS-20 Korg";
        defs[155].category = "CLASSIC";
        defs[155].bankIndex = 4;
        defs[155].activeStages = 4;
        defs[155].isPlanar4 = true;
        defs[155].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[155].corners[c].sections[s].pole_rho = rh;
                defs[155].corners[c].sections[s].pole_theta = th;
                defs[155].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[155].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [157] 157 Prophet Rev2
        defs[156].name = "157 Prophet Rev2";
        defs[156].category = "CLASSIC";
        defs[156].bankIndex = 4;
        defs[156].activeStages = 4;
        defs[156].isPlanar4 = true;
        defs[156].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[156].corners[c].sections[s].pole_rho = rh;
                defs[156].corners[c].sections[s].pole_theta = th;
                defs[156].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[156].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [158] 158 Juno 106
        defs[157].name = "158 Juno 106";
        defs[157].category = "CLASSIC";
        defs[157].bankIndex = 4;
        defs[157].activeStages = 4;
        defs[157].isPlanar4 = true;
        defs[157].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[157].corners[c].sections[s].pole_rho = rh;
                defs[157].corners[c].sections[s].pole_theta = th;
                defs[157].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[157].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [159] 159 JP-8000 Res
        defs[158].name = "159 JP-8000 Res";
        defs[158].category = "CLASSIC";
        defs[158].bankIndex = 4;
        defs[158].activeStages = 4;
        defs[158].isPlanar4 = true;
        defs[158].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[158].corners[c].sections[s].pole_rho = rh;
                defs[158].corners[c].sections[s].pole_theta = th;
                defs[158].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[158].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [160] 160 Notch 24dB
        defs[159].name = "160 Notch 24dB";
        defs[159].category = "CLASSIC";
        defs[159].bankIndex = 4;
        defs[159].activeStages = 4;
        defs[159].isPlanar4 = true;
        defs[159].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[159].corners[c].sections[s].pole_rho = rh;
                defs[159].corners[c].sections[s].pole_theta = th;
                defs[159].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[159].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [161] 161 Band Reject
        defs[160].name = "161 Band Reject";
        defs[160].category = "CLASSIC";
        defs[160].bankIndex = 4;
        defs[160].activeStages = 4;
        defs[160].isPlanar4 = true;
        defs[160].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[160].corners[c].sections[s].pole_rho = rh;
                defs[160].corners[c].sections[s].pole_theta = th;
                defs[160].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[160].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [162] 162 Allpass Pole
        defs[161].name = "162 Allpass Pole";
        defs[161].category = "CLASSIC";
        defs[161].bankIndex = 4;
        defs[161].activeStages = 4;
        defs[161].isPlanar4 = true;
        defs[161].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[161].corners[c].sections[s].pole_rho = rh;
                defs[161].corners[c].sections[s].pole_theta = th;
                defs[161].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[161].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [163] 163 Peak Shifter
        defs[162].name = "163 Peak Shifter";
        defs[162].category = "CLASSIC";
        defs[162].bankIndex = 4;
        defs[162].activeStages = 4;
        defs[162].isPlanar4 = true;
        defs[162].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[162].corners[c].sections[s].pole_rho = rh;
                defs[162].corners[c].sections[s].pole_theta = th;
                defs[162].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[162].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [164] 164 Tilt Filter
        defs[163].name = "164 Tilt Filter";
        defs[163].category = "CLASSIC";
        defs[163].bankIndex = 4;
        defs[163].activeStages = 4;
        defs[163].isPlanar4 = true;
        defs[163].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[163].corners[c].sections[s].pole_rho = rh;
                defs[163].corners[c].sections[s].pole_theta = th;
                defs[163].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[163].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [165] 165 Dual BPF
        defs[164].name = "165 Dual BPF";
        defs[164].category = "CLASSIC";
        defs[164].bankIndex = 4;
        defs[164].activeStages = 4;
        defs[164].isPlanar4 = true;
        defs[164].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[164].corners[c].sections[s].pole_rho = rh;
                defs[164].corners[c].sections[s].pole_theta = th;
                defs[164].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[164].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [166] 166 Notch Peak
        defs[165].name = "166 Notch Peak";
        defs[165].category = "CLASSIC";
        defs[165].bankIndex = 4;
        defs[165].activeStages = 4;
        defs[165].isPlanar4 = true;
        defs[165].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[165].corners[c].sections[s].pole_rho = rh;
                defs[165].corners[c].sections[s].pole_theta = th;
                defs[165].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[165].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [167] 167 Shelf Morph
        defs[166].name = "167 Shelf Morph";
        defs[166].category = "CLASSIC";
        defs[166].bankIndex = 4;
        defs[166].activeStages = 4;
        defs[166].isPlanar4 = true;
        defs[166].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[166].corners[c].sections[s].pole_rho = rh;
                defs[166].corners[c].sections[s].pole_theta = th;
                defs[166].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[166].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [168] 168 Butterworth 6
        defs[167].name = "168 Butterworth 6";
        defs[167].category = "CLASSIC";
        defs[167].bankIndex = 4;
        defs[167].activeStages = 4;
        defs[167].isPlanar4 = true;
        defs[167].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[167].corners[c].sections[s].pole_rho = rh;
                defs[167].corners[c].sections[s].pole_theta = th;
                defs[167].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[167].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [169] 169 Chebyshev 4
        defs[168].name = "169 Chebyshev 4";
        defs[168].category = "CLASSIC";
        defs[168].bankIndex = 4;
        defs[168].activeStages = 4;
        defs[168].isPlanar4 = true;
        defs[168].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[168].corners[c].sections[s].pole_rho = rh;
                defs[168].corners[c].sections[s].pole_theta = th;
                defs[168].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[168].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [170] 170 Elliptic Band
        defs[169].name = "170 Elliptic Band";
        defs[169].category = "CLASSIC";
        defs[169].bankIndex = 4;
        defs[169].activeStages = 4;
        defs[169].isPlanar4 = true;
        defs[169].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[169].corners[c].sections[s].pole_rho = rh;
                defs[169].corners[c].sections[s].pole_theta = th;
                defs[169].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[169].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [171] 171 Bessel Lowpass
        defs[170].name = "171 Bessel Lowpass";
        defs[170].category = "CLASSIC";
        defs[170].bankIndex = 4;
        defs[170].activeStages = 4;
        defs[170].isPlanar4 = true;
        defs[170].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[170].corners[c].sections[s].pole_rho = rh;
                defs[170].corners[c].sections[s].pole_theta = th;
                defs[170].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[170].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [172] 172 State Variable
        defs[171].name = "172 State Variable";
        defs[171].category = "CLASSIC";
        defs[171].bankIndex = 4;
        defs[171].activeStages = 4;
        defs[171].isPlanar4 = true;
        defs[171].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[171].corners[c].sections[s].pole_rho = rh;
                defs[171].corners[c].sections[s].pole_theta = th;
                defs[171].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[171].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [173] 173 Diode Ladder
        defs[172].name = "173 Diode Ladder";
        defs[172].category = "CLASSIC";
        defs[172].bankIndex = 4;
        defs[172].activeStages = 4;
        defs[172].isPlanar4 = true;
        defs[172].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[172].corners[c].sections[s].pole_rho = rh;
                defs[172].corners[c].sections[s].pole_theta = th;
                defs[172].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[172].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [174] 174 Steiner Parker
        defs[173].name = "174 Steiner Parker";
        defs[173].category = "CLASSIC";
        defs[173].bankIndex = 4;
        defs[173].activeStages = 4;
        defs[173].isPlanar4 = true;
        defs[173].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[173].corners[c].sections[s].pole_rho = rh;
                defs[173].corners[c].sections[s].pole_theta = th;
                defs[173].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[173].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [175] 175 Wasp Dual
        defs[174].name = "175 Wasp Dual";
        defs[174].category = "CLASSIC";
        defs[174].bankIndex = 4;
        defs[174].activeStages = 4;
        defs[174].isPlanar4 = true;
        defs[174].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[174].corners[c].sections[s].pole_rho = rh;
                defs[174].corners[c].sections[s].pole_theta = th;
                defs[174].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[174].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [176] 176 Polivoks Drive
        defs[175].name = "176 Polivoks Drive";
        defs[175].category = "CLASSIC";
        defs[175].bankIndex = 4;
        defs[175].activeStages = 4;
        defs[175].isPlanar4 = true;
        defs[175].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[175].corners[c].sections[s].pole_rho = rh;
                defs[175].corners[c].sections[s].pole_theta = th;
                defs[175].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[175].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [177] 177 Formant Peak
        defs[176].name = "177 Formant Peak";
        defs[176].category = "CLASSIC";
        defs[176].bankIndex = 4;
        defs[176].activeStages = 4;
        defs[176].isPlanar4 = true;
        defs[176].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[176].corners[c].sections[s].pole_rho = rh;
                defs[176].corners[c].sections[s].pole_theta = th;
                defs[176].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[176].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [178] 178 Reso Sweep
        defs[177].name = "178 Reso Sweep";
        defs[177].category = "CLASSIC";
        defs[177].bankIndex = 4;
        defs[177].activeStages = 4;
        defs[177].isPlanar4 = true;
        defs[177].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[177].corners[c].sections[s].pole_rho = rh;
                defs[177].corners[c].sections[s].pole_theta = th;
                defs[177].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[177].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [179] 179 Screaming 303
        defs[178].name = "179 Screaming 303";
        defs[178].category = "CLASSIC";
        defs[178].bankIndex = 4;
        defs[178].activeStages = 4;
        defs[178].isPlanar4 = true;
        defs[178].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[178].corners[c].sections[s].pole_rho = rh;
                defs[178].corners[c].sections[s].pole_theta = th;
                defs[178].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[178].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [180] 180 Analog Warmth
        defs[179].name = "180 Analog Warmth";
        defs[179].category = "CLASSIC";
        defs[179].bankIndex = 4;
        defs[179].activeStages = 4;
        defs[179].isPlanar4 = true;
        defs[179].modelNorm = 0.40f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[179].corners[c].sections[s].pole_rho = rh;
                defs[179].corners[c].sections[s].pole_theta = th;
                defs[179].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[179].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [181] 181 LP-to-Comb
        defs[180].name = "181 LP-to-Comb";
        defs[180].category = "MORPH";
        defs[180].bankIndex = 5;
        defs[180].activeStages = 7;
        defs[180].isPlanar4 = false;
        defs[180].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[180].corners[c].sections[s].pole_rho = rh;
                defs[180].corners[c].sections[s].pole_theta = th;
                defs[180].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[180].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [182] 182 HP-to-Phaser
        defs[181].name = "182 HP-to-Phaser";
        defs[181].category = "MORPH";
        defs[181].bankIndex = 5;
        defs[181].activeStages = 7;
        defs[181].isPlanar4 = false;
        defs[181].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[181].corners[c].sections[s].pole_rho = rh;
                defs[181].corners[c].sections[s].pole_theta = th;
                defs[181].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[181].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [183] 183 BP-to-Vocal
        defs[182].name = "183 BP-to-Vocal";
        defs[182].category = "MORPH";
        defs[182].bankIndex = 5;
        defs[182].activeStages = 7;
        defs[182].isPlanar4 = false;
        defs[182].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[182].corners[c].sections[s].pole_rho = rh;
                defs[182].corners[c].sections[s].pole_theta = th;
                defs[182].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[182].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [184] 184 Notch-to-Peak
        defs[183].name = "184 Notch-to-Peak";
        defs[183].category = "MORPH";
        defs[183].bankIndex = 5;
        defs[183].activeStages = 7;
        defs[183].isPlanar4 = false;
        defs[183].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[183].corners[c].sections[s].pole_rho = rh;
                defs[183].corners[c].sections[s].pole_theta = th;
                defs[183].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[183].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [185] 185 Acid-to-Bell
        defs[184].name = "185 Acid-to-Bell";
        defs[184].category = "MORPH";
        defs[184].bankIndex = 5;
        defs[184].activeStages = 7;
        defs[184].isPlanar4 = false;
        defs[184].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[184].corners[c].sections[s].pole_rho = rh;
                defs[184].corners[c].sections[s].pole_theta = th;
                defs[184].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[184].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [186] 186 Wood-to-Metal
        defs[185].name = "186 Wood-to-Metal";
        defs[185].category = "MORPH";
        defs[185].bankIndex = 5;
        defs[185].activeStages = 7;
        defs[185].isPlanar4 = false;
        defs[185].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[185].corners[c].sections[s].pole_rho = rh;
                defs[185].corners[c].sections[s].pole_theta = th;
                defs[185].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[185].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [187] 187 Vowel-to-Brass
        defs[186].name = "187 Vowel-to-Brass";
        defs[186].category = "MORPH";
        defs[186].bankIndex = 5;
        defs[186].activeStages = 7;
        defs[186].isPlanar4 = false;
        defs[186].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[186].corners[c].sections[s].pole_rho = rh;
                defs[186].corners[c].sections[s].pole_theta = th;
                defs[186].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[186].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [188] 188 Clean-to-Chaos
        defs[187].name = "188 Clean-to-Chaos";
        defs[187].category = "MORPH";
        defs[187].bankIndex = 5;
        defs[187].activeStages = 7;
        defs[187].isPlanar4 = false;
        defs[187].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[187].corners[c].sections[s].pole_rho = rh;
                defs[187].corners[c].sections[s].pole_theta = th;
                defs[187].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[187].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [189] 189 Smooth-to-Grit
        defs[188].name = "189 Smooth-to-Grit";
        defs[188].category = "MORPH";
        defs[188].bankIndex = 5;
        defs[188].activeStages = 7;
        defs[188].isPlanar4 = false;
        defs[188].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[188].corners[c].sections[s].pole_rho = rh;
                defs[188].corners[c].sections[s].pole_theta = th;
                defs[188].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[188].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [190] 190 2Pole-to-14Pole
        defs[189].name = "190 2Pole-to-14Pole";
        defs[189].category = "MORPH";
        defs[189].bankIndex = 5;
        defs[189].activeStages = 7;
        defs[189].isPlanar4 = false;
        defs[189].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[189].corners[c].sections[s].pole_rho = rh;
                defs[189].corners[c].sections[s].pole_theta = th;
                defs[189].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[189].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [191] 191 Sweep-to-Reso
        defs[190].name = "191 Sweep-to-Reso";
        defs[190].category = "MORPH";
        defs[190].bankIndex = 5;
        defs[190].activeStages = 7;
        defs[190].isPlanar4 = false;
        defs[190].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[190].corners[c].sections[s].pole_rho = rh;
                defs[190].corners[c].sections[s].pole_theta = th;
                defs[190].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[190].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [192] 192 Dual-to-Quad
        defs[191].name = "192 Dual-to-Quad";
        defs[191].category = "MORPH";
        defs[191].bankIndex = 5;
        defs[191].activeStages = 7;
        defs[191].isPlanar4 = false;
        defs[191].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[191].corners[c].sections[s].pole_rho = rh;
                defs[191].corners[c].sections[s].pole_theta = th;
                defs[191].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[191].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [193] 193 Linear-to-Exp
        defs[192].name = "193 Linear-to-Exp";
        defs[192].category = "MORPH";
        defs[192].bankIndex = 5;
        defs[192].activeStages = 7;
        defs[192].isPlanar4 = false;
        defs[192].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[192].corners[c].sections[s].pole_rho = rh;
                defs[192].corners[c].sections[s].pole_theta = th;
                defs[192].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[192].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [194] 194 Static-to-Ring
        defs[193].name = "194 Static-to-Ring";
        defs[193].category = "MORPH";
        defs[193].bankIndex = 5;
        defs[193].activeStages = 7;
        defs[193].isPlanar4 = false;
        defs[193].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[193].corners[c].sections[s].pole_rho = rh;
                defs[193].corners[c].sections[s].pole_theta = th;
                defs[193].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[193].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [195] 195 Plate-to-Pipe
        defs[194].name = "195 Plate-to-Pipe";
        defs[194].category = "MORPH";
        defs[194].bankIndex = 5;
        defs[194].activeStages = 7;
        defs[194].isPlanar4 = false;
        defs[194].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[194].corners[c].sections[s].pole_rho = rh;
                defs[194].corners[c].sections[s].pole_theta = th;
                defs[194].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[194].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [196] 196 Drum-to-Chime
        defs[195].name = "196 Drum-to-Chime";
        defs[195].category = "MORPH";
        defs[195].bankIndex = 5;
        defs[195].activeStages = 7;
        defs[195].isPlanar4 = false;
        defs[195].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[195].corners[c].sections[s].pole_rho = rh;
                defs[195].corners[c].sections[s].pole_theta = th;
                defs[195].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[195].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [197] 197 Soft-to-Edge
        defs[196].name = "197 Soft-to-Edge";
        defs[196].category = "MORPH";
        defs[196].bankIndex = 5;
        defs[196].activeStages = 7;
        defs[196].isPlanar4 = false;
        defs[196].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[196].corners[c].sections[s].pole_rho = rh;
                defs[196].corners[c].sections[s].pole_theta = th;
                defs[196].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[196].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [198] 198 Dark-to-Shimmer
        defs[197].name = "198 Dark-to-Shimmer";
        defs[197].category = "MORPH";
        defs[197].bankIndex = 5;
        defs[197].activeStages = 7;
        defs[197].isPlanar4 = false;
        defs[197].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[197].corners[c].sections[s].pole_rho = rh;
                defs[197].corners[c].sections[s].pole_theta = th;
                defs[197].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[197].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [199] 199 Vocal-to-Comb
        defs[198].name = "199 Vocal-to-Comb";
        defs[198].category = "MORPH";
        defs[198].bankIndex = 5;
        defs[198].activeStages = 7;
        defs[198].isPlanar4 = false;
        defs[198].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[198].corners[c].sections[s].pole_rho = rh;
                defs[198].corners[c].sections[s].pole_theta = th;
                defs[198].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[198].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [200] 200 String-to-Horn
        defs[199].name = "200 String-to-Horn";
        defs[199].category = "MORPH";
        defs[199].bankIndex = 5;
        defs[199].activeStages = 7;
        defs[199].isPlanar4 = false;
        defs[199].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[199].corners[c].sections[s].pole_rho = rh;
                defs[199].corners[c].sections[s].pole_theta = th;
                defs[199].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[199].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [201] 201 Cave-to-Sky
        defs[200].name = "201 Cave-to-Sky";
        defs[200].category = "MORPH";
        defs[200].bankIndex = 5;
        defs[200].activeStages = 7;
        defs[200].isPlanar4 = false;
        defs[200].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[200].corners[c].sections[s].pole_rho = rh;
                defs[200].corners[c].sections[s].pole_theta = th;
                defs[200].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[200].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [202] 202 Sub-to-Air
        defs[201].name = "202 Sub-to-Air";
        defs[201].category = "MORPH";
        defs[201].bankIndex = 5;
        defs[201].activeStages = 7;
        defs[201].isPlanar4 = false;
        defs[201].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[201].corners[c].sections[s].pole_rho = rh;
                defs[201].corners[c].sections[s].pole_theta = th;
                defs[201].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[201].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [203] 203 Warm-to-Laser
        defs[202].name = "203 Warm-to-Laser";
        defs[202].category = "MORPH";
        defs[202].bankIndex = 5;
        defs[202].activeStages = 7;
        defs[202].isPlanar4 = false;
        defs[202].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[202].corners[c].sections[s].pole_rho = rh;
                defs[202].corners[c].sections[s].pole_theta = th;
                defs[202].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[202].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [204] 204 Analog-to-FM
        defs[203].name = "204 Analog-to-FM";
        defs[203].category = "MORPH";
        defs[203].bankIndex = 5;
        defs[203].activeStages = 7;
        defs[203].isPlanar4 = false;
        defs[203].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[203].corners[c].sections[s].pole_rho = rh;
                defs[203].corners[c].sections[s].pole_theta = th;
                defs[203].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[203].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [205] 205 Organ-to-Choir
        defs[204].name = "205 Organ-to-Choir";
        defs[204].category = "MORPH";
        defs[204].bankIndex = 5;
        defs[204].activeStages = 7;
        defs[204].isPlanar4 = false;
        defs[204].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[204].corners[c].sections[s].pole_rho = rh;
                defs[204].corners[c].sections[s].pole_theta = th;
                defs[204].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[204].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [206] 206 Notch-to-Allpass
        defs[205].name = "206 Notch-to-Allpass";
        defs[205].category = "MORPH";
        defs[205].bankIndex = 5;
        defs[205].activeStages = 7;
        defs[205].isPlanar4 = false;
        defs[205].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[205].corners[c].sections[s].pole_rho = rh;
                defs[205].corners[c].sections[s].pole_theta = th;
                defs[205].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[205].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [207] 207 Reso-to-Squeeze
        defs[206].name = "207 Reso-to-Squeeze";
        defs[206].category = "MORPH";
        defs[206].bankIndex = 5;
        defs[206].activeStages = 7;
        defs[206].isPlanar4 = false;
        defs[206].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[206].corners[c].sections[s].pole_rho = rh;
                defs[206].corners[c].sections[s].pole_theta = th;
                defs[206].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[206].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [208] 208 Hollow-to-Full
        defs[207].name = "208 Hollow-to-Full";
        defs[207].category = "MORPH";
        defs[207].bankIndex = 5;
        defs[207].activeStages = 7;
        defs[207].isPlanar4 = false;
        defs[207].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[207].corners[c].sections[s].pole_rho = rh;
                defs[207].corners[c].sections[s].pole_theta = th;
                defs[207].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[207].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [209] 209 Dry-to-Space
        defs[208].name = "209 Dry-to-Space";
        defs[208].category = "MORPH";
        defs[208].bankIndex = 5;
        defs[208].activeStages = 7;
        defs[208].isPlanar4 = false;
        defs[208].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[208].corners[c].sections[s].pole_rho = rh;
                defs[208].corners[c].sections[s].pole_theta = th;
                defs[208].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[208].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [210] 210 Flat-to-Peak
        defs[209].name = "210 Flat-to-Peak";
        defs[209].category = "MORPH";
        defs[209].bankIndex = 5;
        defs[209].activeStages = 7;
        defs[209].isPlanar4 = false;
        defs[209].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[209].corners[c].sections[s].pole_rho = rh;
                defs[209].corners[c].sections[s].pole_theta = th;
                defs[209].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[209].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [211] 211 Sine-to-Square
        defs[210].name = "211 Sine-to-Square";
        defs[210].category = "MORPH";
        defs[210].bankIndex = 5;
        defs[210].activeStages = 7;
        defs[210].isPlanar4 = false;
        defs[210].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[210].corners[c].sections[s].pole_rho = rh;
                defs[210].corners[c].sections[s].pole_theta = th;
                defs[210].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[210].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [212] 212 Bass-to-Lead
        defs[211].name = "212 Bass-to-Lead";
        defs[211].category = "MORPH";
        defs[211].bankIndex = 5;
        defs[211].activeStages = 7;
        defs[211].isPlanar4 = false;
        defs[211].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[211].corners[c].sections[s].pole_rho = rh;
                defs[211].corners[c].sections[s].pole_theta = th;
                defs[211].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[211].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [213] 213 Fold-to-Comb
        defs[212].name = "213 Fold-to-Comb";
        defs[212].category = "MORPH";
        defs[212].bankIndex = 5;
        defs[212].activeStages = 7;
        defs[212].isPlanar4 = false;
        defs[212].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[212].corners[c].sections[s].pole_rho = rh;
                defs[212].corners[c].sections[s].pole_theta = th;
                defs[212].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[212].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [214] 214 Vactrol-to-Ladder
        defs[213].name = "214 Vactrol-to-Ladder";
        defs[213].category = "MORPH";
        defs[213].bankIndex = 5;
        defs[213].activeStages = 7;
        defs[213].isPlanar4 = false;
        defs[213].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[213].corners[c].sections[s].pole_rho = rh;
                defs[213].corners[c].sections[s].pole_theta = th;
                defs[213].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[213].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [215] 215 Zero-to-Hero
        defs[214].name = "215 Zero-to-Hero";
        defs[214].category = "MORPH";
        defs[214].bankIndex = 5;
        defs[214].activeStages = 7;
        defs[214].isPlanar4 = false;
        defs[214].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[214].corners[c].sections[s].pole_rho = rh;
                defs[214].corners[c].sections[s].pole_theta = th;
                defs[214].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[214].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [216] 216 Infinity Morph
        defs[215].name = "216 Infinity Morph";
        defs[215].category = "MORPH";
        defs[215].bankIndex = 5;
        defs[215].activeStages = 7;
        defs[215].isPlanar4 = false;
        defs[215].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[215].corners[c].sections[s].pole_rho = rh;
                defs[215].corners[c].sections[s].pole_theta = th;
                defs[215].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[215].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [217] 217 Saturating Poles
        defs[216].name = "217 Saturating Poles";
        defs[216].category = "EDGY";
        defs[216].bankIndex = 6;
        defs[216].activeStages = 4;
        defs[216].isPlanar4 = true;
        defs[216].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[216].corners[c].sections[s].pole_rho = rh;
                defs[216].corners[c].sections[s].pole_theta = th;
                defs[216].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[216].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [218] 218 Wavefold Cube
        defs[217].name = "218 Wavefold Cube";
        defs[217].category = "EDGY";
        defs[217].bankIndex = 6;
        defs[217].activeStages = 4;
        defs[217].isPlanar4 = true;
        defs[217].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[217].corners[c].sections[s].pole_rho = rh;
                defs[217].corners[c].sections[s].pole_theta = th;
                defs[217].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[217].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [219] 219 Resonant Grit
        defs[218].name = "219 Resonant Grit";
        defs[218].category = "EDGY";
        defs[218].bankIndex = 6;
        defs[218].activeStages = 4;
        defs[218].isPlanar4 = true;
        defs[218].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[218].corners[c].sections[s].pole_rho = rh;
                defs[218].corners[c].sections[s].pole_theta = th;
                defs[218].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[218].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [220] 220 Overdrive Comb
        defs[219].name = "220 Overdrive Comb";
        defs[219].category = "EDGY";
        defs[219].bankIndex = 6;
        defs[219].activeStages = 4;
        defs[219].isPlanar4 = true;
        defs[219].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[219].corners[c].sections[s].pole_rho = rh;
                defs[219].corners[c].sections[s].pole_theta = th;
                defs[219].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[219].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [221] 221 Industrial Edge
        defs[220].name = "221 Industrial Edge";
        defs[220].category = "EDGY";
        defs[220].bankIndex = 6;
        defs[220].activeStages = 4;
        defs[220].isPlanar4 = true;
        defs[220].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[220].corners[c].sections[s].pole_rho = rh;
                defs[220].corners[c].sections[s].pole_theta = th;
                defs[220].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[220].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [222] 222 Diode Clip
        defs[221].name = "222 Diode Clip";
        defs[221].category = "EDGY";
        defs[221].bankIndex = 6;
        defs[221].activeStages = 4;
        defs[221].isPlanar4 = true;
        defs[221].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[221].corners[c].sections[s].pole_rho = rh;
                defs[221].corners[c].sections[s].pole_theta = th;
                defs[221].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[221].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [223] 223 Asym Distortion
        defs[222].name = "223 Asym Distortion";
        defs[222].category = "EDGY";
        defs[222].bankIndex = 6;
        defs[222].activeStages = 4;
        defs[222].isPlanar4 = true;
        defs[222].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[222].corners[c].sections[s].pole_rho = rh;
                defs[222].corners[c].sections[s].pole_theta = th;
                defs[222].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[222].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [224] 224 Hard Clip Ladder
        defs[223].name = "224 Hard Clip Ladder";
        defs[223].category = "EDGY";
        defs[223].bankIndex = 6;
        defs[223].activeStages = 4;
        defs[223].isPlanar4 = true;
        defs[223].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[223].corners[c].sections[s].pole_rho = rh;
                defs[223].corners[c].sections[s].pole_theta = th;
                defs[223].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[223].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [225] 225 Fuzz Resonator
        defs[224].name = "225 Fuzz Resonator";
        defs[224].category = "EDGY";
        defs[224].bankIndex = 6;
        defs[224].activeStages = 4;
        defs[224].isPlanar4 = true;
        defs[224].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[224].corners[c].sections[s].pole_rho = rh;
                defs[224].corners[c].sections[s].pole_theta = th;
                defs[224].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[224].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [226] 226 Square Drive
        defs[225].name = "226 Square Drive";
        defs[225].category = "EDGY";
        defs[225].bankIndex = 6;
        defs[225].activeStages = 4;
        defs[225].isPlanar4 = true;
        defs[225].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[225].corners[c].sections[s].pole_rho = rh;
                defs[225].corners[c].sections[s].pole_theta = th;
                defs[225].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[225].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [227] 227 Folded Notch
        defs[226].name = "227 Folded Notch";
        defs[226].category = "EDGY";
        defs[226].bankIndex = 6;
        defs[226].activeStages = 4;
        defs[226].isPlanar4 = true;
        defs[226].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[226].corners[c].sections[s].pole_rho = rh;
                defs[226].corners[c].sections[s].pole_theta = th;
                defs[226].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[226].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [228] 228 Biting Lead
        defs[227].name = "228 Biting Lead";
        defs[227].category = "EDGY";
        defs[227].bankIndex = 6;
        defs[227].activeStages = 4;
        defs[227].isPlanar4 = true;
        defs[227].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[227].corners[c].sections[s].pole_rho = rh;
                defs[227].corners[c].sections[s].pole_theta = th;
                defs[227].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[227].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [229] 229 Trash Can
        defs[228].name = "229 Trash Can";
        defs[228].category = "EDGY";
        defs[228].bankIndex = 6;
        defs[228].activeStages = 4;
        defs[228].isPlanar4 = true;
        defs[228].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[228].corners[c].sections[s].pole_rho = rh;
                defs[228].corners[c].sections[s].pole_theta = th;
                defs[228].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[228].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [230] 230 Dirty Vowel
        defs[229].name = "230 Dirty Vowel";
        defs[229].category = "EDGY";
        defs[229].bankIndex = 6;
        defs[229].activeStages = 4;
        defs[229].isPlanar4 = true;
        defs[229].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[229].corners[c].sections[s].pole_rho = rh;
                defs[229].corners[c].sections[s].pole_theta = th;
                defs[229].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[229].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [231] 231 Bitcrush Filter
        defs[230].name = "231 Bitcrush Filter";
        defs[230].category = "EDGY";
        defs[230].bankIndex = 6;
        defs[230].activeStages = 4;
        defs[230].isPlanar4 = true;
        defs[230].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[230].corners[c].sections[s].pole_rho = rh;
                defs[230].corners[c].sections[s].pole_theta = th;
                defs[230].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[230].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [232] 232 Crunch Peak
        defs[231].name = "232 Crunch Peak";
        defs[231].category = "EDGY";
        defs[231].bankIndex = 6;
        defs[231].activeStages = 4;
        defs[231].isPlanar4 = true;
        defs[231].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[231].corners[c].sections[s].pole_rho = rh;
                defs[231].corners[c].sections[s].pole_theta = th;
                defs[231].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[231].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [233] 233 Distorted Flange
        defs[232].name = "233 Distorted Flange";
        defs[232].category = "EDGY";
        defs[232].bankIndex = 6;
        defs[232].activeStages = 4;
        defs[232].isPlanar4 = true;
        defs[232].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[232].corners[c].sections[s].pole_rho = rh;
                defs[232].corners[c].sections[s].pole_theta = th;
                defs[232].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[232].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [234] 234 Aggressive Acid
        defs[233].name = "234 Aggressive Acid";
        defs[233].category = "EDGY";
        defs[233].bankIndex = 6;
        defs[233].activeStages = 4;
        defs[233].isPlanar4 = true;
        defs[233].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[233].corners[c].sections[s].pole_rho = rh;
                defs[233].corners[c].sections[s].pole_theta = th;
                defs[233].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[233].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [235] 235 Screamer Notch
        defs[234].name = "235 Screamer Notch";
        defs[234].category = "EDGY";
        defs[234].bankIndex = 6;
        defs[234].activeStages = 4;
        defs[234].isPlanar4 = true;
        defs[234].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[234].corners[c].sections[s].pole_rho = rh;
                defs[234].corners[c].sections[s].pole_theta = th;
                defs[234].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[234].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [236] 236 Rust Resonator
        defs[235].name = "236 Rust Resonator";
        defs[235].category = "EDGY";
        defs[235].bankIndex = 6;
        defs[235].activeStages = 4;
        defs[235].isPlanar4 = true;
        defs[235].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[235].corners[c].sections[s].pole_rho = rh;
                defs[235].corners[c].sections[s].pole_theta = th;
                defs[235].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[235].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [237] 237 Acid Burn
        defs[236].name = "237 Acid Burn";
        defs[236].category = "EDGY";
        defs[236].bankIndex = 6;
        defs[236].activeStages = 4;
        defs[236].isPlanar4 = true;
        defs[236].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[236].corners[c].sections[s].pole_rho = rh;
                defs[236].corners[c].sections[s].pole_theta = th;
                defs[236].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[236].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [238] 238 Overdriven Pipe
        defs[237].name = "238 Overdriven Pipe";
        defs[237].category = "EDGY";
        defs[237].bankIndex = 6;
        defs[237].activeStages = 4;
        defs[237].isPlanar4 = true;
        defs[237].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[237].corners[c].sections[s].pole_rho = rh;
                defs[237].corners[c].sections[s].pole_theta = th;
                defs[237].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[237].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [239] 239 Saturated Cavity
        defs[238].name = "239 Saturated Cavity";
        defs[238].category = "EDGY";
        defs[238].bankIndex = 6;
        defs[238].activeStages = 4;
        defs[238].isPlanar4 = true;
        defs[238].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[238].corners[c].sections[s].pole_rho = rh;
                defs[238].corners[c].sections[s].pole_theta = th;
                defs[238].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[238].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [240] 240 Pushed Ladder
        defs[239].name = "240 Pushed Ladder";
        defs[239].category = "EDGY";
        defs[239].bankIndex = 6;
        defs[239].activeStages = 4;
        defs[239].isPlanar4 = true;
        defs[239].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[239].corners[c].sections[s].pole_rho = rh;
                defs[239].corners[c].sections[s].pole_theta = th;
                defs[239].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[239].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [241] 241 Fuzz Box
        defs[240].name = "241 Fuzz Box";
        defs[240].category = "EDGY";
        defs[240].bankIndex = 6;
        defs[240].activeStages = 4;
        defs[240].isPlanar4 = true;
        defs[240].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[240].corners[c].sections[s].pole_rho = rh;
                defs[240].corners[c].sections[s].pole_theta = th;
                defs[240].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[240].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [242] 242 Cracked Bell
        defs[241].name = "242 Cracked Bell";
        defs[241].category = "EDGY";
        defs[241].bankIndex = 6;
        defs[241].activeStages = 4;
        defs[241].isPlanar4 = true;
        defs[241].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[241].corners[c].sections[s].pole_rho = rh;
                defs[241].corners[c].sections[s].pole_theta = th;
                defs[241].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[241].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [243] 243 Raw Dirt
        defs[242].name = "243 Raw Dirt";
        defs[242].category = "EDGY";
        defs[242].bankIndex = 6;
        defs[242].activeStages = 4;
        defs[242].isPlanar4 = true;
        defs[242].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[242].corners[c].sections[s].pole_rho = rh;
                defs[242].corners[c].sections[s].pole_theta = th;
                defs[242].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[242].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [244] 244 Bleeding Pole
        defs[243].name = "244 Bleeding Pole";
        defs[243].category = "EDGY";
        defs[243].bankIndex = 6;
        defs[243].activeStages = 4;
        defs[243].isPlanar4 = true;
        defs[243].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[243].corners[c].sections[s].pole_rho = rh;
                defs[243].corners[c].sections[s].pole_theta = th;
                defs[243].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[243].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [245] 245 Ripped Membrane
        defs[244].name = "245 Ripped Membrane";
        defs[244].category = "EDGY";
        defs[244].bankIndex = 6;
        defs[244].activeStages = 4;
        defs[244].isPlanar4 = true;
        defs[244].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[244].corners[c].sections[s].pole_rho = rh;
                defs[244].corners[c].sections[s].pole_theta = th;
                defs[244].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[244].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [246] 246 Overloaded Comb
        defs[245].name = "246 Overloaded Comb";
        defs[245].category = "EDGY";
        defs[245].bankIndex = 6;
        defs[245].activeStages = 4;
        defs[245].isPlanar4 = true;
        defs[245].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[245].corners[c].sections[s].pole_rho = rh;
                defs[245].corners[c].sections[s].pole_theta = th;
                defs[245].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[245].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [247] 247 Clipping Formant
        defs[246].name = "247 Clipping Formant";
        defs[246].category = "EDGY";
        defs[246].bankIndex = 6;
        defs[246].activeStages = 4;
        defs[246].isPlanar4 = true;
        defs[246].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[246].corners[c].sections[s].pole_rho = rh;
                defs[246].corners[c].sections[s].pole_theta = th;
                defs[246].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[246].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [248] 248 Nasty Sweep
        defs[247].name = "248 Nasty Sweep";
        defs[247].category = "EDGY";
        defs[247].bankIndex = 6;
        defs[247].activeStages = 4;
        defs[247].isPlanar4 = true;
        defs[247].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[247].corners[c].sections[s].pole_rho = rh;
                defs[247].corners[c].sections[s].pole_theta = th;
                defs[247].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[247].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [249] 249 Stator Overdrive
        defs[248].name = "249 Stator Overdrive";
        defs[248].category = "EDGY";
        defs[248].bankIndex = 6;
        defs[248].activeStages = 4;
        defs[248].isPlanar4 = true;
        defs[248].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[248].corners[c].sections[s].pole_rho = rh;
                defs[248].corners[c].sections[s].pole_theta = th;
                defs[248].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[248].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [250] 250 Punched Cone
        defs[249].name = "250 Punched Cone";
        defs[249].category = "EDGY";
        defs[249].bankIndex = 6;
        defs[249].activeStages = 4;
        defs[249].isPlanar4 = true;
        defs[249].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[249].corners[c].sections[s].pole_rho = rh;
                defs[249].corners[c].sections[s].pole_theta = th;
                defs[249].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[249].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [251] 251 Grater Peak
        defs[250].name = "251 Grater Peak";
        defs[250].category = "EDGY";
        defs[250].bankIndex = 6;
        defs[250].activeStages = 4;
        defs[250].isPlanar4 = true;
        defs[250].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[250].corners[c].sections[s].pole_rho = rh;
                defs[250].corners[c].sections[s].pole_theta = th;
                defs[250].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[250].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [252] 252 Maximum Drive
        defs[251].name = "252 Maximum Drive";
        defs[251].category = "EDGY";
        defs[251].bankIndex = 6;
        defs[251].activeStages = 4;
        defs[251].isPlanar4 = true;
        defs[251].modelNorm = 0.30f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[251].corners[c].sections[s].pole_rho = rh;
                defs[251].corners[c].sections[s].pole_theta = th;
                defs[251].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[251].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [253] 253 Inverted Poles
        defs[252].name = "253 Inverted Poles";
        defs[252].category = "COSMIC";
        defs[252].bankIndex = 7;
        defs[252].activeStages = 7;
        defs[252].isPlanar4 = false;
        defs[252].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[252].corners[c].sections[s].pole_rho = rh;
                defs[252].corners[c].sections[s].pole_theta = th;
                defs[252].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[252].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [254] 254 Nuclear Waste
        defs[253].name = "254 Nuclear Waste";
        defs[253].category = "COSMIC";
        defs[253].bankIndex = 7;
        defs[253].activeStages = 7;
        defs[253].isPlanar4 = false;
        defs[253].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[253].corners[c].sections[s].pole_rho = rh;
                defs[253].corners[c].sections[s].pole_theta = th;
                defs[253].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[253].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [255] 255 Sub Shifter
        defs[254].name = "255 Sub Shifter";
        defs[254].category = "COSMIC";
        defs[254].bankIndex = 7;
        defs[254].activeStages = 7;
        defs[254].isPlanar4 = false;
        defs[254].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[254].corners[c].sections[s].pole_rho = rh;
                defs[254].corners[c].sections[s].pole_theta = th;
                defs[254].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[254].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [256] 256 Alien Resonator
        defs[255].name = "256 Alien Resonator";
        defs[255].category = "COSMIC";
        defs[255].bankIndex = 7;
        defs[255].activeStages = 7;
        defs[255].isPlanar4 = false;
        defs[255].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[255].corners[c].sections[s].pole_rho = rh;
                defs[255].corners[c].sections[s].pole_theta = th;
                defs[255].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[255].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [257] 257 Void Drone
        defs[256].name = "257 Void Drone";
        defs[256].category = "COSMIC";
        defs[256].bankIndex = 7;
        defs[256].activeStages = 7;
        defs[256].isPlanar4 = false;
        defs[256].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[256].corners[c].sections[s].pole_rho = rh;
                defs[256].corners[c].sections[s].pole_theta = th;
                defs[256].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[256].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [258] 258 Abyssal Formant
        defs[257].name = "258 Abyssal Formant";
        defs[257].category = "COSMIC";
        defs[257].bankIndex = 7;
        defs[257].activeStages = 7;
        defs[257].isPlanar4 = false;
        defs[257].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[257].corners[c].sections[s].pole_rho = rh;
                defs[257].corners[c].sections[s].pole_theta = th;
                defs[257].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[257].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [259] 259 Stellar Wind
        defs[258].name = "259 Stellar Wind";
        defs[258].category = "COSMIC";
        defs[258].bankIndex = 7;
        defs[258].activeStages = 7;
        defs[258].isPlanar4 = false;
        defs[258].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.1800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[258].corners[c].sections[s].pole_rho = rh;
                defs[258].corners[c].sections[s].pole_theta = th;
                defs[258].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[258].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [260] 260 Event Horizon
        defs[259].name = "260 Event Horizon";
        defs[259].category = "COSMIC";
        defs[259].bankIndex = 7;
        defs[259].activeStages = 7;
        defs[259].isPlanar4 = false;
        defs[259].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[259].corners[c].sections[s].pole_rho = rh;
                defs[259].corners[c].sections[s].pole_theta = th;
                defs[259].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[259].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [261] 261 Chaos Matrix
        defs[260].name = "261 Chaos Matrix";
        defs[260].category = "COSMIC";
        defs[260].bankIndex = 7;
        defs[260].activeStages = 7;
        defs[260].isPlanar4 = false;
        defs[260].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[260].corners[c].sections[s].pole_rho = rh;
                defs[260].corners[c].sections[s].pole_theta = th;
                defs[260].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[260].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [262] 262 Spectral Drift
        defs[261].name = "262 Spectral Drift";
        defs[261].category = "COSMIC";
        defs[261].bankIndex = 7;
        defs[261].activeStages = 7;
        defs[261].isPlanar4 = false;
        defs[261].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[261].corners[c].sections[s].pole_rho = rh;
                defs[261].corners[c].sections[s].pole_theta = th;
                defs[261].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[261].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [263] 263 Singularity
        defs[262].name = "263 Singularity";
        defs[262].category = "COSMIC";
        defs[262].bankIndex = 7;
        defs[262].activeStages = 7;
        defs[262].isPlanar4 = false;
        defs[262].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[262].corners[c].sections[s].pole_rho = rh;
                defs[262].corners[c].sections[s].pole_theta = th;
                defs[262].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[262].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [264] 264 Dark Energy
        defs[263].name = "264 Dark Energy";
        defs[263].category = "COSMIC";
        defs[263].bankIndex = 7;
        defs[263].activeStages = 7;
        defs[263].isPlanar4 = false;
        defs[263].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.2800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[263].corners[c].sections[s].pole_rho = rh;
                defs[263].corners[c].sections[s].pole_theta = th;
                defs[263].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[263].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [265] 265 Quantum Foam
        defs[264].name = "265 Quantum Foam";
        defs[264].category = "COSMIC";
        defs[264].bankIndex = 7;
        defs[264].activeStages = 7;
        defs[264].isPlanar4 = false;
        defs[264].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[264].corners[c].sections[s].pole_rho = rh;
                defs[264].corners[c].sections[s].pole_theta = th;
                defs[264].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[264].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [266] 266 Cosmic Ray
        defs[265].name = "266 Cosmic Ray";
        defs[265].category = "COSMIC";
        defs[265].bankIndex = 7;
        defs[265].activeStages = 7;
        defs[265].isPlanar4 = false;
        defs[265].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[265].corners[c].sections[s].pole_rho = rh;
                defs[265].corners[c].sections[s].pole_theta = th;
                defs[265].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[265].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [267] 267 Pulsar Ping
        defs[266].name = "267 Pulsar Ping";
        defs[266].category = "COSMIC";
        defs[266].bankIndex = 7;
        defs[266].activeStages = 7;
        defs[266].isPlanar4 = false;
        defs[266].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[266].corners[c].sections[s].pole_rho = rh;
                defs[266].corners[c].sections[s].pole_theta = th;
                defs[266].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[266].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [268] 268 Nebula Cloud
        defs[267].name = "268 Nebula Cloud";
        defs[267].category = "COSMIC";
        defs[267].bankIndex = 7;
        defs[267].activeStages = 7;
        defs[267].isPlanar4 = false;
        defs[267].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[267].corners[c].sections[s].pole_rho = rh;
                defs[267].corners[c].sections[s].pole_theta = th;
                defs[267].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[267].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [269] 269 Gravity Well
        defs[268].name = "269 Gravity Well";
        defs[268].category = "COSMIC";
        defs[268].bankIndex = 7;
        defs[268].activeStages = 7;
        defs[268].isPlanar4 = false;
        defs[268].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.3800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[268].corners[c].sections[s].pole_rho = rh;
                defs[268].corners[c].sections[s].pole_theta = th;
                defs[268].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[268].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [270] 270 Anti-Matter
        defs[269].name = "270 Anti-Matter";
        defs[269].category = "COSMIC";
        defs[269].bankIndex = 7;
        defs[269].activeStages = 7;
        defs[269].isPlanar4 = false;
        defs[269].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[269].corners[c].sections[s].pole_rho = rh;
                defs[269].corners[c].sections[s].pole_theta = th;
                defs[269].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[269].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [271] 271 Tachyon Burst
        defs[270].name = "271 Tachyon Burst";
        defs[270].category = "COSMIC";
        defs[270].bankIndex = 7;
        defs[270].activeStages = 7;
        defs[270].isPlanar4 = false;
        defs[270].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[270].corners[c].sections[s].pole_rho = rh;
                defs[270].corners[c].sections[s].pole_theta = th;
                defs[270].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[270].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [272] 272 Hyperspace
        defs[271].name = "272 Hyperspace";
        defs[271].category = "COSMIC";
        defs[271].bankIndex = 7;
        defs[271].activeStages = 7;
        defs[271].isPlanar4 = false;
        defs[271].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[271].corners[c].sections[s].pole_rho = rh;
                defs[271].corners[c].sections[s].pole_theta = th;
                defs[271].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[271].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [273] 273 Dimension 5
        defs[272].name = "273 Dimension 5";
        defs[272].category = "COSMIC";
        defs[272].bankIndex = 7;
        defs[272].activeStages = 7;
        defs[272].isPlanar4 = false;
        defs[272].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[272].corners[c].sections[s].pole_rho = rh;
                defs[272].corners[c].sections[s].pole_theta = th;
                defs[272].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[272].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [274] 274 Black Hole
        defs[273].name = "274 Black Hole";
        defs[273].category = "COSMIC";
        defs[273].bankIndex = 7;
        defs[273].activeStages = 7;
        defs[273].isPlanar4 = false;
        defs[273].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.4800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[273].corners[c].sections[s].pole_rho = rh;
                defs[273].corners[c].sections[s].pole_theta = th;
                defs[273].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[273].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [275] 275 Wormhole Sweep
        defs[274].name = "275 Wormhole Sweep";
        defs[274].category = "COSMIC";
        defs[274].bankIndex = 7;
        defs[274].activeStages = 7;
        defs[274].isPlanar4 = false;
        defs[274].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[274].corners[c].sections[s].pole_rho = rh;
                defs[274].corners[c].sections[s].pole_theta = th;
                defs[274].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[274].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [276] 276 Solar Flare
        defs[275].name = "276 Solar Flare";
        defs[275].category = "COSMIC";
        defs[275].bankIndex = 7;
        defs[275].activeStages = 7;
        defs[275].isPlanar4 = false;
        defs[275].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[275].corners[c].sections[s].pole_rho = rh;
                defs[275].corners[c].sections[s].pole_theta = th;
                defs[275].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[275].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [277] 277 Cosmic Dust
        defs[276].name = "277 Cosmic Dust";
        defs[276].category = "COSMIC";
        defs[276].bankIndex = 7;
        defs[276].activeStages = 7;
        defs[276].isPlanar4 = false;
        defs[276].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[276].corners[c].sections[s].pole_rho = rh;
                defs[276].corners[c].sections[s].pole_theta = th;
                defs[276].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[276].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [278] 278 Quasar Pulse
        defs[277].name = "278 Quasar Pulse";
        defs[277].category = "COSMIC";
        defs[277].bankIndex = 7;
        defs[277].activeStages = 7;
        defs[277].isPlanar4 = false;
        defs[277].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[277].corners[c].sections[s].pole_rho = rh;
                defs[277].corners[c].sections[s].pole_theta = th;
                defs[277].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[277].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [279] 279 Entropy Decay
        defs[278].name = "279 Entropy Decay";
        defs[278].category = "COSMIC";
        defs[278].bankIndex = 7;
        defs[278].activeStages = 7;
        defs[278].isPlanar4 = false;
        defs[278].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.5800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[278].corners[c].sections[s].pole_rho = rh;
                defs[278].corners[c].sections[s].pole_theta = th;
                defs[278].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[278].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [280] 280 Supernova
        defs[279].name = "280 Supernova";
        defs[279].category = "COSMIC";
        defs[279].bankIndex = 7;
        defs[279].activeStages = 7;
        defs[279].isPlanar4 = false;
        defs[279].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[279].corners[c].sections[s].pole_rho = rh;
                defs[279].corners[c].sections[s].pole_theta = th;
                defs[279].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[279].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [281] 281 String Theory
        defs[280].name = "281 String Theory";
        defs[280].category = "COSMIC";
        defs[280].bankIndex = 7;
        defs[280].activeStages = 7;
        defs[280].isPlanar4 = false;
        defs[280].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[280].corners[c].sections[s].pole_rho = rh;
                defs[280].corners[c].sections[s].pole_theta = th;
                defs[280].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[280].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [282] 282 Astral Abyss
        defs[281].name = "282 Astral Abyss";
        defs[281].category = "COSMIC";
        defs[281].bankIndex = 7;
        defs[281].activeStages = 7;
        defs[281].isPlanar4 = false;
        defs[281].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[281].corners[c].sections[s].pole_rho = rh;
                defs[281].corners[c].sections[s].pole_theta = th;
                defs[281].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[281].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [283] 283 Nyarlathotep
        defs[282].name = "283 Nyarlathotep";
        defs[282].category = "COSMIC";
        defs[282].bankIndex = 7;
        defs[282].activeStages = 7;
        defs[282].isPlanar4 = false;
        defs[282].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6600f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[282].corners[c].sections[s].pole_rho = rh;
                defs[282].corners[c].sections[s].pole_theta = th;
                defs[282].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[282].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [284] 284 Azathoth Dream
        defs[283].name = "284 Azathoth Dream";
        defs[283].category = "COSMIC";
        defs[283].bankIndex = 7;
        defs[283].activeStages = 7;
        defs[283].isPlanar4 = false;
        defs[283].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.6800f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[283].corners[c].sections[s].pole_rho = rh;
                defs[283].corners[c].sections[s].pole_theta = th;
                defs[283].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[283].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [285] 285 Yog-Sothoth Gate
        defs[284].name = "285 Yog-Sothoth Gate";
        defs[284].category = "COSMIC";
        defs[284].bankIndex = 7;
        defs[284].activeStages = 7;
        defs[284].isPlanar4 = false;
        defs[284].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7000f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[284].corners[c].sections[s].pole_rho = rh;
                defs[284].corners[c].sections[s].pole_theta = th;
                defs[284].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[284].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [286] 286 Cthulhu Slumber
        defs[285].name = "286 Cthulhu Slumber";
        defs[285].category = "COSMIC";
        defs[285].bankIndex = 7;
        defs[285].activeStages = 7;
        defs[285].isPlanar4 = false;
        defs[285].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7200f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[285].corners[c].sections[s].pole_rho = rh;
                defs[285].corners[c].sections[s].pole_theta = th;
                defs[285].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[285].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [287] 287 Hastur Whisper
        defs[286].name = "287 Hastur Whisper";
        defs[286].category = "COSMIC";
        defs[286].bankIndex = 7;
        defs[286].activeStages = 7;
        defs[286].isPlanar4 = false;
        defs[286].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.7400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[286].corners[c].sections[s].pole_rho = rh;
                defs[286].corners[c].sections[s].pole_theta = th;
                defs[286].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[286].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        // [288] 288 Shub-Niggurath
        defs[287].name = "288 Shub-Niggurath";
        defs[287].category = "COSMIC";
        defs[287].bankIndex = 7;
        defs[287].activeStages = 7;
        defs[287].isPlanar4 = false;
        defs[287].modelNorm = 0.33f;
        for (int c = 0; c < 8; ++c) {
            float cX = (c & 1) ? 1.0f : 0.0f;
            float cY = (c & 2) ? 1.0f : 0.0f;
            float cZ = (c & 4) ? 1.0f : 0.0f;
            for (int s = 0; s < 7; ++s) {
                float sF = 1.0f + static_cast<float>(s) * 0.38f;
                float th = (0.0400f * sF) * (1.0f + cX * 1.4f + cY * 0.7f);
                th = juce::jlimit(0.01f, 3.10f, th);
                float rh = 0.85f + cZ * 0.11f - static_cast<float>(s) * 0.012f;
                rh = juce::jlimit(0.50f, 0.965f, rh);
                defs[287].corners[c].sections[s].pole_rho = rh;
                defs[287].corners[c].sections[s].pole_theta = th;
                defs[287].corners[c].sections[s].zero_rho = 0.88f - cZ * 0.18f;
                defs[287].corners[c].sections[s].zero_theta = th * (0.85f + cY * 0.25f);
            }
        }
        return defs;
    }();
    return rom;
}

const ZPlaneModelDef& ZPlaneRom::getModel(int index) noexcept {
    index = juce::jlimit(0, kNumModels - 1, index);
    return getStaticRom()[index];
}

} // namespace Nyarlat
