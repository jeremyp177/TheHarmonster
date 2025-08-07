#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "WoolyMammothDSP.h"

//==============================================================================
class WoolyMammothAudioProcessor : public juce::AudioProcessor
{
public:
    WoolyMammothAudioProcessor();
    ~WoolyMammothAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // Preset/Program management - Updated to support factory presets
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Parameter management
    juce::AudioProcessorValueTreeState parameters;

private:
    WoolyMammothDSP mammothDSP[2]; // Stereo processing
    
    // Parameter pointers
    std::atomic<float>* woolParam = nullptr;
    std::atomic<float>* pinchParam = nullptr;
    std::atomic<float>* eqParam = nullptr;
    std::atomic<float>* outputParam = nullptr;
    std::atomic<float>* bypassParam = nullptr;

    // Preset management
    int currentPresetIndex = 0;
    std::vector<WoolyMammothPresets::Preset> factoryPresets;
    
    // Helper methods for preset management
    void loadPreset(int index);
    void initializeFactoryPresets();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WoolyMammothAudioProcessor)
};