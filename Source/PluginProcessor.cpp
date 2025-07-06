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
        std::make_unique<juce::AudioParameterFloat> ("output", "Output", 0.0f, 1.0f, 0.5f)
    })
{
    woolParam = parameters.getRawParameterValue ("wool");
    pinchParam = parameters.getRawParameterValue ("pinch");
    eqParam = parameters.getRawParameterValue ("eq");
    outputParam = parameters.getRawParameterValue ("output");
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
void WoolyMammothAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void WoolyMammothAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WoolyMammothAudioProcessor();
}