#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "DigiStrikeFS2Processor.h"
#include "DigiStrikeFS2ControlInfo.h"
#include "../dsp/common/SteppedRandom.h"
#include "../dsp/drums/FormlessModMath.h"
#include <vector>
#include <array>
#include <memory>
#include <functional>
#include <unordered_map>

// ==============================================================================
// DigiStrikeFS2Editor.h — Tactical Console GUI for DigiStrike FS2
// ==============================================================================
// Sovereign Tactical Console matching Formless Spawn visual & tactile standards.
// Part of The Klang Suite SOCEP Engine. Copyright (c) 2026 Cracked Chimes.
// ==============================================================================

// ==============================================================================
// PresetStubComponent: Header Preset Selector Stub Capsule
// ==============================================================================
class PresetStubComponent : public juce::Component {
public:
    PresetStubComponent() {
        setInterceptsMouseClicks(false, false);
    }

    void setAccentColour(juce::Colour c) {
        if (accent_ != c) {
            accent_ = c;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF0F131A));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        g.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
        g.setColour(accent_.withAlpha(0.90f));
        g.drawText("<  01: DEFAULT VOID  >", bounds, juce::Justification::centred);
    }

private:
    juce::Colour accent_{0xFFFFAA00};
};

// ==============================================================================
// HeaderInspector: Universal Header Mouse-Over & Live Drag Telemetry Panel
// ==============================================================================
class DigiStrikeHeaderInspector : public juce::Component {
public:
    struct Readout {
        juce::String name;
        juce::String value;
        juce::String blurb;
        juce::Colour accent{0xFFFFAA00};
        bool isDefault{false};
    };

    DigiStrikeHeaderInspector() {
        setInterceptsMouseClicks(false, false);
        setReadoutDefault();
    }

    void setReadout(Readout r) {
        if (readout_.name == r.name && readout_.value == r.value &&
            readout_.blurb == r.blurb && readout_.accent == r.accent &&
            readout_.isDefault == r.isDefault)
            return;

        readout_ = std::move(r);
        repaint();
    }

