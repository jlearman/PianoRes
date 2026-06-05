/*
 ==============================================================================

   This file contains the basic framework code for a JUCE plugin processor.

 ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PianoResAudioProcessor::PianoResAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      apvts(*this, nullptr, "Parameters", createParameters()),
      lowShelfFilter(juce::dsp::IIR::Coefficients<float>::makeLowShelf(
          44100, 20.0f, 1.0f, 0.7f)),
      highShelfFilter(juce::dsp::IIR::Coefficients<float>::makeHighShelf(
          44100, 20000.0f, 1.0f, 0.7f))
#endif
{
}

PianoResAudioProcessor::~PianoResAudioProcessor() {}

//==============================================================================
const juce::String PianoResAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool PianoResAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool PianoResAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool PianoResAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double PianoResAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int PianoResAudioProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int PianoResAudioProcessor::getCurrentProgram() { return 0; }

void PianoResAudioProcessor::setCurrentProgram(int /*index*/) {}

const juce::String PianoResAudioProcessor::getProgramName(int /*oindex*/) {
  return {};
}

void PianoResAudioProcessor::changeProgramName(int /*index*/,
                                             const juce::String &/*newName*/) { }

//==============================================================================
void PianoResAudioProcessor::prepareToPlay(double sampleRate,
                                         int samplesPerBlock) {
  // pre-initialization process
  juce::dsp::ProcessSpec spec = juce::dsp::ProcessSpec();
  spec.sampleRate = sampleRate;
  spec.numChannels = getTotalNumOutputChannels();
  spec.maximumBlockSize = samplesPerBlock;

  
  inputGainer.prepare(spec);
  inputGainer.reset();
  // inputGainer.setRampDurationSeconds(0.02); // TODO? avoid clicks when moving sliders
  dryGainer.prepare(spec);
  dryGainer.reset();
  wetGainer.prepare(spec);
  wetGainer.reset();
  outputGainer.prepare(spec);
  outputGainer.reset();
  convolver.prepare(spec);
  convolver.reset();

  lowShelfFilter.prepare(spec);
  lowShelfFilter.reset();
  highShelfFilter.prepare(spec);
  highShelfFilter.reset();

  adsr.setSampleRate(sampleRate);
  adsrParams.attack = 0.05f; // Seconds -- make a parameter?
  adsrParams.decay = 0.0f;
  adsrParams.sustain = 1.0f;
  adsrParams.release = apvts.getRawParameterValue("ReleaseTime")->load();
  adsr.setParameters(adsrParams);
}

void PianoResAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PianoResAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void PianoResAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                        juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  // In case we have more outputs than inputs, this code clears any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  // This is here to avoid people getting screaming feedback
  // when they first compile a plugin, but obviously you don't need to keep
  // this code if your algorithm always overwrites all the output channels.
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  // acquire parameters from AudioProcessorValueTreeState
  auto inputGainValue = apvts.getRawParameterValue("InputGain");
  auto dryGainValue = apvts.getRawParameterValue("DryGain");
  auto wetGainValue = apvts.getRawParameterValue("WetGain");
  auto outputGainValue = apvts.getRawParameterValue("OutputGain");
  auto isBypassed = apvts.getRawParameterValue("Bypassed");
  updateFilterParameters();

  if (isBypassed->load() != 0.0) {
    return;
  }

  inputGainer.setGainDecibels(inputGainValue->load());
  dryGainer.setGainDecibels(dryGainValue->load());
  wetGainer.setGainDecibels(wetGainValue->load());
  outputGainer.setGainDecibels(outputGainValue->load());


  static bool isSustainPedalDown = false;

  // detect sustain pedal on/off messages and apply ADSR envelope
  for (const auto metadata : midiMessages)  // juce::MidiBufferMetadata
  {
      // Get the actual MIDI message object
      auto message = metadata.getMessage();

      // Get its position relative to the start of this block (0 to buffer.getNumSamples() - 1)
      // int samplePos = metadata.samplePosition; // no point: always zero since we're processing the whole block
      // TODO: sample-accurate pedaling.  It's fine without it for live playing, but when
      // using MIDI in a DAW, sustain on/off messages can get stacked up due to sloppy editing.

      if (message.isSustainPedalOn()) {
          isSustainPedalDown = false;
          adsr.noteOn();
          convolver.reset(); // FIXME: this can cause a click when the pedal is pressed, but without it,
          // the release phase of the previous notes will still be convolved with the IR,
          // which isn't how a piano works.  A better solution might be to have the 
          // convolver only process the "active" part of the buffer, but how?
      }
      else if (message.isSustainPedalOff()) {
          isSustainPedalDown = true;
          auto releaseTime = apvts.getRawParameterValue("ReleaseTime")->load();
          if (releaseTime != adsrParams.release) {
              adsrParams.release = releaseTime;
              adsr.setParameters(adsrParams);
          }
          adsr.noteOff();
      }
  }

  auto block = juce::dsp::AudioBlock<float>(buffer);
  auto context = juce::dsp::ProcessContextReplacing<float>(block);
  inputGainer.process(context);
  dryBufferCopy.makeCopyOf(buffer); // Keep the dry signal (after input gain)
  convolver.process(context);
  lowShelfFilter.process(context);
  highShelfFilter.process(context);
  adsr.applyEnvelopeToBuffer(buffer, 0, buffer.getNumSamples());
  wetGainer.process(context);

  // Mix the dry signal back in with dry gain
  auto dryBlock = juce::dsp::AudioBlock<float>(dryBufferCopy);
  auto dryContext = juce::dsp::ProcessContextReplacing<float>(dryBlock);
  dryGainer.process(dryContext);
  for (int channel = 0; channel < totalNumInputChannels; ++channel)
  {
      buffer.addFrom(channel, 0, dryBufferCopy, channel, 0, buffer.getNumSamples());
  }
  outputGainer.process(context);
}

