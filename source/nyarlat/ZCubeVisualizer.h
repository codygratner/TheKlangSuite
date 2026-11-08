#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <cmath>
#include "NyarlatProcessor.h"

namespace Nyarlat {

// Renders an isometric 3D wireframe cube of Dave Rossum's 8-corner Z-Plane parameter space
// with live glowing fireflies for active polyphonic voices
class ZCubeVisualizer : public juce::Component, private juce::Timer {
public:
    ZCubeVisualizer(NyarlatProcessor& proc) : processor_(proc) {
        startTimerHz(30);
    }

    ~ZCubeVisualizer() override {
        stopTimer();
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat().reduced(4.0f);

        // Dark recessed cavity
        g.setColour(juce::Colour(0xFF0B0F14));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF21262D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY() + 4.0f;
        float s = std::min(bounds.getWidth(), bounds.getHeight()) * 0.32f;

        // Isometric projection: 3D (x,y,z) in [-1, 1] to 2D screen
        auto projectIso = [cx, cy, s](float x, float y, float z) -> juce::Point<float> {
            // Standard 30-degree isometric projection
            float px = cx + (x - z) * s * 0.8660254f;
            float py = cy + (x + z) * s * 0.5f - y * s;
            return { px, py };
        };

        // Draw 8 Corners of the Unit Cube
        std::array<juce::Point<float>, 8> corners = {{
            projectIso(-1, -1, -1), projectIso( 1, -1, -1),
            projectIso(-1,  1, -1), projectIso( 1,  1, -1),
            projectIso(-1, -1,  1), projectIso( 1, -1,  1),
            projectIso(-1,  1,  1), projectIso( 1,  1,  1)
        }};

        // Draw Cube Edges
        g.setColour(juce::Colour(0xFF38434F));
        auto drawEdge = [&](int i, int j) {
            g.drawLine(corners[i].x, corners[i].y, corners[j].x, corners[j].y, 1.2f);
        };

        drawEdge(0, 1); drawEdge(1, 3); drawEdge(3, 2); drawEdge(2, 0); // Front face
        drawEdge(4, 5); drawEdge(5, 7); drawEdge(7, 6); drawEdge(6, 4); // Back face
        drawEdge(0, 4); drawEdge(1, 5); drawEdge(2, 6); drawEdge(3, 7); // Connecting edges

        // Corner Vertices
        g.setColour(juce::Colour(0xFF00C8F0));
        for (const auto& pt : corners) {
            g.fillEllipse(pt.x - 2.5f, pt.y - 2.5f, 5.0f, 5.0f);
        }

        // Draw Polyphonic Voice Fireflies
        auto telemetry = processor_.getVoiceTelemetry();
        for (int v = 0; v < NyarlatProcessor::kNumVoices; ++v) {
            if (!telemetry[v].active) continue;

            // Map [0..1] to [-1..1]
            float vx = telemetry[v].x * 2.0f - 1.0f;
            float vy = telemetry[v].y * 2.0f - 1.0f;
            float vz = telemetry[v].z * 2.0f - 1.0f;

            auto pt = projectIso(vx, vy, vz);

            // Glowing halo
            g.setColour(juce::Colour(0x40FF7A1A));
            g.fillEllipse(pt.x - 8.0f, pt.y - 8.0f, 16.0f, 16.0f);

            // Core bright firefly
            g.setColour(juce::Colour(0xFFFF7A1A));
            g.fillEllipse(pt.x - 3.5f, pt.y - 3.5f, 7.0f, 7.0f);
        }
    }

private:
    void timerCallback() override {
        repaint();
    }

    NyarlatProcessor& processor_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZCubeVisualizer)
};

} // namespace Nyarlat
