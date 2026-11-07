#include "ShoggothEditor.h"
#include <random>

namespace Shoggoth {

// ==============================================================================
// ShoggothSpectrumCanvas Implementation
// ==============================================================================
ShoggothSpectrumCanvas::ShoggothSpectrumCanvas(ShoggothProcessor& proc)
    : processor_(proc)
{
    saveUndoSnapshot();
}

void ShoggothSpectrumCanvas::saveUndoSnapshot() {
    SpectrumSnapshot snap;
    for (int k = 0; k < 63; ++k) {
        snap.s1[k] = processor_.getHarmonicLevel(0, k);
        snap.s2[k] = processor_.getHarmonicLevel(1, k);
    }
    if (undoBuffer_.size() >= 16) {
        undoBuffer_.erase(undoBuffer_.begin());
    }
    undoBuffer_.push_back(snap);
}

void ShoggothSpectrumCanvas::undo() {
    if (undoBuffer_.size() > 1) {
        undoBuffer_.pop_back();
        const auto& snap = undoBuffer_.back();
        for (int k = 0; k < 63; ++k) {
            processor_.setHarmonicLevel(0, k, snap.s1[k]);
            processor_.setHarmonicLevel(1, k, snap.s2[k]);
        }
        repaint();
    }
}

void ShoggothSpectrumCanvas::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(4.0f);

    // Card background
    g.setColour(juce::Colour(0xFF0D1117));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(0xFF30363D));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    // Color definitions for DHG envelopes
    const juce::Colour envColours[4] = {
        juce::Colour(0xFF00E5FF), // Env 0: Cyan
        juce::Colour(0xFFFFB300), // Env 1: Amber
        juce::Colour(0xFFD500F9), // Env 2: Violet
        juce::Colour(0xFF00E676)  // Env 3: Emerald
    };

    const float barW = (bounds.getWidth() - 8.0f) / 63.0f;
    const float bottomY = bounds.getBottom() - 4.0f;
    const float maxH = bounds.getHeight() - 16.0f;

    for (int k = 0; k < 63; ++k) {
        const float lvl = processor_.getHarmonicLevel(activeSource_, k);
        const float barH = (lvl / 99.0f) * maxH;
        const float x = bounds.getX() + 4.0f + static_cast<float>(k) * barW;
        const float y = bottomY - barH;

        const uint8_t envIdx = processor_.getHarmonicEnvAssign(k) & 0x03;
        g.setColour(envColours[envIdx]);

        if (barH > 1.0f) {
            g.fillRect(x + 0.5f, y, barW - 1.0f, barH);
        } else {
            g.setColour(juce::Colour(0x40FFFFFF));
            g.fillRect(x + 0.5f, bottomY - 1.0f, barW - 1.0f, 1.0f);
        }
    }

    // Source Label
    g.setColour(juce::Colour(0xFF8B949E));
    g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    g.drawText(activeSource_ == 0 ? "SOURCE 1 (HARMONICS 1-63)" : "SOURCE 2 (HARMONICS 1-63)",
               bounds.getX() + 8.0f, bounds.getY() + 4.0f, 200.0f, 16.0f, juce::Justification::left);
}

void ShoggothSpectrumCanvas::resized() {}

void ShoggothSpectrumCanvas::mouseDown(const juce::MouseEvent& e) {
    if (e.mods.isPopupMenu()) {
        juce::PopupMenu m;
        m.addItem(1, "Sawtooth (1/k)");
        m.addItem(2, "Square (Odd 1/k)");
        m.addItem(3, "Triangle (1/k^2)");
        m.addItem(4, "Pluck / Acoustic (1/k^1.5)");
        m.addItem(5, "Hollow Comb");
        m.addSeparator();
        m.addItem(6, "Clear All");
        m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
                        [this](int result) {
                            if (result == 1) applyTemplate("Saw");
                            else if (result == 2) applyTemplate("Square");
                            else if (result == 3) applyTemplate("Triangle");
                            else if (result == 4) applyTemplate("Pluck");
                            else if (result == 5) applyTemplate("Comb");
                            else if (result == 6) applyTemplate("Clear");
                        });
        return;
    }
    updateHarmonicFromMouse(e.x, e.y);
}

void ShoggothSpectrumCanvas::mouseDrag(const juce::MouseEvent& e) {
    updateHarmonicFromMouse(e.x, e.y);
}

void ShoggothSpectrumCanvas::mouseUp(const juce::MouseEvent&) {
    saveUndoSnapshot();
}