    void setReadoutDefault() {
        Readout d;
        d.name = "DIGISTRIKE FS2";
        d.value = "READY";
        d.blurb = "sovereign vintage PCM workstation (2,069 voices across 20 manufacturer lineages)";
        d.accent = juce::Colour(0xFFFFAA00);
        d.isDefault = true;
        setReadout(d);
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();

        // Recessed OLED display cavity
        g.setColour(juce::Colour(0xFF0F131A));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // Col 1: Parameter Name (x: 10, w: 115, 12pt bold - zero truncation)
        auto nameRect = juce::Rectangle<float>(bounds.getX() + 10.0f, bounds.getY(), 115.0f, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
        g.setColour(readout_.accent);
        g.drawText(readout_.name, nameRect, juce::Justification::centredLeft, true);

        // Col 2: Explanation blurb across remaining area (2 lines)
        float blurbW = std::max(40.0f, bounds.getWidth() - 132.0f);
        auto blurbRect = juce::Rectangle<float>(bounds.getX() + 128.0f, bounds.getY(), blurbW, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.setColour(readout_.isDefault ? juce::Colour(0xFF6E7681) : juce::Colour(0xFFC9D1D9));
        g.drawFittedText(readout_.blurb, blurbRect.toNearestInt(), juce::Justification::centredLeft, 2);
    }

private:
    Readout readout_;
};

// ==============================================================================
// 3-Way Rocker Toggle (Lifeform Triad / Voice Mode)
// ==============================================================================
class DigiStrikeRocker3 : public juce::Component {
public:
    DigiStrikeRocker3(const std::array<juce::String, 3>& labels) : labels_(labels) {}

    void setColours(const std::array<juce::Colour, 3>& c) {
        colours_ = c;
        repaint();
    }

    void setState(int idx) {
        idx = std::clamp(idx, 0, 2);
        if (activeIndex_ != idx) {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }
    std::function<void(int)> onSelect;

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float segW = bounds.getWidth() / 3.0f;
        auto activeRect = juce::Rectangle<float>(bounds.getX() + activeIndex_ * segW, bounds.getY(), segW, bounds.getHeight());
        g.setColour(colours_[static_cast<size_t>(activeIndex_)]);
        g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        for (int i = 0; i < 3; ++i) {
            bool isActive = (i == activeIndex_);
            auto segRect = juce::Rectangle<float>(bounds.getX() + static_cast<float>(i) * segW, bounds.getY(), segW, bounds.getHeight());
            g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFFF0F6FC));
            g.drawText(labels_[static_cast<size_t>(i)], segRect, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const float segW = static_cast<float>(getWidth()) / 3.0f;
        int clickedIdx = std::clamp(static_cast<int>(e.position.x / segW), 0, 2);
        if (clickedIdx != activeIndex_) {
            activeIndex_ = clickedIdx;
            if (onSelect) onSelect(activeIndex_);
            repaint();
        }
    }

private:
    std::array<juce::String, 3> labels_;
    int activeIndex_{0};
    std::array<juce::Colour, 3> colours_{
        juce::Colour(0xFF00C8FF), // Synthetic
        juce::Colour(0xFFE08C00), // Cyborg
        juce::Colour(0xFF6BBF59)  // Organic
    };
};

// ==============================================================================
// DigiStrikeSegmentedBar: Multi-Segment Tactile Button Matrix Strip
// ==============================================================================
class DigiStrikeSegmentedBar : public juce::Component {
public:
    DigiStrikeSegmentedBar(const std::vector<juce::String>& labels) : labels_(labels) {}

    void setState(int idx) {
        idx = std::clamp(idx, 0, static_cast<int>(labels_.size()) - 1);
        if (activeIndex_ != idx) {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }
    std::function<void(int)> onSelect;

    void setAccentColour(juce::Colour c) {
        if (accentColour_ != c) {
            accentColour_ = c;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const int numSegs = static_cast<int>(labels_.size());
        if (numSegs <= 0) return;

        const float segW = bounds.getWidth() / static_cast<float>(numSegs);
        auto activeRect = juce::Rectangle<float>(bounds.getX() + static_cast<float>(activeIndex_) * segW, bounds.getY(), segW, bounds.getHeight());
        g.setColour(accentColour_);
        g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

        // Inner vertical dividers between inactive segments
        g.setColour(juce::Colour(0xFF262D38));
        for (int i = 1; i < numSegs; ++i) {
            if (i != activeIndex_ && i != activeIndex_ + 1) {
                float divX = bounds.getX() + static_cast<float>(i) * segW;
                g.drawVerticalLine(static_cast<int>(std::round(divX)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);
            }
        }

        g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
        for (int i = 0; i < numSegs; ++i) {
            bool isActive = (i == activeIndex_);
            auto segRect = juce::Rectangle<float>(bounds.getX() + static_cast<float>(i) * segW, bounds.getY(), segW, bounds.getHeight());
            g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFF8B949E));
            g.drawText(labels_[static_cast<size_t>(i)], segRect, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const int numSegs = static_cast<int>(labels_.size());
        if (numSegs <= 0) return;
        const float segW = static_cast<float>(getWidth()) / static_cast<float>(numSegs);
        int clickedIdx = std::clamp(static_cast<int>(e.position.x / segW), 0, numSegs - 1);
        activeIndex_ = clickedIdx;
        if (onSelect) onSelect(activeIndex_);
        repaint();
    }

private:
    std::vector<juce::String> labels_;
    int activeIndex_{0};
    juce::Colour accentColour_{0xFFFFAA00};
};

// ==============================================================================
// DigiStrikeMultiSelectBar: Strictly Alphabetical 2x10 Grid with Hardware Livery
// ==============================================================================
class DigiStrikeMultiSelectBar : public juce::Component {
public:
    // Static display mapping: Slot [0..19] -> Canonical Lineage ID [0..19]
    // Row 1 (0..9):  Adios, Apex, Berlin, Circuits, Crown, Curios, Dartmouth, Empyrean, Falcon, Gothenburg
    // Row 2 (10..19): Hamamatsu, Hexcess, Keio, K-Works, Malvern, Nippon, Osaka, Shibuya, Sydney, Tokyo
    static constexpr std::array<int, 20> kSlotToLineage = {{
        13, // Slot 0:  Adios       (Alesis HR-16 / SR-16)
         1, // Slot 1:  Apex        (Oberheim DMX / DX)
        18, // Slot 2:  Berlin      (Jomox XBase 09)
         8, // Slot 3:  Circuits    (Sequential Drumtraks / Studio 440)
         0, // Slot 4:  Crown       (Linn LinnDrum / LM-1)
        19, // Slot 5:  Curios      (Circuit-Bent / Esoteric)
        15, // Slot 6:  Dartmouth   (Synclavier II / Direct-to-Disk)
         2, // Slot 7:  Empyrean    (E-mu SP-1200 / Drumulator)
         3, // Slot 8:  Falcon      (Akai MPC60 / XR10)
         7, // Slot 9:  Gothenburg  (Elektron Machinedrum / Digitakt)
        10, // Slot 10: Hamamatsu   (Yamaha RX5 / TX16W)
        17, // Slot 11: Hexcess     (Simmons SDS-V / SDS7)
        11, // Slot 12: Keio        (Korg DDD-1 / Electribe)
         4, // Slot 13: K-Works     (Kawai R-50 / XD-5)
        16, // Slot 14: Malvern     (Ensoniq Mirage / ASR-10)
        12, // Slot 15: Nippon      (Sony Broadcast PCM)
         5, // Slot 16: Osaka       (Boss Dr. Rhythm DR-110 / DR-660)
         9, // Slot 17: Shibuya     (Casio RZ-1 / CZ / SK-1)
        14, // Slot 18: Sydney      (Fairlight CMI Series IIx)
         6  // Slot 19: Tokyo       (Roland TR-808 / TR-909)
    }};

    static constexpr std::array<int, 20> kLineageToSlot = {{
         4, // Lineage 0  (Crown)     -> Slot 4
         1, // Lineage 1  (Apex)      -> Slot 1
         7, // Lineage 2  (Empyrean)  -> Slot 7
         8, // Lineage 3  (Falcon)    -> Slot 8
        13, // Lineage 4  (K-Works)   -> Slot 13
        16, // Lineage 5  (Osaka)     -> Slot 16
        19, // Lineage 6  (Tokyo)     -> Slot 19
         9, // Lineage 7  (Gothenburg)-> Slot 9
         3, // Lineage 8  (Circuits)  -> Slot 3
        17, // Lineage 9  (Shibuya)   -> Slot 17
        10, // Lineage 10 (Hamamatsu) -> Slot 10
        12, // Lineage 11 (Keio)      -> Slot 12
        15, // Lineage 12 (Nippon)    -> Slot 15
         0, // Lineage 13 (Adios)     -> Slot 0
        18, // Lineage 14 (Sydney)    -> Slot 18
         6, // Lineage 15 (Dartmouth) -> Slot 6
        14, // Lineage 16 (Malvern)   -> Slot 14
        11, // Lineage 17 (Hexcess)   -> Slot 11
         2, // Lineage 18 (Berlin)    -> Slot 2
         5  // Lineage 19 (Curios)    -> Slot 5
    }};

    static constexpr int lineageAt(int slot) noexcept {
        return (slot >= 0 && slot < 20) ? kSlotToLineage[static_cast<size_t>(slot)] : 0;
    }

    static constexpr int slotOf(int lineage) noexcept {
        return (lineage >= 0 && lineage < 20) ? kLineageToSlot[static_cast<size_t>(lineage)] : 0;
    }

    struct BrandTheme {
        juce::uint32 bgActive;       // Rich hardware chassis when ON
        juce::uint32 borderActive;   // High-luminance perimeter glow
        juce::uint32 textActive;     // High-contrast text
        juce::uint32 bgInactive;     // Deep obsidian with subtle 8% brand tint
        juce::uint32 borderInactive; // Subtle tinted bezel
        juce::uint32 textInactive;   // Legible muted text (>= 4.5:1 against bgInactive)
        juce::uint32 pinstripe;      // Signature brand heritage color
    };

    static constexpr std::array<BrandTheme, 20> kBrandThemes = {{
        // 0: Crown (Linn: Warm Walnut / Amber LED)
        { 0xFF5C3A16, 0xFFFFB300, 0xFFFFF6E6, 0xFF1C1713, 0xFF3D2E20, 0xFFC9B095, 0xFFFFB300 },
        // 1: Apex (Oberheim: Royal Navy / Electric Blue)
        { 0xFF0D3266, 0xFF0088FF, 0xFFFFFFFF, 0xFF121722, 0xFF1E3554, 0xFF8EB5E0, 0xFF0088FF },
        // 2: Empyrean (E-mu: Battleship Slate / Cyan)
        { 0xFF1F3D52, 0xFF00D2FF, 0xFF00F5FF, 0xFF131B22, 0xFF203B4D, 0xFF82B0C7, 0xFF00D2FF },
        // 3: Falcon (Akai: MPC60 Vintage Cream Chassis / Bold Crimson Text)
        { 0xFFEDE8DC, 0xFFE60000, 0xFFB80000, 0xFF201A18, 0xFF4A3532, 0xFFC7BFAF, 0xFFE60000 },
        // 4: K-Works (Kawai: 80s Synth Dark Teal / Mint)
        { 0xFF0C4238, 0xFF00FFC4, 0xFFE0FFF8, 0xFF121E1C, 0xFF1C423A, 0xFF85BDB0, 0xFF00D1A0 },
        // 5: Osaka (Boss: Tangerine Orange / Dark Charcoal)
        { 0xFFD65500, 0xFFFF8533, 0xFF0D1117, 0xFF201712, 0xFF4D2F1E, 0xFFC79E85, 0xFFFF6B00 },
        // 6: Tokyo (Roland: TR-808 Acid Orange / Charcoal)
        { 0xFF8C3E00, 0xFFFFA033, 0xFFFFFFFF, 0xFF201611, 0xFF4D2B1A, 0xFFC7A189, 0xFFFF8C00 },
        // 7: Gothenburg (Elektron: Dark Anodized Graphite / Crimson LED)
        { 0xFF3D1F24, 0xFFFF3B30, 0xFFF5F5F7, 0xFF1C1517, 0xFF3D2328, 0xFFA89CA0, 0xFFFF3B30 },
        // 8: Circuits (Sequential: Dark Cherry Walnut / Prophet Red)
        { 0xFF5E1720, 0xFFFF3B4B, 0xFFFFF2E6, 0xFF1F1315, 0xFF452026, 0xFFC49FA5, 0xFFD32F2F },
        // 9: Shibuya (Casio: Deep Electric Cobalt / Sky)
        { 0xFF14387D, 0xFF4D94FF, 0xFFFFFFFF, 0xFF131724, 0xFF203659, 0xFF8CAADB, 0xFF2979FF },
        // 10: Hamamatsu (Yamaha: DX/TX Aqua Teal / Olive)
        { 0xFF0D4045, 0xFF00F0FF, 0xFFE0FFFF, 0xFF121D1F, 0xFF1E3F45, 0xFF85B5BA, 0xFF00E5FF },
        // 11: Keio (Korg: Deep Indigo / Sky Cyan)
        { 0xFF0D3254, 0xFF33C2FF, 0xFFFFFFFF, 0xFF121A22, 0xFF1C3A52, 0xFF85B0D1, 0xFF00B0FF },
        // 12: Nippon (Sony: Titanium Champagne Gold)
        { 0xFF574614, 0xFFFFDF33, 0xFFFFFDF0, 0xFF1D1A12, 0xFF42391E, 0xFFC2B693, 0xFFFFD700 },
        // 13: Adios (Alesis: HR-16 Deep Violet / Neon)
        { 0xFF4A155E, 0xFFB866FF, 0xFFFFFFFF, 0xFF181220, 0xFF3A204D, 0xFFB099D1, 0xFF9D4EDD },
        // 14: Sydney (Fairlight: P31 CRT Screen Green)
        { 0xFF0A3314, 0xFF39FF14, 0xFF39FF14, 0xFF111C13, 0xFF1C3D22, 0xFF7FB888, 0xFF39FF14 },
        // 15: Dartmouth (Synclavier: Military Dark Ruby / 7-Segment Red)
        { 0xFF5C0F1A, 0xFFFF2E4C, 0xFFFFFFFF, 0xFF1F1214, 0xFF451921, 0xFFC29198, 0xFFFF1744 },
        // 16: Malvern (Ensoniq: VFD Vacuum Tube Coral Orange)
        { 0xFF7A3314, 0xFFFF8559, 0xFFFFFFFF, 0xFF201512, 0xFF4A2A1E, 0xFFC49F91, 0xFFFF7043 },
        // 17: Hexcess (Simmons: SDS-V Hex Pad Blue)
        { 0xFF03669C, 0xFF1EB4FF, 0xFFFFFFFF, 0xFF111922, 0xFF163E59, 0xFF82B3D6, 0xFF0288D1 },
        // 18: Berlin (Jomox: Industrial Techno Tangerine)
        { 0xFF8A290A, 0xFFFF6E3D, 0xFFFFFFFF, 0xFF201511, 0xFF4D281C, 0xFFC2998C, 0xFFFF5722 },
        // 19: Curios (Circuit-Bent: Glitch Neon Hot Pink)
        { 0xFFB8005C, 0xFFFF2E94, 0xFFFFFFFF, 0xFF20111A, 0xFF4D1B36, 0xFFC28CAE, 0xFFFF007F }
    }};

    DigiStrikeMultiSelectBar(const std::vector<juce::String>& labels) : labels_(labels) {}

    void setMask(uint32_t mask) {
        if (mask_ != mask) {
            mask_ = mask;
            repaint();
        }
    }

    uint32_t getMask() const noexcept { return mask_; }

    int getSelected() const noexcept {
        const uint32_t m = (mask_ & 0xFFFFFu) != 0 ? (mask_ & 0xFFFFFu) : 0xFFFFFu;
        return std::countr_zero(m);
    }

    void setAccentColour(juce::Colour c) {
        if (accentColour_ != c) {
            accentColour_ = c;
            repaint();
        }
    }

    void updateMorphDNA(uint32_t mask, int category, float morphNorm) {
        morphNorm = std::clamp(morphNorm, 0.0f, 1.0f);
        category = std::clamp(category, 0, 9);

        int lin0 = -1;
        int lin1 = -1;
        float w0 = 0.0f;
        float w1 = 0.0f;

        if (mask != 0) {
            std::array<DigiStrikeFS2Catalog::WaveCoord, 80> activeWaves{};
            int activeCount = 0;
            for (int i = 0; i < 80; ++i) {
                const auto coord = DigiStrikeFS2Catalog::kMusicalOrder[static_cast<size_t>(category)][static_cast<size_t>(i)];
                if ((mask & (1u << coord.lineage)) != 0) {
                    if (activeCount < 80) {
                        activeWaves[static_cast<size_t>(activeCount++)] = coord;
                    }
                }
            }

            if (activeCount == 1) {
                lin0 = activeWaves[0].lineage;
                w0 = 1.0f;
            } else if (activeCount > 1) {
                const float span = static_cast<float>(activeCount - 1);
                const float pos = std::clamp(morphNorm * span, 0.0f, span);
                const int i0 = static_cast<int>(pos);
                const int i1 = std::min(i0 + 1, activeCount - 1);
                const float frac = pos - static_cast<float>(i0);
                const float smoothFrac = frac * frac * (3.0f - 2.0f * frac);

                lin0 = activeWaves[static_cast<size_t>(i0)].lineage;
                lin1 = activeWaves[static_cast<size_t>(i1)].lineage;
                w0 = 1.0f - smoothFrac;
                w1 = smoothFrac;
            }
        }

        if (morphLin0_ != lin0 || morphLin1_ != lin1 ||
            std::abs(morphWeight0_ - w0) > 0.01f || std::abs(morphWeight1_ - w1) > 0.01f) {
            morphLin0_ = lin0;
            morphLin1_ = lin1;
            morphWeight0_ = w0;
            morphWeight1_ = w1;
            repaint();
        }
    }

    std::function<void(uint32_t)> onMaskChange;
    std::function<void(int hoveredIdx)> onHover;
    std::function<void(const juce::String&)> onBudgetLimit;

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        const int totalButtons = static_cast<int>(labels_.size());
        if (totalButtons <= 0) return;

        // Render strictly alphabetical 2x10 Grid (80x22px buttons, 4px gaps)
        const int cols = 10;
        const int rows = 2;
        const float gapX = 4.0f;
        const float gapY = 4.0f;
        const float btnW = (bounds.getWidth() - static_cast<float>(cols - 1) * gapX) / static_cast<float>(cols);
        const float btnH = (bounds.getHeight() - static_cast<float>(rows - 1) * gapY) / static_cast<float>(rows);

        for (int slot = 0; slot < totalButtons; ++slot) {
            const int r = slot / cols;
            const int c = slot % cols;
            const auto btnRect = juce::Rectangle<float>(
                bounds.getX() + static_cast<float>(c) * (btnW + gapX),
                bounds.getY() + static_cast<float>(r) * (btnH + gapY),
                btnW, btnH);

            const int lin = (slot < 20) ? kSlotToLineage[static_cast<size_t>(slot)] : slot;
            const bool isOn = (mask_ & (1u << lin)) != 0;
            const auto& theme = kBrandThemes[static_cast<size_t>(lin % 20)];
            const auto heritageCol = juce::Colour(theme.pinstripe);

            if (isOn) {
                // Active button: Authentic brand chassis
                g.setColour(juce::Colour(theme.bgActive));
                g.fillRoundedRectangle(btnRect, 3.0f);

                // Perimeter brand glow border (1.2px)
                g.setColour(juce::Colour(theme.borderActive));
                g.drawRoundedRectangle(btnRect.reduced(0.5f), 3.0f, 1.2f);

                // Authentic Heritage Pinstripe across top (full brilliance)
                auto stripe = btnRect.reduced(1.5f, 0.0f).removeFromTop(2.5f);
                g.setColour(heritageCol);
                g.fillRoundedRectangle(stripe, 1.25f);

                // Morph DNA Halo highlight at bottom edge
                if (lin == morphLin0_ || lin == morphLin1_) {
                    float w = (lin == morphLin0_) ? morphWeight0_ : morphWeight1_;
                    auto haloBar = btnRect.reduced(3.0f, 0.0f).removeFromBottom(2.5f);
                    g.setColour(juce::Colour(0xFF00FF88).withAlpha(0.6f + 0.4f * w));
                    g.fillRoundedRectangle(haloBar, 1.25f);
                }

                auto textRect = btnRect.withTrimmedTop(3.0f).withTrimmedBottom(1.0f).reduced(2.0f, 0.0f);
                g.setFont(juce::Font(juce::FontOptions(9.5f).withStyle("Bold")));
                g.setColour(juce::Colour(theme.textActive));
                g.drawFittedText(labels_[static_cast<size_t>(lin)], textRect.toNearestInt(), juce::Justification::centred, 1, 0.80f);
            } else {
                // Normal inactive button: Dark obsidian body with 8% brand undertone
                g.setColour(juce::Colour(theme.bgInactive));
                g.fillRoundedRectangle(btnRect, 3.0f);
                g.setColour(juce::Colour(theme.borderInactive));
                g.drawRoundedRectangle(btnRect.reduced(0.5f), 3.0f, 0.8f);

                auto stripe = btnRect.reduced(2.0f, 0.0f).removeFromTop(2.5f);
                g.setColour(heritageCol.withAlpha(0.70f));
                g.fillRoundedRectangle(stripe, 1.25f);

                auto textRect = btnRect.withTrimmedTop(3.0f).withTrimmedBottom(1.0f).reduced(2.0f, 0.0f);
                g.setFont(juce::Font(juce::FontOptions(9.5f).withStyle("Bold")));
                g.setColour(juce::Colour(theme.textInactive));
                g.drawFittedText(labels_[static_cast<size_t>(lin)], textRect.toNearestInt(), juce::Justification::centred, 1, 0.80f);
            }
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const int totalButtons = static_cast<int>(labels_.size());
        if (totalButtons <= 0) return;

        auto bounds = getLocalBounds().toFloat();
        const int cols = 10;
        const int rows = 2;
        const float gapX = 4.0f;
        const float gapY = 4.0f;
        const float btnW = (bounds.getWidth() - static_cast<float>(cols - 1) * gapX) / static_cast<float>(cols);
        const float btnH = (bounds.getHeight() - static_cast<float>(rows - 1) * gapY) / static_cast<float>(rows);

        int clickedSlot = -1;
        for (int slot = 0; slot < totalButtons; ++slot) {
            const int r = slot / cols;
            const int c = slot % cols;
            const auto btnRect = juce::Rectangle<float>(
                bounds.getX() + static_cast<float>(c) * (btnW + gapX),
                bounds.getY() + static_cast<float>(r) * (btnH + gapY),
                btnW, btnH);
            if (btnRect.contains(e.position)) {
                clickedSlot = slot;
                break;
            }
        }
        if (clickedSlot < 0 || clickedSlot >= totalButtons) return;
        const int clickedLin = (clickedSlot < 20) ? kSlotToLineage[static_cast<size_t>(clickedSlot)] : clickedSlot;

        int activeLineageCount = 0;
        for (int i = 0; i < totalButtons; ++i) {
            if ((mask_ & (1u << i)) != 0) ++activeLineageCount;
        }

        if (e.getNumberOfClicks() >= 2) {
            // Double-Click: Reset all lineages active
            mask_ = 0xFFFFFu;
        } else if (e.mods.isPopupMenu() || e.mods.isAltDown()) {
            // Solo clicked lineage (or reset to all if already soloed)
            uint32_t soloMask = (1u << clickedLin);
            if (mask_ == soloMask) {
                mask_ = 0xFFFFFu;
            } else {
                mask_ = soloMask;
            }
        } else {
            // Standard click toggle: minimum-1 lineage invariant
            bool wasOn = (mask_ & (1u << clickedLin)) != 0;
            if (wasOn) {
                if (activeLineageCount > 1) {
                    mask_ &= ~(1u << clickedLin);
                }
            } else {
                if (activeLineageCount < 20) {
                    mask_ |= (1u << clickedLin);
                }
            }
        }

        if (onMaskChange) onMaskChange(mask_);
        repaint();
    }

    void mouseMove(const juce::MouseEvent& e) override {
        const int totalButtons = static_cast<int>(labels_.size());
        if (totalButtons <= 0) return;
        auto bounds = getLocalBounds().toFloat();
        const int cols = 10;
        const int rows = 2;
        const float gapX = 4.0f;
        const float gapY = 4.0f;
        const float btnW = (bounds.getWidth() - static_cast<float>(cols - 1) * gapX) / static_cast<float>(cols);
        const float btnH = (bounds.getHeight() - static_cast<float>(rows - 1) * gapY) / static_cast<float>(rows);

        int foundLin = -1;
        for (int slot = 0; slot < totalButtons; ++slot) {
            const int r = slot / cols;
            const int c = slot % cols;
            const auto btnRect = juce::Rectangle<float>(
                bounds.getX() + static_cast<float>(c) * (btnW + gapX),
                bounds.getY() + static_cast<float>(r) * (btnH + gapY),
                btnW, btnH);
            if (btnRect.contains(e.position)) {
                foundLin = (slot < 20) ? kSlotToLineage[static_cast<size_t>(slot)] : slot;
                break;
            }
        }
        if (foundLin != hoveredIdx_) {
            hoveredIdx_ = foundLin;
            if (onHover) onHover(hoveredIdx_);
        }
    }

    void mouseExit(const juce::MouseEvent& /*e*/) override {
        hoveredIdx_ = -1;
        if (onHover) onHover(-1);
    }

    int getHoveredIndex() const noexcept { return hoveredIdx_; }

private:
    std::vector<juce::String> labels_;
    uint32_t mask_{0x00000010};
    juce::Colour accentColour_{0xFF00C8FF};

    int morphLin0_{-1};
    int morphLin1_{-1};
    float morphWeight0_{0.0f};
    float morphWeight1_{0.0f};
    int hoveredIdx_{-1};
};
// ==============================================================================
// TactileTriggerPad: Center Altar Drum Trigger with Tactile Gold Flash
// ==============================================================================
class DigiStrikeTriggerPad : public juce::Component {
public:
    DigiStrikeTriggerPad() = default;

    std::function<void()> onTrigger;
    std::function<void()> onRelease;
    std::function<void()> onRightTrigger;
    std::function<void()> onRightRelease;

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (isDown_) {
            g.setColour(isRightClick_ ? juce::Colour(0xFF00E676) : juce::Colour(0xFFFFAA00));
            g.fillRoundedRectangle(bounds, 5.0f);
            g.setFont(juce::Font(juce::FontOptions(13.0f).withStyle("Bold")));
            g.setColour(juce::Colour(0xFF0D1117));
            g.drawText(isRightClick_ ? "C3" : "TRIG", bounds, juce::Justification::centred);
        } else {
            g.setColour(juce::Colour(0xFF21262D));
            g.fillRoundedRectangle(bounds, 5.0f);
            g.setColour(juce::Colour(0xFF30363D));
            g.drawRoundedRectangle(bounds, 5.0f, 1.2f);
            g.setFont(juce::Font(juce::FontOptions(13.0f).withStyle("Bold")));
            g.setColour(juce::Colour(0xFFF0F6FC));
            g.drawText("TRIG", bounds, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        isDown_ = true;
        isRightClick_ = e.mods.isRightButtonDown();
        repaint();
        if (isRightClick_) {
            if (onRightTrigger) onRightTrigger();
        } else {
            if (onTrigger) onTrigger();
        }
    }

    void mouseUp(const juce::MouseEvent& e) override {
        juce::ignoreUnused(e);
        if (isDown_) {
            isDown_ = false;
            repaint();
            if (isRightClick_) {
                if (onRightRelease) onRightRelease();
            } else {
                if (onRelease) onRelease();
            }
            isRightClick_ = false;
        }
    }

    void mouseExit(const juce::MouseEvent& e) override {
        juce::ignoreUnused(e);
        if (isDown_) {
            isDown_ = false;
            repaint();
            if (isRightClick_) {
                if (onRightRelease) onRightRelease();
            } else {
                if (onRelease) onRelease();
            }
            isRightClick_ = false;
        }
    }

private:
    bool isDown_{false};
    bool isRightClick_{false};
};

// ==============================================================================
// LimiterTactileButton: Tactile Limit Button with 3-State Hardware Jewel LED Pip
// ==============================================================================
class DigiStrikeLimiterButton : public juce::Button {
public:
    DigiStrikeLimiterButton() : juce::Button("LIMIT") {
        setClickingTogglesState(true);
    }

    void updateTelemetry(float gr) noexcept {
        if (std::abs(gr - currentGr_) > 0.004f) {
            currentGr_ = gr;
            repaint();
        }
    }

    void paintButton(juce::Graphics& g, bool, bool) override {
        auto bounds = getLocalBounds().toFloat();
        const bool on = getToggleState();

        const auto bgCol = on ? juce::Colour(0xFF21262D) : juce::Colour(0xFF161B22);
        g.setColour(bgCol);
        g.fillRoundedRectangle(bounds, 3.0f);

        const auto borderCol = on ? juce::Colour(0xFF484F58) : juce::Colour(0xFF30363D);
        g.setColour(borderCol);
        g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

        // Hardware Jewel LED Pip
        const float ledX = 7.0f;
        const float ledY = bounds.getCentreY() - 2.5f;
        const juce::Rectangle<float> ledRect(ledX, ledY, 5.0f, 5.0f);

        g.setColour(juce::Colour(0xFF0D1117));
        g.fillEllipse(ledRect.expanded(1.0f));

        if (!on) {
            g.setColour(juce::Colour(0xFF21262D));
            g.fillEllipse(ledRect);
        } else {
            juce::Colour ledCol = juce::Colour(0xFF39D353);
            juce::Colour glowCol = juce::Colour(0x662EA043);
            if (currentGr_ < 0.6310f) {
                ledCol = juce::Colour(0xFFFF3B30);
                glowCol = juce::Colour(0x88DA3633);
            } else if (currentGr_ < 0.9441f) {
                ledCol = juce::Colour(0xFFF3CA3E);
                glowCol = juce::Colour(0x66D29922);
            }

            g.setColour(glowCol);
            g.fillEllipse(ledRect.expanded(2.5f));
            g.setColour(ledCol);
            g.fillEllipse(ledRect);
            g.setColour(juce::Colours::white.withAlpha(0.6f));
            g.fillEllipse(juce::Rectangle<float>(ledX + 1.0f, ledY + 1.0f, 1.5f, 1.5f));
        }

        const auto textCol = on ? juce::Colours::white : juce::Colour(0xFF8B949E);
        g.setColour(textCol);
        g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
        const juce::Rectangle<float> labelArea(15.0f, 0.0f, bounds.getWidth() - 17.0f, bounds.getHeight());
        g.drawText("LIMIT", labelArea, juce::Justification::centred, false);
    }

private:
    float currentGr_{1.0f};
};

// ==============================================================================
// DigiStrikeGrid2x2: 2x2 Filter Type Selector (LP, HP, BP, PK)
// ==============================================================================
// ==============================================================================
// VoiceSelectorCapsule: Discrete Voice Stepper & Search Capsule
// ==============================================================================
class VoiceSelectorCapsule : public juce::Component {
public:
    VoiceSelectorCapsule() = default;

    void setText(const juce::String& text) {
        if (displayText_ != text) {
            displayText_ = text;
            repaint();
        }
    }

    std::function<void()> onStepPrev;
    std::function<void()> onStepNext;
    std::function<void()> onClickCenter;

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        g.setColour(juce::Colour(0xFF141820));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        auto leftArrow = juce::Rectangle<float>(bounds.getX(), bounds.getY(), 22.0f, bounds.getHeight());
        auto rightArrow = juce::Rectangle<float>(bounds.getRight() - 22.0f, bounds.getY(), 22.0f, bounds.getHeight());

        g.setColour(juce::Colour(0xFF222834));
        g.fillRoundedRectangle(leftArrow.reduced(1.0f), 3.0f);
        g.fillRoundedRectangle(rightArrow.reduced(1.0f), 3.0f);

        g.setColour(juce::Colour(0xFF8B949E));
        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        g.drawText("<", leftArrow, juce::Justification::centred);
        g.drawText(">", rightArrow, juce::Justification::centred);

        auto textRect = bounds.withTrimmedLeft(24.0f).withTrimmedRight(24.0f);
        g.setColour(juce::Colour(0xFF00E5FF));
        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        g.drawText(displayText_, textRect, juce::Justification::centred);
    }

    void mouseDown(const juce::MouseEvent& e) override {
        if (e.position.x < 24.0f) {
            if (onStepPrev) onStepPrev();
        } else if (e.position.x > getWidth() - 24.0f) {
            if (onStepNext) onStepNext();
        } else {
            if (onClickCenter) onClickCenter();
        }
    }

    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override {
        if (wheel.deltaY > 0.0f) {
            if (onStepNext) onStepNext();
        } else if (wheel.deltaY < 0.0f) {
            if (onStepPrev) onStepPrev();
        }
    }

private:
    juce::String displayText_{"FAIRL_ORCH5 (1/15) - Fairlight"};
};

class DigiStrikeRevButton : public juce::Button {
public:
    DigiStrikeRevButton() : juce::Button("REV") {
        setClickingTogglesState(true);
    }
    void paintButton(juce::Graphics& g, bool, bool) override {
        auto bounds = getLocalBounds().toFloat();
        const bool on = getToggleState();
        const auto bgCol = on ? juce::Colour(0xFF38BDF8) : juce::Colour(0xFF161B22);
        g.setColour(bgCol);
        g.fillRoundedRectangle(bounds, 3.0f);
        const auto borderCol = on ? juce::Colour(0xFF7DD3FC) : juce::Colour(0xFF30363D);
        g.setColour(borderCol);
        g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

        g.setColour(on ? juce::Colour(0xFF0D1117) : juce::Colour(0xFFF0F6FC));
        g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
        g.drawText("REV", bounds, juce::Justification::centred);
    }
};

class DigiStrikeGrid2x2 : public juce::Component {
public:
    DigiStrikeGrid2x2(const std::array<juce::String, 4>& labels) : labels_(labels) {}

    void setAccentColour(juce::Colour c) {
        accent_ = c;
        repaint();
    }

    void setState(int idx) {
        idx = std::clamp(idx, -1, 3);
        if (activeIndex_ != idx) {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }
    std::function<void(int)> onSelect;

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float colW = bounds.getWidth() * 0.5f;
        const float rowH = bounds.getHeight() * 0.5f;

        // Dividers
        g.setColour(juce::Colour(0xFF262D38));
        g.drawHorizontalLine(static_cast<int>(std::round(bounds.getY() + rowH)), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
        g.drawVerticalLine(static_cast<int>(std::round(bounds.getX() + colW)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);

        g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
        for (int i = 0; i < 4; ++i) {
            int row = i / 2;
            int col = i % 2;
            auto cellRect = juce::Rectangle<float>(bounds.getX() + static_cast<float>(col) * colW, bounds.getY() + static_cast<float>(row) * rowH, colW, rowH);
            bool isActive = (i == activeIndex_);

            if (isActive) {
                g.setColour(accent_);
                g.fillRoundedRectangle(cellRect.reduced(1.0f), 3.0f);
                g.setColour(juce::Colour(0xFF0D1117));
            } else {
                g.setColour(juce::Colour(0xFFF0F6FC));
            }

            g.drawFittedText(labels_[static_cast<size_t>(i)], cellRect.toNearestInt(), juce::Justification::centred, 1, 0.75f);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const float colW = static_cast<float>(getWidth()) * 0.5f;
        const float rowH = static_cast<float>(getHeight()) * 0.5f;
        int col = std::clamp(static_cast<int>(e.position.x / colW), 0, 1);
        int row = std::clamp(static_cast<int>(e.position.y / rowH), 0, 1);
        int idx = row * 2 + col;
        if (activeIndex_ == idx) {
            setState(-1);
            if (onSelect) onSelect(-1);
        } else {
            setState(idx);
            if (onSelect) onSelect(idx);
        }
    }

private:
    std::array<juce::String, 4> labels_;
    int activeIndex_{0};
    juce::Colour accent_{0xFF00E676};
};

// ==============================================================================
// DigiStrike LookAndFeel (Concentric Dual-Radar Halo Mechanics matching Formless Spawn)
// ==============================================================================
class DigiStrikeLookAndFeel : public juce::LookAndFeel_V4 {
public:
    DigiStrikeLookAndFeel() {
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xFF0D1117));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override {
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                             static_cast<float>(width), static_cast<float>(height)).reduced(3.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto centre = bounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        const float sweep = rotaryEndAngle - rotaryStartAngle;

        // Background Track Cavity
        juce::Path bgTrack;
        bgTrack.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xFF1F242E));
        g.strokePath(bgTrack, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // 1. Trickle-Down Ecoknobics Halo Arc (Inner Track Arc - Electric Cyan)
        if (slider.getProperties().contains("modLo") && slider.getProperties().contains("modHi")) {
            const float mLo = std::clamp(static_cast<float>(slider.getProperties()["modLo"]), 0.0f, 1.0f);
            const float mHi = std::clamp(static_cast<float>(slider.getProperties()["modHi"]), 0.0f, 1.0f);
            if (mHi > mLo + 0.001f) {
                juce::Path modArc;
                const float aLo = rotaryStartAngle + mLo * sweep;
                const float aHi = rotaryStartAngle + mHi * sweep;
                modArc.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f, aLo, aHi, true);
                g.setColour(juce::Colour(0xFF00E5FF).withAlpha(0.55f));
                g.strokePath(modArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
            if (slider.getProperties().contains("modPip")) {
                const float mPip = std::clamp(static_cast<float>(slider.getProperties()["modPip"]), 0.0f, 1.0f);
                const float aPip = rotaryStartAngle + mPip * sweep;
                const float pipR = radius - 3.0f;
                const float pipX = centre.x + pipR * std::sin(aPip);
                const float pipY = centre.y - pipR * std::cos(aPip);
                g.setColour(juce::Colour(0xFF00E5FF));
                g.fillEllipse(pipX - 2.5f, pipY - 2.5f, 5.0f, 5.0f);
            }
        }

        // 2. Stepped Random Range Halo (Outer Track Arc - Caustic Yellow)
        if (slider.getProperties().contains("randLo") && slider.getProperties().contains("randHi")) {
            const float rLo = static_cast<float>(slider.getProperties()["randLo"]);
            const float rHi = static_cast<float>(slider.getProperties()["randHi"]);
            if (rHi > rLo + 0.001f) {
                juce::Path randArc;
                const float aLo = rotaryStartAngle + rLo * sweep;
                const float aHi = rotaryStartAngle + rHi * sweep;
                randArc.addCentredArc(centre.x, centre.y, radius + 2.5f, radius + 2.5f, 0.0f, aLo, aHi, true);
                g.setColour(juce::Colour(0xFFF2E10F).withAlpha(0.40f));
                g.strokePath(randArc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }

        // Active Value Arc
        juce::Colour baseColour = slider.findColour(juce::Slider::rotarySliderFillColourId);
        juce::Path valArc;
        const bool centerAtNoon = slider.getProperties().contains("centerAtNoon")
            ? static_cast<bool>(slider.getProperties()["centerAtNoon"])
            : (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0);

        if (centerAtNoon) {
            const float zeroNorm = (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0)
                ? static_cast<float>(-slider.getMinimum() / (slider.getMaximum() - slider.getMinimum()))
                : 0.5f;
            const float midAngle = rotaryStartAngle + zeroNorm * sweep;
            if (std::abs(angle - midAngle) > 0.005f) {
                if (angle > midAngle) {
                    valArc.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f, midAngle, angle, true);
                } else {
                    valArc.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f, angle, midAngle, true);
                }
                g.setColour(baseColour);
                g.strokePath(valArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        } else {
            if (sliderPosProportional > 0.001f) {
                valArc.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f, rotaryStartAngle, angle, true);
                g.setColour(baseColour);
                g.strokePath(valArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }

        // Knob Dial Faceplate
        const float knobRadius = radius - 8.0f;
        g.setColour(juce::Colour(0xFF13171F));
        g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.2f);

        // Indicator Needle
        juce::Path needle;
        needle.startNewSubPath(centre.x, centre.y);
        needle.lineTo(centre.x + (knobRadius - 2.0f) * std::sin(angle), centre.y - (knobRadius - 2.0f) * std::cos(angle));
        g.setColour(baseColour.brighter(0.2f));
        g.strokePath(needle, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // =========================================================================
        // KILOHEARTS IN-DIAL VALUE BADGE (Ephemerally Rendered On Interaction)
        // =========================================================================
        bool isInteracting = slider.isMouseButtonDown();
        if (!isInteracting && slider.getProperties().contains("badgeExpiry")) {
            auto expiry = static_cast<juce::int64>(static_cast<double>(slider.getProperties()["badgeExpiry"]));
            if (juce::Time::getMillisecondCounter() < expiry) {
                isInteracting = true;
            }
        }
        if (isInteracting) {
            juce::String text = slider.textFromValueFunction
                ? slider.textFromValueFunction(slider.getValue())
                : slider.getTextFromValue(slider.getValue());
            drawKiloheartsValueBadge(g, bounds, text, baseColour);
        }
    }

    static void drawKiloheartsValueBadge(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                         const juce::String& text, juce::Colour accentColour) {
        if (text.isEmpty()) return;
        const bool isCompact = bounds.getWidth() < 50.0f;
        const auto font = juce::Font(juce::FontOptions(isCompact ? 9.0f : 10.5f).withStyle("Bold"));
        const float textW = juce::GlyphArrangement::getStringWidth(font, text);
        const float badgeW = std::clamp(textW + (isCompact ? 6.0f : 10.0f), 22.0f, bounds.getWidth() - 4.0f);
        const float badgeH = isCompact ? 12.0f : 14.0f;
        const float badgeX = bounds.getCentreX() - badgeW * 0.5f;
        const float badgeY = bounds.getBottom() - badgeH - 1.0f;
        const auto badgeRect = juce::Rectangle<float>(badgeX, badgeY, badgeW, badgeH);

        // Obsidian Badge Background
        g.setColour(juce::Colour(0xFA090D12));
        g.fillRoundedRectangle(badgeRect, 3.5f);

        // Subtle Accent-tinted Border
        g.setColour(accentColour.withAlpha(0.65f));
        g.drawRoundedRectangle(badgeRect, 3.5f, 1.0f);

        // Crisp White Text
        g.setFont(font);
        g.setColour(juce::Colour(0xFFF0F6FC));
        g.drawFittedText(text, badgeRect.toNearestInt(), juce::Justification::centred, 1);
    }
};

// ==============================================================================
// DigiStrikeFS2AudioProcessorEditor: Master Workstation Console
// ==============================================================================
class DigiStrikeFS2AudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    static constexpr int kBaseWidth = 900;
    static constexpr int kBaseHeight = 564;

    explicit DigiStrikeFS2AudioProcessorEditor(DigiStrikeFS2AudioProcessor&);
    ~DigiStrikeFS2AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    struct KnobItem {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    struct InspectorBinding {
        juce::String paramId;
        juce::Colour accent;
        juce::Slider* slider{nullptr};
        juce::String customName{};
        juce::String customBlurb{};
    };

private:
    void setupKnob(KnobItem& item, const juce::String& paramId, const juce::String& name,
                   const juce::String& unit, const juce::String& blurb,
                   juce::Colour col, bool isBipolar = false);

    void registerInspector(juce::Component* comp, const juce::String& paramId, juce::Colour accent,
                           juce::Slider* s = nullptr, const juce::String& cName = {}, const juce::String& cBlurb = {});
    juce::Component* resolveInspectorComponent(juce::Component* comp);
    void updateInspectorForComponent(juce::Component* comp);

    DigiStrikeFS2AudioProcessor& processor_;
    DigiStrikeLookAndFeel lnf_;

    std::unordered_map<juce::Component*, InspectorBinding> inspectorBindings_;
    juce::Component::SafePointer<juce::Component> lockedComponent_;
    juce::Component::SafePointer<juce::Slider> activeBadgeSlider_;
    juce::int64 activeBadgeExpiry_{0};

    // Header Row
    PresetStubComponent presetStub_;
    DigiStrikeHeaderInspector headerInspector_;
    DigiStrikeRocker3 lifeformRocker_{{"SYNTHETIC", "CYBORG", "ORGANIC"}};

    // Card 1 Knobs: Master Macro & Dynamics (Top-Left, 8)
    KnobItem macroXKnob_;
    KnobItem macroYKnob_;
    KnobItem macroZKnob_;
    KnobItem macroTKnob_;
    KnobItem velAggrKnob_;
    KnobItem velTimbKnob_;
    KnobItem panKnob_;
    KnobItem decayCurveKnob_;

    // Card 2: Uniform Sound Lab (Top-Right)
    // 8 Knobs (Upper Bay)
    KnobItem pitchDecayKnob_;
    KnobItem clickKnob_;
    KnobItem startKnob_;
    KnobItem timeKnob_;
    KnobItem punchKnob_;
    KnobItem noiseKnob_;
    KnobItem lengthKnob_;
    KnobItem morphKnob_;
    DigiStrikeGrid2x2 loopModeBar_{{"ONCE", "HOLD", "LOOP", "PING"}};
    juce::Label loopLabel_;
    DigiStrikeRevButton revButton_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> revAttachment_;
    VoiceSelectorCapsule voiceSelectorCapsule_;

    // Bottom Tactile Matrix (Lower Bay)
    DigiStrikeSegmentedBar categoryGrid_{{"BD", "SN", "TM", "CP", "HH", "CY", "LN", "MC", "FX", "ST", "BS"}};
    DigiStrikeMultiSelectBar lineageGrid_{{
        "Crown", "Apex", "Empyrean", "Falcon", "K-Works", "Osaka", "Tokyo", "Gothenburg", "Circuits", "Shibuya",
        "Hamamatsu", "Keio", "Nippon", "Adios", "Sydney", "Dartmouth", "Malvern", "Hexcess", "Berlin", "Curios"
    }};

    // Card 3 Pod 1: Stepped Random (Bottom-Left, 8)
    KnobItem randXKnob_;
    KnobItem randYKnob_;
    KnobItem randZKnob_;
    KnobItem randTKnob_;
    KnobItem randAggrKnob_;
    KnobItem randTimbKnob_;
    KnobItem randPanKnob_;
    KnobItem randDecayKnob_;

    // Card 3 Pod 2: Performance Altar
    KnobItem macroDecayKnob_;
    DigiStrikeTriggerPad trigButton_;

    // Card 3 Pod 3: VCF, Mojo & Output
    KnobItem cutoffKnob_;
    KnobItem resonanceKnob_;
    KnobItem filterEnvAmtKnob_;
    KnobItem filterDecayKnob_;
    DigiStrikeGrid2x2 filterTypeBar_{{"LP", "HP", "BP", "PK"}};
    void updateFilterTypeUI(int filterIdx);

    KnobItem filterDriveKnob_;
    KnobItem crushKnob_;
    KnobItem shiftKnob_;
    KnobItem squashKnob_;
    KnobItem widerKnob_;
    KnobItem sustainKnob_;

    // Master Output Strip
    DigiStrikeLimiterButton limiterButton_;
    float guiLimiterGr_{1.0f};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limiterAttachment_;
    KnobItem volumeKnob_;
    KnobItem ampDecayKnob_;
    KnobItem glideTimeKnob_;
    DigiStrikeRocker3 voiceModeRocker_{{"MONO", "GLIDE", "POLY"}};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DigiStrikeFS2AudioProcessorEditor)
};
