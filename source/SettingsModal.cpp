#include "SettingsModal.h"
#include <cmath>

// ==============================================================================
// 1. GearButton Implementation
// ==============================================================================
GearButton::GearButton()
    : juce::Button("SettingsButton")
{
    setTooltip(RlyehSound::ParameterManager::getInstance().getGlobalString("btn_settings_tooltip", "SETTINGS & ABOUT — Version updates, check for new releases, and build metadata."));
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void GearButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Background button rect
    juce::Colour bgCol = shouldDrawButtonAsDown ? juce::Colour(0xff232938)
                       : shouldDrawButtonAsHighlighted ? juce::Colour(0xff1e222c)
                       : juce::Colour(0xff181a20);
    g.setColour(bgCol);
    g.fillRoundedRectangle(bounds, 4.0f);

    juce::Colour borderCol = shouldDrawButtonAsHighlighted ? juce::Colour(0xff3a4358) : juce::Colour(0xff252936);
    g.setColour(borderCol);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    // Gear Icon
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();
    float outerR = 6.2f;
    float toothR = 8.0f;
    float innerR = 3.0f;
    int numTeeth = 6;

    juce::Path gearPath;
    float angleStep = juce::MathConstants<float>::twoPi / static_cast<float>(numTeeth);

    for (int i = 0; i < numTeeth; ++i) {
        float baseAngle = static_cast<float>(i) * angleStep;
        float a1 = baseAngle - angleStep * 0.22f;
        float a2 = baseAngle - angleStep * 0.12f;
        float a3 = baseAngle + angleStep * 0.12f;
        float a4 = baseAngle + angleStep * 0.22f;

        float x1 = cx + std::cos(a1) * outerR;
        float y1 = cy + std::sin(a1) * outerR;
        float x2 = cx + std::cos(a2) * toothR;
        float y2 = cy + std::sin(a2) * toothR;
        float x3 = cx + std::cos(a3) * toothR;
        float y3 = cy + std::sin(a3) * toothR;
        float x4 = cx + std::cos(a4) * outerR;
        float y4 = cy + std::sin(a4) * outerR;

        if (i == 0)
            gearPath.startNewSubPath(x1, y1);
        else
            gearPath.lineTo(x1, y1);

        gearPath.lineTo(x2, y2);
        gearPath.lineTo(x3, y3);
        gearPath.lineTo(x4, y4);
    }
    gearPath.closeSubPath();

    // Center hole
    juce::Path hole;
    hole.addEllipse(cx - innerR, cy - innerR, innerR * 2.0f, innerR * 2.0f);
    gearPath.setUsingNonZeroWinding(false);
    gearPath.addPath(hole);

    juce::Colour iconCol = shouldDrawButtonAsDown ? juce::Colour(0xff00d2ff)
                         : shouldDrawButtonAsHighlighted ? juce::Colours::white
                         : juce::Colour(0xff8b99a6);
    g.setColour(iconCol);
    g.fillPath(gearPath);
}

// ==============================================================================
// 2. UpdateBadgeButton Implementation
// ==============================================================================
UpdateBadgeButton::UpdateBadgeButton()
    : juce::Button("UpdateBadgeButton")
{
    setTooltip(RlyehSound::ParameterManager::getInstance().getGlobalString("badge_update_tooltip", "UPDATE AVAILABLE: Click to view and download the latest release on GitHub."));
    setMouseCursor(juce::MouseCursor::PointingHandCursor);

    VersionChecker::getInstance().addChangeListener(this);

    onClick = [this] {
        if (releaseUrl.isNotEmpty())
            juce::URL(releaseUrl).launchInDefaultBrowser();
    };

    changeListenerCallback(&VersionChecker::getInstance());
}

UpdateBadgeButton::~UpdateBadgeButton()
{
    VersionChecker::getInstance().removeChangeListener(this);
}

void UpdateBadgeButton::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    juce::ignoreUnused(source);
    auto info = VersionChecker::getInstance().getReleaseInfo();

    if (info.status == VersionChecker::Status::UpdateAvailable) {
        candidateVersion = info.latestTag;
        releaseUrl = info.releaseUrl;
        juce::String badgeText = candidateVersion.isNotEmpty() ? "UPDATE: " + candidateVersion : "UPDATE AVAILABLE";
        setButtonText(badgeText);
        setVisible(true);
    } else {
        setVisible(false);
    }
    repaint();
}

void UpdateBadgeButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    juce::Colour emerald(0xff00e676);
    juce::Colour bgCol = shouldDrawButtonAsDown ? emerald.withAlpha(0.35f)
                       : shouldDrawButtonAsHighlighted ? emerald.withAlpha(0.25f)
                       : emerald.withAlpha(0.15f);

    g.setColour(bgCol);
    g.fillRoundedRectangle(bounds, bounds.getHeight() * 0.5f);

    g.setColour(emerald);
    g.drawRoundedRectangle(bounds, bounds.getHeight() * 0.5f, 1.2f);

    // Glowing dot indicator
    float dotSize = 6.0f;
    float dotX = bounds.getX() + 8.0f;
    float dotY = bounds.getCentreY() - dotSize * 0.5f;
    g.setColour(emerald);
    g.fillEllipse(dotX, dotY, dotSize, dotSize);

    // Text
    g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    g.setColour(shouldDrawButtonAsHighlighted ? juce::Colours::white : emerald);
    auto textArea = bounds.withTrimmedLeft(18.0f).withTrimmedRight(6.0f);
    g.drawText(getButtonText(), textArea, juce::Justification::centredLeft, true);
}

// ==============================================================================
// 3. SettingsModalComponent Implementation
// ==============================================================================
SettingsModalComponent::SettingsModalComponent(const juce::String& name)
    : productName(name)
{
    VersionChecker::getInstance().addChangeListener(this);

    // Close button
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff232938));
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffffff));
    closeButton.onClick = [this] { setVisible(false); };
    addAndMakeVisible(closeButton);

    // Theme Engine Section
    themeLabel.setText("Active Theme:", juce::dontSendNotification);
    themeLabel.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    themeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc5d1e0));
    addAndMakeVisible(themeLabel);

    themeBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff161b22));
    themeBox.setColour(juce::ComboBox::textColourId, juce::Colour(0xffffffff));
    themeBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff283141));

    const auto availableThemes = RlyehSound::ParameterManager::getInstance().getAvailableThemes();
    int selectedThemeId = 1;
    for (size_t i = 0; i < availableThemes.size(); ++i) {
        int id = static_cast<int>(i) + 1;
        themeBox.addItem(availableThemes[i].name, id);
        if (availableThemes[i].id == RlyehSound::ParameterManager::getInstance().getActiveTheme()) {
            selectedThemeId = id;
        }
    }
    themeBox.setSelectedId(selectedThemeId, juce::dontSendNotification);
    themeBox.onChange = [this] {
        const auto themes = RlyehSound::ParameterManager::getInstance().getAvailableThemes();
        int selIdx = themeBox.getSelectedId() - 1;
        if (selIdx >= 0 && selIdx < static_cast<int>(themes.size())) {
            RlyehSound::ParameterManager::getInstance().loadTheme(themes[selIdx].id);
            if (auto* p = getTopLevelComponent()) p->repaint();
            repaint();
        }
    };
    addAndMakeVisible(themeBox);

    accentLabel.setText("Quick Accent:", juce::dontSendNotification);
    accentLabel.setFont(juce::FontOptions(10.5f, juce::Font::plain));
    accentLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8892a4));
    addAndMakeVisible(accentLabel);

    struct AccentDef { const char* name; juce::Colour col; const char* tooltip; };
    const AccentDef accents[] = {
        { "Cyan",    juce::Colour(0xff38bdf8), "Electric Cyan accent" },
        { "Amber",   juce::Colour(0xfff59e0b), "Solar Amber accent" },
        { "Emerald", juce::Colour(0xff10b981), "Acid Lime / Emerald accent" },
        { "Magenta", juce::Colour(0xffec4899), "Hot Magenta accent" },
        { "Coral",   juce::Colour(0xffff7043), "Coral accent" },
        { "Gold",    juce::Colour(0xfffacc15), "Gold accent" },
        { "Ice",     juce::Colour(0xff7dd3fc), "Arctic Ice Blue accent" }
    };

    for (const auto& a : accents) {
        auto btn = std::make_unique<SwatchButton>(a.name, a.col, a.tooltip);
        btn->onClick = [this, col = a.col] {
            RlyehSound::ParameterManager::getInstance().applyCustomTint(juce::Colours::transparentBlack, col);
            if (auto* p = getParentComponent()) p->repaint();
            repaint();
        };
        addAndMakeVisible(*btn);
        accentSwatches.push_back(std::move(btn));
    }

    bgLabel.setText("Chassis Tint:", juce::dontSendNotification);
    bgLabel.setFont(juce::FontOptions(10.5f, juce::Font::plain));
    bgLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8892a4));
    addAndMakeVisible(bgLabel);

    struct BgDef { const char* name; juce::Colour col; const char* tooltip; };
    const BgDef bgs[] = {
        { "Pure Black",   juce::Colour(0xff0a0d12), "Pure Black dark chassis" },
        { "Deep Slate",   juce::Colour(0xff18202c), "Deep Cosmic Slate chassis" },
        { "Abyssal Navy", juce::Colour(0xff0b192e), "Abyssal Navy Blue chassis" },
        { "Concrete",     juce::Colour(0xff242424), "Matte Concrete Grey chassis" }
    };

    for (const auto& b : bgs) {
        auto btn = std::make_unique<SwatchButton>(b.name, b.col, b.tooltip);
        btn->onClick = [this, col = b.col] {
            RlyehSound::ParameterManager::getInstance().applyCustomTint(col, juce::Colours::transparentBlack);
            if (auto* p = getParentComponent()) p->repaint();
            repaint();
        };
        addAndMakeVisible(*btn);
        backgroundSwatches.push_back(std::move(btn));
    }

    // Toggle
    checkOnLaunchToggle.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("settings_check_on_launch", "Check for updates automatically on startup"));
    checkOnLaunchToggle.setToggleState(VersionChecker::isCheckOnLaunchEnabled(), juce::dontSendNotification);
    checkOnLaunchToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffc5d1e0));
    checkOnLaunchToggle.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xff00e676));
    checkOnLaunchToggle.onClick = [this] {
        VersionChecker::setCheckOnLaunchEnabled(checkOnLaunchToggle.getToggleState());
    };
    addAndMakeVisible(checkOnLaunchToggle);

    // Check Now button
    checkNowButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("settings_btn_check_now", "CHECK FOR UPDATES"));
    checkNowButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e2433));
    checkNowButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff4a9eff));
    checkNowButton.onClick = [this] {
        VersionChecker::getInstance().checkForUpdates(true);
        updateStatusDisplay();
    };
    addAndMakeVisible(checkNowButton);

    // Download button
    downloadButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("settings_btn_download", "DOWNLOAD UPDATE"));
    downloadButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00894b));
    downloadButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    downloadButton.onClick = [this] {
        auto info = VersionChecker::getInstance().getReleaseInfo();
        if (info.releaseUrl.isNotEmpty())
            juce::URL(info.releaseUrl).launchInDefaultBrowser();
    };
    addChildComponent(downloadButton);

    // Status label
    statusLabel.setFont(juce::FontOptions(12.0f, juce::Font::plain));
    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8892a4));
    addAndMakeVisible(statusLabel);

    // Community / Links
    githubButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("settings_btn_github", "GITHUB REPOSITORY"));
    githubButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181b22));
    githubButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc5d1e0));
    githubButton.onClick = [] {
        juce::URL("https://github.com/codygratner/TheKlangSuite").launchInDefaultBrowser();
    };
    addAndMakeVisible(githubButton);

    issuesButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("settings_btn_issues", "REPORT AN ISSUE"));
    issuesButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181b22));
    issuesButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc5d1e0));
    issuesButton.onClick = [] {
        juce::URL("https://github.com/codygratner/TheKlangSuite/issues").launchInDefaultBrowser();
    };
    addAndMakeVisible(issuesButton);

    setWantsKeyboardFocus(true);
    updateStatusDisplay();
}