void ShoggothSpectrumCanvas::updateHarmonicFromMouse(int mouseX, int mouseY) {
    auto bounds = getLocalBounds().toFloat().reduced(4.0f);
    const float barW = (bounds.getWidth() - 8.0f) / 63.0f;
    const int k = static_cast<int>((static_cast<float>(mouseX) - (bounds.getX() + 4.0f)) / barW);

    if (k >= 0 && k < 63) {
        const float bottomY = bounds.getBottom() - 4.0f;
        const float maxH = bounds.getHeight() - 16.0f;
        const float normH = std::clamp((bottomY - static_cast<float>(mouseY)) / maxH, 0.0f, 1.0f);
        processor_.setHarmonicLevel(activeSource_, k, normH * 99.0f);
        repaint();
    }
}

void ShoggothSpectrumCanvas::applyTemplate(const juce::String& templateName) {
    saveUndoSnapshot();
    for (int k = 0; k < 63; ++k) {
        const float harm = static_cast<float>(k + 1);
        float val = 0.0f;
        if (templateName == "Saw") {
            val = 99.0f / harm;
        } else if (templateName == "Square") {
            val = (k % 2 == 0) ? (99.0f / harm) : 0.0f;
        } else if (templateName == "Triangle") {
            val = (k % 2 == 0) ? (99.0f / (harm * harm)) : 0.0f;
        } else if (templateName == "Pluck") {
            val = 99.0f / std::pow(harm, 1.5f);
        } else if (templateName == "Comb") {
            val = 99.0f * (0.5f * (1.0f + std::cos(3.14159f * harm * 0.25f)));
        } else if (templateName == "Clear") {
            val = (k == 0) ? 99.0f : 0.0f;
        }
        processor_.setHarmonicLevel(activeSource_, k, val);
    }
    repaint();
}

// ==============================================================================
// ShoggothEditor Implementation
// ==============================================================================
ShoggothEditor::ShoggothEditor(ShoggothProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      presetSelector_(p, [this]() {
          spectrumCanvas_.repaint();
          envelopeDisplay_.repaint();
      }),
      lifeModeToggle_({ { "SYNTHETIC", "CYBORG", "ORGANIC" } }),
      spectrumCanvas_(p),
      envelopeDisplay_(p)
{
    setLookAndFeel(&lnf_);

    // Title Labels
    titleLabel.setText("SHOGGOTH", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions(20.0f).withStyle("Bold")));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF00FFCC));
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("CRACKED CHIMES", juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font(juce::FontOptions(9.5f).withStyle("Bold")));
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF8B949E));
    addAndMakeVisible(subtitleLabel);

    // Header Capsules
    addAndMakeVisible(presetSelector_);

    // Triad Lifeform Rocker Toggle
    auto& ap = audioProcessor.getAPVTS();
    if (auto* param = ap.getParameter(ShoggothParameters::Ids::life_mode)) {
        int initialIdx = std::clamp(static_cast<int>(param->convertFrom0to1(param->getValue())), 0, 2);
        lifeModeToggle_.setIndex(initialIdx, juce::dontSendNotification);
        lifeModeToggle_.onChange = [&ap](int idx) {
            if (auto* p = ap.getParameter(ShoggothParameters::Ids::life_mode)) {
                p->setValueNotifyingHost(p->convertTo0to1(static_cast<float>(idx)));
            }
        };
    }
    addAndMakeVisible(lifeModeToggle_);

    addAndMakeVisible(headerInspector_);

    addAndMakeVisible(spectrumCanvas_);
    addAndMakeVisible(envelopeDisplay_);

    bindControls();

    setSize(1160, 680);
    startTimerHz(30);
}