//==============================================================================
bool PianoResAudioProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *PianoResAudioProcessor::createEditor() {
  return new PianoResAudioProcessorEditor(*this);
}

//==============================================================================
void PianoResAudioProcessor::getStateInformation(juce::MemoryBlock &/*destData*/) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void PianoResAudioProcessor::setStateInformation(const void */*data*/,
                                               int /*sizeInBytes*/) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
}

void PianoResAudioProcessor::setIRBufferSize(int newNumChannels,
                                           int newNumSamples,
                                           bool keepExistingContent,
                                           bool clearExtraSpace,
                                           bool avoidReallocating) {
  originalIRBuffer.setSize(newNumChannels, newNumSamples, keepExistingContent,
                           clearExtraSpace, avoidReallocating);
}

juce::AudioBuffer<float> &PianoResAudioProcessor::getOriginalIR() {
  return originalIRBuffer;
}

juce::AudioBuffer<float> &PianoResAudioProcessor::getModifiedIR() {
  return modifiedIRBuffer;
}

void PianoResAudioProcessor::loadImpulseResponse() {
  // normalized IR signal
  float globalMaxMagnitude =
      originalIRBuffer.getMagnitude(0, originalIRBuffer.getNumSamples());
  originalIRBuffer.applyGain(1.0f / (globalMaxMagnitude + 0.01f));

  updateImpulseResponse(originalIRBuffer);
}

void PianoResAudioProcessor::updateImpulseResponse(
    juce::AudioBuffer<float> irBuffer) {
  convolver.loadImpulseResponse(std::move(irBuffer), this->getSampleRate(),
                                juce::dsp::Convolution::Stereo::yes,
                                juce::dsp::Convolution::Trim::yes,
                                juce::dsp::Convolution::Normalise::yes);
}

// TODO: remove this since there aren't any more IR parameters?
void PianoResAudioProcessor::updateIRParameters() {
  if (originalIRBuffer.getNumSamples() < 1) {
    return;
  }

  updateImpulseResponse(originalIRBuffer);
}


void PianoResAudioProcessor::updateFilterParameters() {
  const float sampleRate = static_cast<float>(this->getSampleRate());

  auto lowShelfFreqValue = apvts.getRawParameterValue("LowShelfFreq");
  auto lowShelfGainValue = apvts.getRawParameterValue("LowShelfGain");
  auto highShelfFreqValue = apvts.getRawParameterValue("HighShelfFreq");
  auto highShelfGainValue = apvts.getRawParameterValue("HighShelfGain");
  *lowShelfFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
      sampleRate, lowShelfFreqValue->load(), 0.7f,
      juce::Decibels::decibelsToGain(lowShelfGainValue->load()));
  *highShelfFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
      sampleRate, highShelfFreqValue->load(), 0.7f,
      juce::Decibels::decibelsToGain(highShelfGainValue->load()));
}

juce::AudioProcessorValueTreeState::ParameterLayout
PianoResAudioProcessor::createParameters() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

  juce::NormalisableRange<float> gainRange =
      juce::NormalisableRange<float>(-72.0f, 36.0f, 0.1f);
  gainRange.setSkewForCentre(0.0f);
  juce::NormalisableRange<float> releaseTimeRange =
      juce::NormalisableRange<float>(0.0f, 1.00f, 0.01f);
  juce::NormalisableRange<float> lowShelfCutoffFreqRange =
      juce::NormalisableRange<float>(20.0f, 2000.0f, 1.0f);
  lowShelfCutoffFreqRange.setSkewForCentre(400.0f);
  juce::NormalisableRange<float> lowShelfGainRange =
      juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f);
  lowShelfGainRange.setSkewForCentre(0.0f);
  juce::NormalisableRange<float> highShelfCutoffFreqRange =
      juce::NormalisableRange<float>(200.0f, 20000.0f, 1.0f);
  highShelfCutoffFreqRange.setSkewForCentre(4000.0f);
  juce::NormalisableRange<float> highShelfGainRange =
      juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f);

  parameters.push_back(std::make_unique<juce::AudioParameterBool>(
      "Reversed", "Reversed", false));
  parameters.push_back(std::make_unique<juce::AudioParameterBool>(
      "Bypassed", "Bypassed", false));
  parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
      "InputGain", "Input Gain", gainRange, 0.0f));
  parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
      "DryGain", "Dry Gain", gainRange, 0.0f));
  parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
      "WetGain", "Wet Gain", gainRange, -7.5f));
  parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
      "OutputGain", "Output Gain", gainRange, 0.0f));
  parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
      "ReleaseTime", "Decay", releaseTimeRange, 0.2f));
  parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
      "LowShelfFreq", "LowFreq", lowShelfCutoffFreqRange, 20.0f));
  parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
      "LowShelfGain", "LowGain", lowShelfGainRange, 0.0f));
  parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
      "HighShelfFreq", "HighFreq", highShelfCutoffFreqRange, 20000.0f));
  parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
      "HighShelfGain", "HighGain", highShelfGainRange, 0.0f));
  return {parameters.begin(), parameters.end()};
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new PianoResAudioProcessor();
}
