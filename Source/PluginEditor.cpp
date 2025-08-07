#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// WoolyLookAndFeel Implementation
//==============================================================================

WoolyLookAndFeel::WoolyLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, juce::Colour(0xFFD4A574));
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFF8B4513));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF2F1B14));
    setColour(juce::Label::textColourId, juce::Colour(0xFFF5DEB3));
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF654321));
    setColour(juce::ComboBox::textColourId, juce::Colour(0xFFF5DEB3));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFFD2B48C));
}

void WoolyLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                     const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    (void)slider; // Suppress unused parameter warning
    
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto centre = bounds.getCentre();
    
    // Fix the angle calculation - this was backwards before
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    // Load the custom knob image from BinaryData
    auto knobImage = juce::ImageCache::getFromMemory(BinaryData::harmonster_custom_knob_png, BinaryData::harmonster_custom_knob_pngSize);
    
    if (knobImage.isValid())
    {
        // Calculate exact integer positions to avoid floating point precision issues
        auto imageX = static_cast<int>(bounds.getX());
        auto imageY = static_cast<int>(bounds.getY());
        auto imageWidth = static_cast<int>(bounds.getWidth());
        auto imageHeight = static_cast<int>(bounds.getHeight());
        
        // Calculate the exact center using integer arithmetic for precision
        auto imageCentreX = imageX + imageWidth / 2.0f;
        auto imageCentreY = imageY + imageHeight / 2.0f;
        
        // Save the current graphics state
        juce::Graphics::ScopedSaveState saveState(g);
        
        // Apply rotation around the exact center
        g.addTransform(juce::AffineTransform::rotation(angle, imageCentreX, imageCentreY));
        
        // Draw the knob image at exact position
        g.drawImage(knobImage, imageX, imageY, imageWidth, imageHeight, 
                   0, 0, knobImage.getWidth(), knobImage.getHeight());
    }
    else
    {
        // Fallback: draw a simple circle if image fails to load
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f; // Minimal padding
        auto rx = centre.x - radius;
        auto ry = centre.y - radius;
        auto rw = radius * 2.0f;
        
        // Draw basic knob
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.fillEllipse(rx, ry, rw, rw);
        
        // Draw indicator line - fix angle calculation here too
        auto lineLength = radius * 0.8f;
        auto lineX = centre.x + std::cos(angle) * lineLength;
        auto lineY = centre.y + std::sin(angle) * lineLength;
        
        g.setColour(juce::Colour(0xFFFFF8DC));
        g.drawLine(centre.x, centre.y, lineX, lineY, 2.0f);
    }
}

void WoolyLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                 int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box)
{
    (void)isButtonDown; // Suppress unused parameter warning
    (void)box; // Suppress unused parameter warning
    
    // Draw outer shadow for depth
    g.setColour(juce::Colour(0x60000000));
    g.fillRoundedRectangle(1, 1, width - 1, height - 1, 6.0f);
    
    // Main button body with realistic gradient
    juce::ColourGradient buttonGradient(
        juce::Colour(0xFF2A2A2A), 0, 0,           // Lighter at top
        juce::Colour(0xFF0A0A0A), 0, height,      // Darker at bottom
        false);
    buttonGradient.addColour(0.1, juce::Colour(0xFF404040));  // Top highlight
    buttonGradient.addColour(0.9, juce::Colour(0xFF151515));  // Bottom shadow
    
    g.setGradientFill(buttonGradient);
    g.fillRoundedRectangle(0, 0, width, height, 6.0f);
    
    // Add brushed metal texture
    g.setColour(juce::Colour(0x08FFFFFF));
    for (int i = 0; i < height; i += 3)
    {
        g.drawHorizontalLine(i, 0, width);
    }
    
    // Outer rim with gradient
    juce::ColourGradient rimGradient(
        juce::Colour(0xFF505050), 0, 0,
        juce::Colour(0xFF202020), 0, height,
        false);
    g.setGradientFill(rimGradient);
    g.drawRoundedRectangle(0, 0, width, height, 6.0f, 2.0f);
    
    // Cyan accent border
    g.setColour(juce::Colour(0xFF00BFFF));
    g.drawRoundedRectangle(1, 1, width - 2, height - 2, 5.0f, 1.5f);
    
    // Inner glow
    g.setColour(juce::Colour(0x4000BFFF));
    g.drawRoundedRectangle(2, 2, width - 4, height - 4, 4.0f, 1.0f);
    
    // Draw realistic dropdown arrow with 3D effect
    auto arrowArea = juce::Rectangle<int>(buttonX, buttonY, buttonW, buttonH);
    auto centerX = arrowArea.getCentreX();
    auto centerY = arrowArea.getCentreY();
    auto arrowSize = 5.0f;
    
    // Arrow shadow
    g.setColour(juce::Colour(0x80000000));
    juce::Path shadowArrow;
    shadowArrow.addTriangle(centerX - arrowSize + 1, centerY - arrowSize/2 + 1,
                           centerX + arrowSize + 1, centerY - arrowSize/2 + 1,
                           centerX + 1, centerY + arrowSize/2 + 1);
    g.fillPath(shadowArrow);
    
    // Main arrow with gradient
    juce::ColourGradient arrowGradient(
        juce::Colour(0xFF00FFFF), centerX, centerY - arrowSize/2,
        juce::Colour(0xFF0088BB), centerX, centerY + arrowSize/2,
        false);
    g.setGradientFill(arrowGradient);
    
    juce::Path arrow;
    arrow.addTriangle(centerX - arrowSize, centerY - arrowSize/2,
                     centerX + arrowSize, centerY - arrowSize/2,
                     centerX, centerY + arrowSize/2);
    g.fillPath(arrow);
    
    // Arrow highlight
    g.setColour(juce::Colour(0xFFFFFFFF));
    g.strokePath(arrow, juce::PathStrokeType(0.5f));
    
    // Add subtle top highlight to entire button
    g.setColour(juce::Colour(0x20FFFFFF));
    g.fillRoundedRectangle(2, 2, width - 4, height * 0.3f, 4.0f);
}

void WoolyLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    (void)shouldDrawButtonAsHighlighted; // Suppress unused parameter warning
    (void)shouldDrawButtonAsDown; // Suppress unused parameter warning
    
    auto bounds = button.getLocalBounds().toFloat();
    bool isOn = button.getToggleState();
    
    // Load the appropriate power button image based on state
    auto buttonImage = isOn ? 
        juce::ImageCache::getFromMemory(BinaryData::poweron_png, BinaryData::poweron_pngSize) :
        juce::ImageCache::getFromMemory(BinaryData::poweroff_png, BinaryData::poweroff_pngSize);
    
    if (buttonImage.isValid())
    {
        // Draw the power button image scaled to fit the button bounds
        g.drawImage(buttonImage, bounds, 
                   juce::RectanglePlacement::centred | juce::RectanglePlacement::fillDestination);
    }
    else
    {
        // Fallback: simple circle if images fail to load
        auto centre = bounds.getCentre();
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;
        
        // Draw simple circle with color based on state
        g.setColour(isOn ? juce::Colour(0xFF00FF00) : juce::Colour(0xFF666666));
        g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2, radius * 2);
        
        // Draw border
        g.setColour(juce::Colour(0xFFCCCCCC));
        g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2, radius * 2, 2.0f);
    }
}

//==============================================================================
// WoolyMammothAudioProcessorEditor Implementation
//==============================================================================

