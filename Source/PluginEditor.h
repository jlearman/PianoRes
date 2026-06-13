/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "CustomStyle.h"
#include "PluginProcessor.h"
#include <JuceHeader.h>

//==============================================================================
/**
 */
class PianoResAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     public juce::ChangeListener {
public:
  using APVTS = juce::AudioProcessorValueTreeState;

  PianoResAudioProcessorEditor(PianoResAudioProcessor &);
  ~PianoResAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  PianoResAudioProcessor &audioProcessor;

  void changeListenerCallback(juce::ChangeBroadcaster* source) override;

  juce::CustomStyle customStyle;

  juce::AudioFormatManager formatManager;
  std::unique_ptr<juce::FileChooser> fileChooser;

  int waveformPainted = 0; // FIXME: find out why boolean doesn't work

  juce::TextButton openIRFileButton;
  juce::Label irFileLabel;
  juce::ToggleButton bypassButton;
  std::unique_ptr<APVTS::ButtonAttachment> bypassButtonAttachment;
  juce::Slider inputGainSlider;
  juce::Label inputGainLabel;
  std::unique_ptr<APVTS::SliderAttachment> inputGainSliderAttachment;
  juce::Slider dryGainSlider;
  juce::Label dryGainLabel;
  std::unique_ptr<APVTS::SliderAttachment> dryGainSliderAttachment;
  juce::Slider wetGainSlider;
  juce::Label wetGainLabel;
  std::unique_ptr<APVTS::SliderAttachment> wetGainSliderAttachment;
  juce::Slider releaseTimeSlider;
  juce::Label releaseTimeLabel;
  std::unique_ptr<APVTS::SliderAttachment> releaseTimeSliderAttachment;
  juce::Slider outputGainSlider;
  juce::Label outputGainLabel;
  std::unique_ptr<APVTS::SliderAttachment> outputGainSliderAttachment;
  juce::Slider lowShelfFreqSlider;
  juce::Label lowShelfFreqLabel;
  std::unique_ptr<APVTS::SliderAttachment> lowShelfFreqSliderAttachment;
  juce::Slider lowShelfGainSlider;
  juce::Label lowShelfGainLabel;
  std::unique_ptr<APVTS::SliderAttachment> lowShelfGainSliderAttachment;
  juce::Slider highShelfFreqSlider;
  juce::Label highShelfFreqLabel;
  std::unique_ptr<APVTS::SliderAttachment> highShelfFreqSliderAttachment;
  juce::Slider highShelfGainSlider;
  juce::Label highShelfGainLabel;
  std::unique_ptr<APVTS::SliderAttachment> highShelfGainSliderAttachment;

  void openButtonClicked();
  void createSlider(juce::Slider &slider, juce::String textValueSuffix);
  void createLabel(juce::Label &label, juce::String text,
                   juce::Component *slider);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoResAudioProcessorEditor)
};