ShoggothEditor::~ShoggothEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void ShoggothEditor::setupKnob(RotaryKnob& item, const juce::String& paramId, const juce::String& label, juce::Colour col) {
    item.paramId = paramId;

    // Top-mounted Bold Label
    item.label.setText(label, juce::dontSendNotification);
    item.label.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
    item.label.setJustificationType(juce::Justification::centred);
    item.label.setColour(juce::Label::textColourId, juce::Colour(0xFFF0F6FC));
    item.label.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(item.label);

    // Rotary Slider with Fleet LookAndFeel
    item.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    item.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    item.slider.setColour(juce::Slider::rotarySliderFillColourId, col);
    item.slider.setColour(juce::Slider::thumbColourId, col);
    addAndMakeVisible(item.slider);

    auto& ap = audioProcessor.getAPVTS();
    if (auto* p = ap.getParameter(paramId)) {
        float defaultVal = p->convertFrom0to1(p->getDefaultValue());
        item.slider.setDoubleClickReturnValue(true, defaultVal);
        item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(ap, paramId, item.slider);
    }

    // Engineering Unit Formatters for Kilohearts Value Badges
    if (paramId.contains("cutoff")) {
        item.slider.textFromValueFunction = [](double val) {
            return val >= 1000.0 ? (juce::String(val / 1000.0, 1) + " kHz") : (juce::String(std::round(val), 0) + " Hz");
        };
    } else if (paramId.contains("reso")) {
        item.slider.textFromValueFunction = [](double val) {
            return juce::String(std::round(val * 100.0), 0) + "%";
        };
    } else if (paramId == ShoggothParameters::Ids::spectral_tilt) {
        item.slider.textFromValueFunction = [](double val) {
            return (val > 0.0 ? "+" : "") + juce::String(val, 1) + " dB/oct";
        };
        item.slider.setDoubleClickReturnValue(true, 0.0);
    } else if (paramId == ShoggothParameters::Ids::odd_even_bal) {
        item.slider.textFromValueFunction = [](double val) {
            if (std::abs(val) < 0.02) return juce::String("CENTER");
            return val > 0.0 ? ("EVEN " + juce::String(std::round(val * 100.0), 0) + "%")
                             : ("ODD " + juce::String(std::round(-val * 100.0), 0) + "%");
        };
        item.slider.setDoubleClickReturnValue(true, 0.0);
    } else if (paramId == ShoggothParameters::Ids::dissonance_warp) {
        item.slider.textFromValueFunction = [](double val) {
            return juce::String(val, 2) + " alpha";
        };
        item.slider.setDoubleClickReturnValue(true, 1.0);
    } else if (paramId.contains("attack") || paramId.contains("decay") || paramId.contains("release")) {
        item.slider.textFromValueFunction = [](double val) {
            return val < 1.0 ? (juce::String(std::round(val * 1000.0), 0) + " ms") : (juce::String(val, 2) + " s");
        };
    } else if (paramId.contains("sustain") || paramId.contains("morph") || paramId.contains("mix") || paramId.contains("amount")) {
        item.slider.textFromValueFunction = [](double val) {
            return juce::String(std::round(val * 100.0), 0) + "%";
        };
    }
}

void ShoggothEditor::bindControls() {
    size_t i = 0;
    for (const auto& ctrl : ShoggothInfo::kControls) {
        juce::String paramIdStr(ctrl.paramId.data(), ctrl.paramId.size());
        juce::String labelStr(ctrl.name.data(), ctrl.name.size());

        auto item = std::make_unique<RotaryKnob>();
        juce::Colour col = (i < 16) ? juce::Colour(0xFF00FFCC) : juce::Colour(0xFFFF9900);
        setupKnob(*item, paramIdStr, labelStr, col);
        knobs_.push_back(std::move(item));
        ++i;
    }
}

void ShoggothEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF06090E));

    auto drawCard = [&](juce::Graphics& gTarget, juce::Rectangle<float> r, const juce::String& title, juce::Colour badgeCol, int badgeWidth = 160) {
        gTarget.setColour(juce::Colour(0xFF161B22));
        gTarget.fillRoundedRectangle(r, 6.0f);
        gTarget.setColour(juce::Colour(0xFF30363D));
        gTarget.drawRoundedRectangle(r, 6.0f, 1.0f);

        if (title.isNotEmpty()) {
            float bw = static_cast<float>(badgeWidth);
            auto badge = juce::Rectangle<float>(r.getX() + 10.0f, r.getY() - 9.0f, bw, 18.0f);
            gTarget.setColour(badgeCol);
            gTarget.fillRoundedRectangle(badge, 4.0f);
            gTarget.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
            gTarget.setColour(juce::Colour(0xFF0D1117));
            gTarget.drawText(title, badge, juce::Justification::centred);
        }
    };

    // Draw Modern Fleet Module Cards
    drawCard(g, juce::Rectangle<float>(14, 44, 560, 622), "SPECTRAL HARMONICS & TIMBRE", juce::Colour(0xFF00FFCC), 220);
    drawCard(g, juce::Rectangle<float>(586, 44, 560, 622), "ENVELOPES, FILTERS & MODULATION", juce::Colour(0xFFFF9900), 240);

    // File Drag Highlight Border
    if (isDraggingFile_) {
        g.setColour(juce::Colour(0xFF00FFCC));
        g.drawRect(getLocalBounds(), 3);
    }
}