SettingsModalComponent::~SettingsModalComponent()
{
    VersionChecker::getInstance().removeChangeListener(this);
}

void SettingsModalComponent::setPluginName(const juce::String& name)
{
    productName = name;
    repaint();
}

void SettingsModalComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    juce::ignoreUnused(source);
    updateStatusDisplay();
    repaint();
}

void SettingsModalComponent::updateStatusDisplay()
{
    auto info = VersionChecker::getInstance().getReleaseInfo();

    switch (info.status) {
        case VersionChecker::Status::Checking:
            statusLabel.setText(RlyehSound::ParameterManager::getInstance().getGlobalString("settings_status_checking", "Checking GitHub for latest release..."), juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff4a9eff));
            downloadButton.setVisible(false);
            break;

        case VersionChecker::Status::UpToDate:
            statusLabel.setText(productName + " is up to date!", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00e676));
            downloadButton.setVisible(false);
            break;

        case VersionChecker::Status::UpdateAvailable:
            statusLabel.setText(RlyehSound::ParameterManager::getInstance().getGlobalString("settings_status_update_found", "A new release is available: ") + info.latestTag, juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffab00));
            downloadButton.setVisible(true);
            break;

        case VersionChecker::Status::Error:
            statusLabel.setText(info.errorMessage.isNotEmpty() ? info.errorMessage : RlyehSound::ParameterManager::getInstance().getGlobalString("settings_status_error", "Unable to connect to GitHub release API."), juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff5252));
            downloadButton.setVisible(false);
            break;

        case VersionChecker::Status::Idle:
        default:
            statusLabel.setText("No release check performed yet.", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8892a4));
            downloadButton.setVisible(false);
            break;
    }
}

