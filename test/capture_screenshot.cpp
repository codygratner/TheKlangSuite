#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <iostream>

class AdvancedColorPickerComponent : public juce::Component {
public:
    juce::Colour originalColor;
    juce::Colour currentColor;
    float currentHue, currentSat, currentVal, currentAlpha;

    AdvancedColorPickerComponent(juce::Colour initColor) {
        originalColor = initColor;
        currentColor = initColor;
        currentColor.getHSB(currentHue, currentSat, currentVal);
        currentAlpha = currentColor.getFloatAlpha();

        setSize(480, 360);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff202225)); // Dark background

        // Draw Hue Ring
        juce::Rectangle<float> ringBounds(20, 40, 200, 200);
        float centerX = ringBounds.getCentreX();
        float centerY = ringBounds.getCentreY();
        float outerRadius = 100.0f;
        float innerRadius = 70.0f;

        // Draw conical gradient manually by drawing small arcs
        for (float angle = 0.0f; angle < juce::MathConstants<float>::twoPi; angle += 0.02f) {
            float hue = angle / juce::MathConstants<float>::twoPi;
            g.setColour(juce::Colour::fromHSV(hue, 1.0f, 1.0f, 1.0f));
            juce::Path p;
            p.addCentredArc(centerX, centerY, (outerRadius+innerRadius)*0.5f, (outerRadius+innerRadius)*0.5f, 
                            0.0f, angle, angle + 0.03f, true);
            g.strokePath(p, juce::PathStrokeType(outerRadius - innerRadius));
        }

        // Draw cursor on the ring
        float cursorAngle = currentHue * juce::MathConstants<float>::twoPi;
        float cursorRadius = (outerRadius + innerRadius) * 0.5f;
        float cx = centerX + cursorRadius * std::sin(cursorAngle);
        float cy = centerY - cursorRadius * std::cos(cursorAngle);
        g.setColour(juce::Colours::white);
        g.drawEllipse(cx - 8, cy - 8, 16, 16, 2.0f);

        // Draw inner square
        juce::Rectangle<float> innerSquare(centerX - 45, centerY - 45, 90, 90);
        g.setColour(currentColor);
        g.fillRoundedRectangle(innerSquare, 8.0f);

        // Hex overlay on square
        g.setColour(currentColor.contrasting());
        g.drawText(currentColor.toDisplayString(true).toUpperCase(), 
                   innerSquare.withTrimmedTop(60).toNearestInt(), juce::Justification::centred, false);
    }
};

#include "support/HeadlessX11.h"

int main() {
    HeadlessX11::install();
    juce::ScopedJuceInitialiser_GUI guiInit;
    AdvancedColorPickerComponent picker(juce::Colour(0xff33A1FF));
    
    juce::File file("C:/Dev/TheKlangFarmer/screenshots/test_advanced_picker.png");
    file.deleteFile();
    juce::FileOutputStream stream(file);
    if (stream.openedOk()) {
        juce::PNGImageFormat png;
        png.writeImageToStream(picker.createComponentSnapshot(picker.getLocalBounds()), stream);
    }
    return 0;
}
