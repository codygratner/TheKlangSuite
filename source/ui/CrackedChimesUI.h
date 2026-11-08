#pragma once
// ==============================================================================
// CrackedChimesUI.h - Universal Fleet Umbrella Header
// ==============================================================================
// Consolidates the complete Cracked Chimes design system:
// - Design Tokens, Lifeform Triad & Semantic Colors
// - Concentric 4-Ring Knob Architecture with Live & Ghost Pips
// - Skew-True Transfer Functions & Engineering Unit Formatting
// - Pure OLED Graphics Displays (Strict Zero-Text Standard)
// - Shared Widgets (RockerToggle, RockerToggle3, SegmentedButtonBar, Modals)
// - Dual-Column Header Inspector with Blurbs & Accents
// - Stepper Audition Bar & Preset Manager Integration
// - Universal Scalable Window Harness (EditorScaleManager)
// ==============================================================================

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

// Anti-Mojibake MSVC UTF-8 Verification Law
static_assert(sizeof("\u00e9") == 3, "Source file must be compiled as UTF-8 (/utf-8 on MSVC).");

// Core Design System Modules
#include "cracked_chimes/CrackedChimesColours.h"
#include "cracked_chimes/CrackedChimesModMath.h"
#include "cracked_chimes/CrackedChimesKnob.h"
#include "cracked_chimes/CrackedChimesFader.h"
#include "cracked_chimes/CrackedChimesLookAndFeel.h"
#include "cracked_chimes/CrackedChimesWidgets.h"
#include "cracked_chimes/CrackedChimesHeaderInspector.h"
#include "cracked_chimes/CrackedChimesOledCanvas.h"
#include "cracked_chimes/CrackedChimesPresets.h"
#include "cracked_chimes/CrackedChimesHeaderTitle.h"
#include "cracked_chimes/AboutOverlay.h"
#include "cracked_chimes/CrackedChimesEditorHarness.h"

namespace CrackedChimes::PropIds
{
    inline const juce::Identifier modLo              ("modLo");
    inline const juce::Identifier modHi              ("modHi");
    inline const juce::Identifier modPip             ("modPip");
    inline const juce::Identifier modColour          ("modColour");
    inline const juce::Identifier randLo             ("randLo");
    inline const juce::Identifier randHi             ("randHi");
    inline const juce::Identifier randPip            ("randPip");
    inline const juce::Identifier randColour         ("randColour");
    inline const juce::Identifier stepRandAmount     ("stepRandAmount");
    inline const juce::Identifier activeStepVal      ("activeStepVal");
    inline const juce::Identifier isCutoffDial       ("isCutoffDial");
    inline const juce::Identifier cutoffEnvDepth     ("cutoffEnvDepth");
    inline const juce::Identifier cutoffEnvPip       ("cutoffEnvPip");
    inline const juce::Identifier cutoffLfoDepth     ("cutoffLfoDepth");
    inline const juce::Identifier cutoffLfoPip       ("cutoffLfoPip");
    inline const juce::Identifier cutoffBurbleDepth  ("cutoffBurbleDepth");
    inline const juce::Identifier cutoffBurblePip    ("cutoffBurblePip");
    inline const juce::Identifier cutoffFmDepth      ("cutoffFmDepth");
    inline const juce::Identifier badgeExpiry        ("badgeExpiry");
    inline const juce::Identifier isBipolar          ("isBipolar");
    inline const juce::Identifier centerAtNoon       ("centerAtNoon");
    inline const juce::Identifier morphLfoAmount     ("morphLfoAmount");
    inline const juce::Identifier orbitLfoAmount     ("orbitLfoAmount");
    inline const juce::Identifier amXModAmount       ("amXModAmount");
    inline const juce::Identifier amXModSource       ("amXModSource");
} // namespace CrackedChimes::PropIds

// Import primary components into CrackedChimes namespace for clean usage
namespace CrackedChimes
{
    using namespace UI;
    using namespace Colours;
}
