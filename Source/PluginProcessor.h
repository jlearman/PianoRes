/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 */
class PianoResAudioProcessor : public juce::AudioProcessor {
public:
  using APVTS = juce::AudioProcessorValueTreeState;
  //==============================================================================
  PianoResAudioProcessor();
  ~PianoResAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  void setIRBufferSize(int newNumChannels, int newNumSamples,
                       bool keepExistingContent = false,
                       bool clearExtraSpace = false,
                       bool avoidReallocating = false);
  juce::AudioBuffer<float> &getOriginalIR();
  juce::AudioBuffer<float> &getModifiedIR();

  void loadImpulseResponse();
  void updateImpulseResponse(juce::AudioBuffer<float> irBuffer);
  void openMemoryIrFile();

  void updateIRParameters();
  void updateFilterParameters();

  APVTS apvts;

private:
  juce::AudioBuffer<float> originalIRBuffer;
  juce::AudioBuffer<float> modifiedIRBuffer;

  juce::AudioFormatManager formatManager;

  // Use an ADSR to control sustain release
  juce::ADSR adsr;
  juce::ADSR::Parameters adsrParams;

  APVTS::ParameterLayout createParameters();

  juce::dsp::Gain<float> inputGainer;
  juce::dsp::Gain<float> dryGainer;
  juce::dsp::Gain<float> wetGainer;
  juce::dsp::Gain<float> outputGainer;
  juce::dsp::Convolution convolver{ juce::dsp::Convolution::NonUniform { 1024 } };
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                 juce::dsp::IIR::Coefficients<float>>
      lowShelfFilter;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                 juce::dsp::IIR::Coefficients<float>>
      highShelfFilter;
  juce::AudioBuffer<float> dryBufferCopy; // for mixing wet and dry with different gains

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoResAudioProcessor)
};
