# Advanced Typography Engine (JUCE 9)

## Goal Description
Implement a data-driven, CSS-class style typography system utilizing JUCE 9's advanced text rendering pipeline. This removes hardcoded font sizes and styles from the C++ source code, allowing complete visual overhauls of the synth's text rendering purely via JSON.

## 1. Asset Pipeline (Embedded Fonts)
To ensure 100% visual parity across macOS, Windows, and Linux VST3 hosts, we will not rely on system fonts (e.g. `Arial` or `Helvetica Neue`).

- **Sourcing**: Add open-source .ttf or .otf files (e.g. Inter, Roboto) to a new `assets/fonts/` directory.
- **BinaryData**: Add these fonts to the Projucer/CMake `BinaryData` target.
- **Registration**: On plugin startup, register the fonts globally using `juce::Typeface::createSystemTypefaceFor(BinaryData::FontFile_ttf, BinaryData::FontFile_ttfSize)`.

## 2. JSON Schema (The 'CSS Class' Approach)
Instead of styling every single knob individually, we will define global Typography Classes in the layout JSON (e.g., `tkf_layout.json`).

`json
{
  "typography": {
    "CardHeader": {
      "font_family": "Inter",
      "weight": "Bold",
      "size": 18.0,
      "tracking": 1.2,
      "justification": "centered"
    },
    "KnobLabel": {
      "font_family": "Inter",
      "weight": "Regular",
      "size": 12.0,
      "tracking": 0.0,
      "justification": "centered"
    }
  }
}
``n
## 3. The Klang Editor (TKE) Integration
Inside the JSON Editor, when a Layout JSON is selected, a new dedicated sub-panel or tab section for **Typography** will be exposed.
- This will parse the "typography" block and generate sliders for `size` and `tracking`, and dropdowns for `font_family`, `weight`, and `justification`.
- Because the Visual Preview pane uses the shared C++ UI code, moving the `tracking` slider will instantly update the letter-spacing of all connected text labels in the Live Preview via JUCE 9's text layout engine.

## 4. C++ Implementation (UIComponents.h)
Update the shared `SynthCardComponent` and generic label painters to query the active layout's Typography dictionary:
- `juce::Font` construction will pull from the registered BinaryData typefaces.
- Use JUCE 9's advanced `juce::AttributedString` or `juce::TextLayout` to apply the `tracking` (letter-spacing) parameter before calling `g.drawText()` or `g.drawFittedText()`.