juce::Rectangle<int> SettingsModalComponent::getCardBounds() const
{
    int cardW = std::min(620, getWidth() - 40);
    int cardH = std::min(510, getHeight() - 30);
    int cardX = (getWidth() - cardW) / 2;
    int cardY = (getHeight() - cardH) / 2;
    return juce::Rectangle<int>(cardX, cardY, cardW, cardH);
}

void SettingsModalComponent::resized()
{
    auto card = getCardBounds();
    closeButton.setBounds(card.getRight() - 36, card.getY() + 10, 26, 26);

    int contentX = card.getX() + 24;
    int contentW = card.getWidth() - 48;
    int currentY = card.getY() + 86;

    // 1. Theme Engine Section
    themeLabel.setBounds(contentX, currentY, 95, 24);
    themeBox.setBounds(contentX + 100, currentY, 240, 24);
    currentY += 30;

    accentLabel.setBounds(contentX, currentY, 95, 22);
    int swatchX = contentX + 100;
    int swatchW = 58;
    int swatchSpacing = 6;
    for (size_t i = 0; i < accentSwatches.size(); ++i) {
        accentSwatches[i]->setBounds(swatchX + static_cast<int>(i) * (swatchW + swatchSpacing), currentY, swatchW, 22);
    }
    currentY += 28;

    bgLabel.setBounds(contentX, currentY, 95, 22);
    int bgSwatchW = 86;
    int bgSwatchSpacing = 8;
    for (size_t i = 0; i < backgroundSwatches.size(); ++i) {
        backgroundSwatches[i]->setBounds(swatchX + static_cast<int>(i) * (bgSwatchW + bgSwatchSpacing), currentY, bgSwatchW, 22);
    }
    currentY += 36;

    // 2. Updates Section
    checkOnLaunchToggle.setBounds(contentX, currentY, contentW, 24);
    currentY += 28;

    checkNowButton.setBounds(contentX, currentY, 160, 26);
    downloadButton.setBounds(contentX + 172, currentY, 160, 26);
    currentY += 30;

    statusLabel.setBounds(contentX, currentY, contentW, 20);

    // 3. Links Section at bottom
    int linksY = card.getBottom() - 44;
    githubButton.setBounds(contentX, linksY, 170, 28);
    issuesButton.setBounds(contentX + 182, linksY, 170, 28);
}

void SettingsModalComponent::mouseDown(const juce::MouseEvent& e)
{
    if (!getCardBounds().contains(e.getPosition()))
        setVisible(false);
}

bool SettingsModalComponent::keyPressed(const juce::KeyPress& key)
{
    if (key.isKeyCode(juce::KeyPress::escapeKey)) {
        setVisible(false);
        return true;
    }
    return false;
}