WoolyMammothAudioProcessorEditor::WoolyMammothAudioProcessorEditor (WoolyMammothAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set the new HARMONSTER dimensions
    setSize (HarmonsterLayout::PLUGIN_WIDTH, HarmonsterLayout::PLUGIN_HEIGHT);
    setLookAndFeel(&woolyLF);

    // Title label removed - new background includes "THE HARMONSTER" text

    // Setup sliders with tooltips
    auto setupSlider = [this](juce::Slider& slider, const juce::String& tooltip, double defaultValue)
    {
        slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0); // No text box - values only show when dragging
        slider.setRange (0.0, 1.0, 0.001); // Finer resolution for smoother rotation
        slider.setValue (defaultValue);
        slider.addListener (this);
        slider.setTooltip (tooltip);
        
        // Optimize rotation sensitivity for smoother turning
        slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,        // Start angle (216 degrees)
                                  juce::MathConstants<float>::pi * 2.8f,        // End angle (504 degrees) 
                                  true);                                        // Stop at end
        
        slider.setMouseDragSensitivity(100); // Lower value = more sensitive/smoother
        slider.setVelocityBasedMode(true);   // Enable velocity-based dragging
        slider.setVelocityModeParameters(0.9, 1, 0.09, false); // Smoother velocity response
        
        addAndMakeVisible (&slider);
    };

    // Setup EQ knob (leftmost)
    setupSlider(eqSlider, "Adjusts the tone - counterclockwise for darker, clockwise for brighter", 0.5);
    
    // Setup SNARL knob (second from left - this was previously WOOL)
    setupSlider(snarlSlider, "Controls the bass roll-off and fuzz character - the original Woolly Mammoth WOOL control", 0.5);
    
    // Setup PINCH knob (third from left)
    setupSlider(pinchSlider, "Controls the fuzz intensity and gating behavior", 0.3);
    
    // Setup OUTPUT knob (rightmost)
    setupSlider(outputSlider, "Controls the overall output volume of the effect", 0.5);

    // Setup footswitch button
    footswitchButton.setButtonText("");
    footswitchButton.setTooltip("Click to bypass/enable the effect");
    footswitchButton.setToggleState(false, juce::dontSendNotification); // Default to ON (not bypassed)
    addAndMakeVisible(&footswitchButton);

    // Create parameter attachments for the 4 knobs
    eqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "eq", eqSlider);
    snarlAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "wool", snarlSlider);
    pinchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "pinch", pinchSlider);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "output", outputSlider);
    
    // Create parameter attachment for footswitch
    footswitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.parameters, "bypass", footswitchButton);

    // Size already set at beginning of constructor
}

WoolyMammothAudioProcessorEditor::~WoolyMammothAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void WoolyMammothAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    (void)slider; // Suppress unused parameter warning
    // All parameter updates are handled by the parameter attachments
    // This method can be left empty or used for additional UI updates
}

void WoolyMammothAudioProcessorEditor::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    (void)comboBoxThatHasChanged; // Suppress unused parameter warning
    // No combo boxes in HARMONSTER design
}

void WoolyMammothAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Load the HARMONSTER custom background image from BinaryData
    auto backgroundImage = juce::ImageCache::getFromMemory(BinaryData::harmonster_custom_ui_png, 
                                                          BinaryData::harmonster_custom_ui_pngSize);
    
    if (backgroundImage.isValid())
    {
        // Draw the HARMONSTER background image scaled to fit the plugin window
        // Scale from original to current window size (360x540)
        g.drawImage(backgroundImage, bounds.toFloat(), 
                   juce::RectanglePlacement::centred | juce::RectanglePlacement::fillDestination);
    }
    else
    {
        // Fallback: solid color background
        g.setColour(juce::Colour(0xFF2D1B3D)); // Dark purple fallback
        g.fillAll();
        
        // Error message
        g.setColour(juce::Colour(0xFFFFFFFF));
        g.setFont(juce::Font(juce::FontOptions(16.0f)));
        g.drawText("HARMONSTER", bounds, juce::Justification::centred);
        
        g.setFont(juce::Font(juce::FontOptions(12.0f)));
        auto messageArea = bounds.removeFromBottom(60);
        g.drawText("Custom background image could not be loaded from BinaryData", 
                   messageArea, juce::Justification::centred);
    }
}

void WoolyMammothAudioProcessorEditor::resized()
{
    using namespace HarmonsterLayout;
    
    // Position all knobs at exactly the same Y coordinate for perfect alignment
    const int knobY = KNOBS_Y;
    
    // EQ knob (leftmost)
    eqSlider.setBounds(EQ_X, knobY, KNOB_WIDTH, KNOB_HEIGHT);
    
    // SNARL knob (second from left)
    snarlSlider.setBounds(SNARL_X, knobY, KNOB_WIDTH, KNOB_HEIGHT);
    
    // PINCH knob (third from left)
    pinchSlider.setBounds(PINCH_X, knobY, KNOB_WIDTH, KNOB_HEIGHT);
    
    // OUTPUT knob (rightmost)
    outputSlider.setBounds(OUTPUT_X, knobY, KNOB_WIDTH, KNOB_HEIGHT);
    
    // Footswitch button (bottom center)
    footswitchButton.setBounds(FOOTSWITCH_X, FOOTSWITCH_Y, FOOTSWITCH_WIDTH, FOOTSWITCH_HEIGHT);
}