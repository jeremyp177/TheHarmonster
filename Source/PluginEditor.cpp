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
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto centre = bounds.getCentre();
    
    // Fix the angle calculation - this was backwards before
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    // Load the knob image from BinaryData
    auto knobImage = juce::ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);
    
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
    auto bounds = button.getLocalBounds().toFloat();
    auto centre = bounds.getCentre();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;
    
    bool isOn = button.getToggleState();
    
    // Real guitar pedal footswitch design - button appearance doesn't change with state
    
    // 1. Deep shadow for 3D effect
    g.setColour(juce::Colour(0x80000000));
    g.fillEllipse(centre.x - radius + 2, centre.y - radius + 2, radius * 2, radius * 2);
    
    // 2. Outer metallic ring (chrome bezel) - always the same
    juce::ColourGradient outerRing = juce::ColourGradient(
        juce::Colour(0xFFB8C6DB), centre.x, centre.y - radius,     // Light chrome at top
        juce::Colour(0xFF6C7B7F), centre.x, centre.y + radius,     // Darker chrome at bottom
        false);
    outerRing.addColour(0.3, juce::Colour(0xFFD4D4D4));            // Bright chrome highlight
    outerRing.addColour(0.7, juce::Colour(0xFF8C8C8C));            // Chrome lowlight
    
    g.setGradientFill(outerRing);
    g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2, radius * 2);
    
    // 3. Main button surface (smaller than outer ring) - always metallic, never changes
    auto buttonRadius = radius * 0.75f;
    
    // Button always looks the same - classic metallic finish
    juce::ColourGradient buttonGradient = juce::ColourGradient(
        juce::Colour(0xFFE8E8E8), centre.x, centre.y - buttonRadius,     // Light metallic at top
        juce::Colour(0xFF999999), centre.x, centre.y + buttonRadius,     // Darker at bottom
        false);
    buttonGradient.addColour(0.5, juce::Colour(0xFFCCCCCC));            // Mid metallic
    
    g.setGradientFill(buttonGradient);
    g.fillEllipse(centre.x - buttonRadius, centre.y - buttonRadius, buttonRadius * 2, buttonRadius * 2);
    
    // 4. Realistic brushed metal texture
    g.setColour(juce::Colour(0x15FFFFFF));
    for (int i = -static_cast<int>(buttonRadius * 0.8f); i < static_cast<int>(buttonRadius * 0.8f); i += 2)
    {
        auto y = centre.y + i;
        auto halfWidth = std::sqrt(buttonRadius * buttonRadius - i * i) * 0.8f;
        auto opacity = 0.6f - (std::abs(i) / buttonRadius) * 0.4f;
        
        g.setOpacity(opacity);
        g.drawHorizontalLine(static_cast<int>(y), centre.x - halfWidth, centre.x + halfWidth);
    }
    g.setOpacity(1.0f);
    
    // 5. Concentric circles for machined look
    for (float r = buttonRadius * 0.2f; r < buttonRadius * 0.9f; r += buttonRadius * 0.15f)
    {
        g.setColour(juce::Colour(0x12FFFFFF));
        g.drawEllipse(centre.x - r, centre.y - r, r * 2, r * 2, 0.5f);
    }
    
    // 6. Realistic edge highlight and shadow
    // Top highlight
    g.setColour(juce::Colour(0x40FFFFFF));
    g.drawEllipse(centre.x - buttonRadius, centre.y - buttonRadius, buttonRadius * 2, buttonRadius * 2, 1.5f);
    
    // Bottom shadow
    g.setColour(juce::Colour(0x30000000));
    auto shadowOffset = buttonRadius * 0.1f;
    g.drawEllipse(centre.x - buttonRadius + shadowOffset, centre.y - buttonRadius + shadowOffset, 
                 buttonRadius * 2, buttonRadius * 2, 1.0f);
    
    // 7. Subtle wear marks (realistic pedal use)
    g.setColour(juce::Colour(0x10000000));
    for (int i = 0; i < 5; i++)
    {
        auto angle = juce::MathConstants<float>::pi * 0.4f + i * 0.2f;
        auto wearRadius = buttonRadius * 0.6f;
        auto x = centre.x + std::cos(angle) * wearRadius;
        auto y = centre.y + std::sin(angle) * wearRadius;
        auto size = 1.0f + i * 0.3f;
        g.fillEllipse(x - size, y - size, size * 2, size * 2);
    }
    
    // 8. Separate LED indicator positioned above the footswitch (like real pedals)
    auto ledSize = radius * 0.25f;
    auto ledX = centre.x - ledSize * 0.5f;
    auto ledY = centre.y - radius * 1.3f; // Position above the footswitch
    
    // LED housing (black plastic housing like real 3mm/5mm LEDs)
    g.setColour(juce::Colour(0xFF1A1A1A));
    g.fillEllipse(ledX - ledSize * 0.2f, ledY - ledSize * 0.2f, ledSize * 1.4f, ledSize * 1.4f);
    
    if (isOn)
    {
        // LED ON state - bright red
        
        // Outer glow
        g.setColour(juce::Colour(0x60FF3030));
        g.fillEllipse(ledX - ledSize * 0.8f, ledY - ledSize * 0.8f, ledSize * 2.6f, ledSize * 2.6f);
        
        // Medium glow
        g.setColour(juce::Colour(0x80FF4040));
        g.fillEllipse(ledX - ledSize * 0.4f, ledY - ledSize * 0.4f, ledSize * 1.8f, ledSize * 1.8f);
        
        // LED core (bright red)
        juce::ColourGradient ledGradient = juce::ColourGradient(
            juce::Colour(0xFFFF6060), ledX, ledY - ledSize * 0.3f,     // Lighter red at top
            juce::Colour(0xFFDD0000), ledX, ledY + ledSize * 0.7f,     // Darker red at bottom
            false);
        ledGradient.addColour(0.5, juce::Colour(0xFFFF3030));          // Bright red middle
        
        g.setGradientFill(ledGradient);
        g.fillEllipse(ledX, ledY, ledSize, ledSize);
        
        // LED highlight (like real LED lens)
        g.setColour(juce::Colour(0xAAFFAAAA)); // Slight white highlight
        g.fillEllipse(ledX + ledSize * 0.2f, ledY + ledSize * 0.15f, ledSize * 0.3f, ledSize * 0.3f);
    }
    else
    {
        // LED OFF state - dark with subtle reflection
        
        // Dark LED (off)
        g.setColour(juce::Colour(0xFF2A0A0A)); // Very dark red
        g.fillEllipse(ledX, ledY, ledSize, ledSize);
        
        // Subtle reflection on dark LED
        g.setColour(juce::Colour(0x40FFFFFF));
        g.fillEllipse(ledX + ledSize * 0.2f, ledY + ledSize * 0.15f, ledSize * 0.25f, ledSize * 0.25f);
    }
    
    // LED housing rim (like real LED components)
    g.setColour(juce::Colour(0xFF333333));
    g.drawEllipse(ledX - ledSize * 0.1f, ledY - ledSize * 0.1f, ledSize * 1.2f, ledSize * 1.2f, 1.0f);
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

    // Setup title label
    titleLabel.setText ("THE HARMONSTER", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions ("Arial", 24.0f, juce::Font::bold)));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour(0xFFFF8C00)); // Orange
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (&titleLabel);

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
    // All parameter updates are handled by the parameter attachments
    // This method can be left empty or used for additional UI updates
}

void WoolyMammothAudioProcessorEditor::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    // No combo boxes in HARMONSTER design
}

void WoolyMammothAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Load the HARMONSTER background image from BinaryData
    auto backgroundImage = juce::ImageCache::getFromMemory(BinaryData::harmonster_puringerdsp_png, 
                                                          BinaryData::harmonster_puringerdsp_pngSize);
    
    if (backgroundImage.isValid())
    {
        // Draw the HARMONSTER background image scaled to fit the plugin window
        // Scale from original 1024x1536 to current window size (360x540)
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
        g.drawText("Background image could not be loaded from BinaryData", 
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