void SettingsModalComponent::paint(juce::Graphics& g)
{
    // Dim background
    g.fillAll(juce::Colour(0xd00a0d14));

    auto card = getCardBounds().toFloat();

    // Card background
    g.setColour(juce::Colour(0xff12151c));
    g.fillRoundedRectangle(card, 8.0f);

    // Card border
    g.setColour(juce::Colour(0xff2a3245));
    g.drawRoundedRectangle(card.reduced(0.5f), 8.0f, 1.5f);

    // Top accent strip
    auto topStrip = card.removeFromTop(4.0f);
    g.setColour(RlyehSound::ParameterManager::getInstance().getGlobalColor("accent_cyan", juce::Colour(0xff00d2ff)));
    g.fillRoundedRectangle(topStrip, 2.0f);

    // Title & Subtitle
    g.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    g.drawText(RlyehSound::ParameterManager::getInstance().getGlobalString("settings_title", "SETTINGS & ABOUT"), static_cast<int>(card.getX()) + 24, static_cast<int>(card.getY()) + 10, 400, 24, juce::Justification::left, true);

    g.setFont(juce::FontOptions(12.0f, juce::Font::plain));
    g.setColour(juce::Colour(0xff8892a4));
    g.drawText(RlyehSound::ParameterManager::getInstance().getGlobalString("settings_subtitle", "Studio Palettes, Release Updates & Build Metadata"), static_cast<int>(card.getX()) + 24, static_cast<int>(card.getY()) + 34, 500, 18, juce::Justification::left, true);

    // Header divider line
    g.setColour(juce::Colour(0xff222736));
    g.drawHorizontalLine(static_cast<int>(card.getY()) + 56, card.getX() + 16.0f, card.getRight() - 16.0f);

    // Section 1 Header: STUDIO THEMES & COLOR PALETTES
    int sec1Y = static_cast<int>(card.getY()) + 66;
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.setColour(RlyehSound::ParameterManager::getInstance().getGlobalColor("accent_cyan", juce::Colour(0xff4a9eff)));
    g.drawText("STUDIO THEMES & COLOR PALETTES", static_cast<int>(card.getX()) + 24, sec1Y, 300, 16, juce::Justification::left, true);

    // Divider line 1
    int div1Y = static_cast<int>(card.getY()) + 172;
    g.setColour(juce::Colour(0xff222736));
    g.drawHorizontalLine(div1Y, card.getX() + 16.0f, card.getRight() - 16.0f);

    // Section 2 Header: UPDATES & RELEASES
    int sec2Y = div1Y + 8;
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.setColour(RlyehSound::ParameterManager::getInstance().getGlobalColor("accent_cyan", juce::Colour(0xff4a9eff)));
    g.drawText("UPDATES & RELEASES", static_cast<int>(card.getX()) + 24, sec2Y, 300, 16, juce::Justification::left, true);

    // Divider line 2
    int div2Y = div1Y + 116;
    g.setColour(juce::Colour(0xff222736));
    g.drawHorizontalLine(div2Y, card.getX() + 16.0f, card.getRight() - 16.0f);

    // Section 3 Header: BUILD & SYSTEM METADATA
    int sec3Y = div2Y + 8;
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.setColour(RlyehSound::ParameterManager::getInstance().getGlobalColor("accent_cyan", juce::Colour(0xff4a9eff)));
    g.drawText("BUILD & SYSTEM METADATA", static_cast<int>(card.getX()) + 24, sec3Y, 300, 16, juce::Justification::left, true);

    // Metadata entries
    int metaY = sec3Y + 20;
    int col1X = static_cast<int>(card.getX()) + 24;
    int col2X = static_cast<int>(card.getX()) + 310;

    auto drawMetaRow = [&](int x, int y, const juce::String& label, const juce::String& value) {
        g.setFont(juce::FontOptions(11.5f, juce::Font::plain));
        g.setColour(juce::Colour(0xff75849b));
        g.drawText(label + ":", x, y, 100, 18, juce::Justification::left, true);
        g.setColour(juce::Colour(0xffc5d1e0));
        g.drawText(value, x + 104, y, 180, 18, juce::Justification::left, true);
    };

#ifdef JucePlugin_VersionString
    juce::String verStr = "v" JucePlugin_VersionString;
#else
    juce::String verStr = "v0.3.0";
#endif
#ifdef TKF_FEATURE_TAG
    if (juce::String(TKF_FEATURE_TAG).isNotEmpty())
        verStr += juce::String(TKF_FEATURE_TAG);
#endif

    drawMetaRow(col1X, metaY, "Product", productName);
    drawMetaRow(col1X, metaY + 18, "Version", verStr);
    drawMetaRow(col1X, metaY + 36, "Framework", "JUCE 9.0.3");
    drawMetaRow(col1X, metaY + 54, "Language", "C++20");

    drawMetaRow(col2X, metaY, "Architecture", "64-bit");
    drawMetaRow(col2X, metaY + 18, "Build Date", __DATE__);
    drawMetaRow(col2X, metaY + 36, "License", "GPL-3.0 (FOSS)");
    drawMetaRow(col2X, metaY + 54, "Vendor", "The Cult of Klang");

    // Repository metadata row
    g.setFont(juce::FontOptions(11.5f, juce::Font::plain));
    g.setColour(juce::Colour(0xff75849b));
    g.drawText("Repository:", col1X, metaY + 72, 100, 18, juce::Justification::left, true);
    g.setColour(juce::Colour(0xff4a9eff));
    g.drawText("github.com/codygratner/TheKlangSuite", col1X + 104, metaY + 72, 400, 18, juce::Justification::left, true);

    // Bottom divider line
    int botY = static_cast<int>(card.getBottom()) - 54;
    g.setColour(juce::Colour(0xff222736));
    g.drawHorizontalLine(botY, card.getX() + 16.0f, card.getRight() - 16.0f);
}