void ShoggothEditor::resized() {
    // 4-Capsule Header Strip Geometry (1160px canvas)
    titleLabel.setBounds        (14,  4, 120, 20);
    subtitleLabel.setBounds     (14, 24, 120, 12);
    presetSelector_.setBounds   (140, 8, 288, 28);
    lifeModeToggle_.setBounds   (436, 8, 185, 28);
    headerInspector_.setBounds  (629, 8, 517, 28);

    // Visualizers inside Cards
    spectrumCanvas_.setBounds   (24, 76, 540, 200);
    envelopeDisplay_.setBounds  (596, 76, 540, 200);

    // Knobs inside Card 1 (16 controls in 4x4 grid below canvas)
    const int startY = 290;
    const int cellW = 132;
    const int cellH = 80;

    for (size_t i = 0; i < 16 && i < knobs_.size(); ++i) {
        const int col = static_cast<int>(i % 4);
        const int row = static_cast<int>(i / 4);
        const int x = 24 + col * (cellW + 4);
        const int y = startY + row * cellH;

        knobs_[i]->label.setBounds(x, y, cellW, 16);
        knobs_[i]->slider.setBounds(x + (cellW - 56) / 2, y + 18, 56, 56);
    }

    // Knobs inside Card 2 (16 controls in 4x4 grid below envelope)
    for (size_t i = 16; i < 32 && i < knobs_.size(); ++i) {
        const int idx = static_cast<int>(i - 16);
        const int col = idx % 4;
        const int row = idx / 4;
        const int x = 596 + col * (cellW + 4);
        const int y = startY + row * cellH;

        knobs_[i]->label.setBounds(x, y, cellW, 16);
        knobs_[i]->slider.setBounds(x + (cellW - 56) / 2, y + 18, 56, 56);
    }
}

void ShoggothEditor::timerCallback() {
    auto& ap = audioProcessor.getAPVTS();
    const float fEnv = *ap.getRawParameterValue(ShoggothParameters::Ids::filter_env_depth);
    const float chaos = *ap.getRawParameterValue(ShoggothParameters::Ids::chaos_amount);

    for (auto& k : knobs_) {
        if (!k) continue;
        if (k->paramId == ShoggothParameters::Ids::filter_cutoff) {
            auto& sp = k->slider.getProperties();
            const float kNorm = static_cast<float>(k->slider.valueToProportionOfLength(k->slider.getValue()));
            if (std::abs(fEnv) > 0.001f) {
                const float tgt = std::clamp(kNorm + fEnv * (1.0f - kNorm), 0.0f, 1.0f);
                sp.set("modLo", std::min(kNorm, tgt));
                sp.set("modHi", std::max(kNorm, tgt));
                sp.set("modPip", tgt);
            } else {
                sp.remove("modLo");
                sp.remove("modHi");
                sp.remove("modPip");
            }
            if (chaos > 0.001f) {
                const float lo = std::clamp(kNorm - chaos * 0.5f, 0.0f, 1.0f);
                const float hi = std::clamp(kNorm + chaos * 0.5f, 0.0f, 1.0f);
                sp.set("randLo", lo);
                sp.set("randHi", hi);
                sp.set("randPip", kNorm);
            } else {
                sp.remove("randLo");
                sp.remove("randHi");
                sp.remove("randPip");
            }
            k->slider.repaint();
        }
    }
}

// ==============================================================================
// FileDragAndDropTarget
// ==============================================================================
bool ShoggothEditor::isInterestedInFileDrag(const juce::StringArray& files) {
    for (const auto& f : files) {
        const auto ext = juce::File(f).getFileExtension().toLowerCase();
        if (ext == ".syx" || ext == ".mid" || ext == ".wav" || ext == ".aif" || ext == ".flac" || ext == ".vitaltable")
            return true;
    }
    return false;
}

void ShoggothEditor::fileDragEnter(const juce::StringArray&, int, int) {
    isDraggingFile_ = true;
    repaint();
}

void ShoggothEditor::fileDragExit(const juce::StringArray&) {
    isDraggingFile_ = false;
    repaint();
}

void ShoggothEditor::filesDropped(const juce::StringArray& files, int, int) {
    isDraggingFile_ = false;
    for (const auto& f : files) {
        const juce::File file(f);
        const auto ext = file.getFileExtension().toLowerCase();
        if (ext == ".syx" || ext == ".mid") {
            audioProcessor.loadSysExFile(file);
            spectrumCanvas_.repaint();
            break;
        } else if (ext == ".wav" || ext == ".aif" || ext == ".flac" || ext == ".vitaltable") {
            audioProcessor.loadAudioFile(file);
            spectrumCanvas_.repaint();
            break;
        }
    }
    repaint();
}

} // namespace Shoggoth
