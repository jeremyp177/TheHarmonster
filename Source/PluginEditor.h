#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "BinaryData.h"

//==============================================================================
// Layout Constants for HARMONSTER Stomp Box Design
//==============================================================================
namespace HarmonsterLayout
{
    // Plugin dimensions (matching 1024x1536 background ratio, scaled to ~35% for DAW)
    static constexpr int PLUGIN_WIDTH = 360;
    static constexpr int PLUGIN_HEIGHT = 540;
    
    // Knob positioning - smaller knobs for better fit
    static constexpr int KNOB_WIDTH = 65;  // Reduced from 70 for better spacing
    static constexpr int KNOB_HEIGHT = 65; // Reduced from 70 for better spacing
    static constexpr int LABEL_HEIGHT = 20;
    
    // LIVE-TUNING: Einfach diese Werte ändern für schnelle Anpassungen
    static constexpr int KNOBS_TOP_MARGIN = 78;  // ↕️ Vertikale Position aller Knöpfe
    
    // 🎛️ EINZELNE KNOPF-POSITIONEN (einfach änderbar):
    static constexpr int EQ_X = 40;      // 🎚️ EQ Knopf X-Position
    static constexpr int SNARL_X = 110;  // 🎚️ SNARL Knopf X-Position  
    static constexpr int PINCH_X = 180;  // 🎚️ PINCH Knopf X-Position
    static constexpr int OUTPUT_X = 255; // 🎚️ OUTPUT Knopf X-Position
    static constexpr int KNOBS_Y = KNOBS_TOP_MARGIN;
    
    // Footswitch button positioning (aligned with power button in background image)
    static constexpr int FOOTSWITCH_WIDTH = 70;
    static constexpr int FOOTSWITCH_HEIGHT = 70;
    static constexpr int FOOTSWITCH_X = (PLUGIN_WIDTH - FOOTSWITCH_WIDTH) / 2;  // Center horizontally
    static constexpr int FOOTSWITCH_Y = PLUGIN_HEIGHT - 155;  // Aligned with background power button
}

//==============================================================================
// Custom Look and Feel for Wooly Theme
//==============================================================================
class WoolyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    WoolyLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                         const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                     int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;
                     
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

//==============================================================================
// Enhanced GUI with Presets and Animations
//==============================================================================
class WoolyMammothAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Slider::Listener,
                                        private juce::ComboBox::Listener
{
public:
    WoolyMammothAudioProcessorEditor (WoolyMammothAudioProcessor&);
    ~WoolyMammothAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void sliderValueChanged (juce::Slider* slider) override;
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;
    
    // UI Components
    // Title label removed - included in new background image
    
    juce::Slider eqSlider;
    juce::Slider snarlSlider;
    juce::Slider pinchSlider;
    juce::Slider outputSlider;
    
    juce::ToggleButton footswitchButton;
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> snarlAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pinchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> footswitchAttachment;

    WoolyMammothAudioProcessor& audioProcessor;
    WoolyLookAndFeel woolyLF;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WoolyMammothAudioProcessorEditor)
};