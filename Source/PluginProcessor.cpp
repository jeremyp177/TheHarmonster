#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WoolyMammothAudioProcessor::WoolyMammothAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    parameters (*this, nullptr, juce::Identifier ("WoolyMammoth"),
    {
        std::make_unique<juce::AudioParameterFloat> ("wool", "Wool", 0.0f, 1.0f, 0.5f),
        std::make_unique<juce::AudioParameterFloat> ("pinch", "Pinch", 0.0f, 1.0f, 0.3f),
        std::make_unique<juce::AudioParameterFloat> ("eq", "EQ", 0.0f, 1.0f, 0.5f),
        std::make_unique<juce::AudioParameterFloat> ("output", "Output", 0.0f, 1.0f, 0.5f),
        std::make_unique<juce::AudioParameterBool> ("bypass", "Bypass", false)
    })
{
    woolParam = parameters.getRawParameterValue ("wool");
    pinchParam = parameters.getRawParameterValue ("pinch");
    eqParam = parameters.getRawParameterValue ("eq");
    outputParam = parameters.getRawParameterValue ("output");
    bypassParam = parameters.getRawParameterValue ("bypass");
    
    // Initialize factory presets
    initializeFactoryPresets();
}

WoolyMammothAudioProcessor::~WoolyMammothAudioProcessor()
{
}

//==============================================================================
void WoolyMammothAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    for (auto& dsp : mammothDSP)
    {
        dsp.setSampleRate (sampleRate);
        dsp.reset();
    }
}

void WoolyMammothAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void WoolyMammothAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Check if bypassed
    bool isBypassed = bypassParam->load() > 0.5f;
    
    if (isBypassed)
    {
        // Bypass - pass audio through unchanged
        return;
    }

    // Update DSP parameters
    for (auto& dsp : mammothDSP)
    {
        dsp.setWool (woolParam->load());
        dsp.setPinch (pinchParam->load());
        dsp.setEQ (eqParam->load());
        dsp.setOutput (outputParam->load());
    }

    // Process audio
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        auto& channelDSP = mammothDSP[channel];

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = static_cast<float>(channelDSP.process(channelData[sample]));
        }
    }
}

//==============================================================================
juce::AudioProcessorEditor* WoolyMammothAudioProcessor::createEditor()
{
    return new WoolyMammothAudioProcessorEditor (*this);
}

//==============================================================================
// Preset/Program management implementation
int WoolyMammothAudioProcessor::getNumPrograms()
{
    return static_cast<int>(factoryPresets.size());
}

int WoolyMammothAudioProcessor::getCurrentProgram()
{
    return currentPresetIndex;
}

void WoolyMammothAudioProcessor::setCurrentProgram(int index)
{
    if (index >= 0 && index < static_cast<int>(factoryPresets.size()))
    {
        currentPresetIndex = index;
        loadPreset(index);
    }
}

const juce::String WoolyMammothAudioProcessor::getProgramName(int index)
{
    if (index >= 0 && index < static_cast<int>(factoryPresets.size()))
        return factoryPresets[index].name;
    return "Unknown";
}

void WoolyMammothAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    // For factory presets, we don't allow name changes
    // This could be extended to support user presets in the future
    (void)index;
    (void)newName;
}

//==============================================================================
void WoolyMammothAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    
    // Add current preset index to the state
    state.setProperty("currentPreset", currentPresetIndex, nullptr);
    
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void WoolyMammothAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            auto newState = juce::ValueTree::fromXml (*xmlState);
            parameters.replaceState (newState);
            
            // Restore current preset index
            if (newState.hasProperty("currentPreset"))
            {
                currentPresetIndex = newState.getProperty("currentPreset", 0);
                currentPresetIndex = juce::jlimit(0, static_cast<int>(factoryPresets.size()) - 1, currentPresetIndex);
            }
        }
    }
}

//==============================================================================
// Preset management helper methods
void WoolyMammothAudioProcessor::initializeFactoryPresets()
{
    factoryPresets = WoolyMammothPresets::getFactoryPresets();
    
    // Load the first preset by default
    if (!factoryPresets.empty())
    {
        loadPreset(0);
    }
}

void WoolyMammothAudioProcessor::loadPreset(int index)
{
    if (index >= 0 && index < static_cast<int>(factoryPresets.size()))
    {
        const auto& preset = factoryPresets[index];
        
        // Update parameter values
        if (auto* woolParamObj = parameters.getParameter("wool"))
            woolParamObj->setValueNotifyingHost(static_cast<float>(preset.wool));
            
        if (auto* pinchParamObj = parameters.getParameter("pinch"))
            pinchParamObj->setValueNotifyingHost(static_cast<float>(preset.pinch));
            
        if (auto* eqParamObj = parameters.getParameter("eq"))
            eqParamObj->setValueNotifyingHost(static_cast<float>(preset.eq));
            
        if (auto* outputParamObj = parameters.getParameter("output"))
            outputParamObj->setValueNotifyingHost(static_cast<float>(preset.output));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WoolyMammothAudioProcessor();
}