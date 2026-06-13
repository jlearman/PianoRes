/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include <filesystem>
#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
PianoResAudioProcessorEditor::PianoResAudioProcessorEditor(PianoResAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {

  audioProcessor.addChangeListener(this);

  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(750, 300);
  juce::LookAndFeel::setDefaultLookAndFeel(&customStyle);

  // set AudioFormatManager for reading IR file
  formatManager.registerBasicFormats();

  addAndMakeVisible(openIRFileButton);
  openIRFileButton.setButtonText("Open IR File...");
  openIRFileButton.onClick = [this] { openButtonClicked(); };
  addAndMakeVisible(irFileLabel);
  irFileLabel.setText("", juce::dontSendNotification);
  irFileLabel.setJustificationType(juce::Justification::centredLeft);

  addAndMakeVisible(bypassButton);
  bypassButton.setButtonText("Bypass");
  bypassButtonAttachment = std::make_unique<APVTS::ButtonAttachment>(
      audioProcessor.apvts, "Bypassed", bypassButton);

  createSlider(inputGainSlider, " dB");
  createLabel(inputGainLabel, "Input", &inputGainSlider);
  inputGainSliderAttachment = std::make_unique<APVTS::SliderAttachment>(
      audioProcessor.apvts, "InputGain", inputGainSlider);

  createSlider(dryGainSlider, " dB");
  createLabel(dryGainLabel, "Dry", &dryGainSlider);
  dryGainSliderAttachment = std::make_unique<APVTS::SliderAttachment>(
      audioProcessor.apvts, "DryGain", dryGainSlider);

  createSlider(wetGainSlider, " dB");
  createLabel(wetGainLabel, "Wet", &wetGainSlider);
  wetGainSliderAttachment = std::make_unique<APVTS::SliderAttachment>(
      audioProcessor.apvts, "WetGain", wetGainSlider);

  createSlider(releaseTimeSlider, " sec");
  createLabel(releaseTimeLabel, "Release", &releaseTimeSlider);
  releaseTimeSliderAttachment = std::make_unique<APVTS::SliderAttachment>(
      audioProcessor.apvts, "ReleaseTime", releaseTimeSlider);

  createSlider(outputGainSlider, " dB");
  createLabel(outputGainLabel, "Output", &outputGainSlider);
  outputGainSliderAttachment = std::make_unique<APVTS::SliderAttachment>(
      audioProcessor.apvts, "OutputGain", outputGainSlider);

  createSlider(lowShelfFreqSlider, " Hz");
  createLabel(lowShelfFreqLabel, "LowFreq", &lowShelfFreqSlider);
  lowShelfFreqSliderAttachment = std::make_unique<APVTS::SliderAttachment>(
      audioProcessor.apvts, "LowShelfFreq", lowShelfFreqSlider);

  createSlider(lowShelfGainSlider, " dB");
  createLabel(lowShelfGainLabel, "LowGain", &lowShelfGainSlider);
  lowShelfGainSliderAttachment = std::make_unique<APVTS::SliderAttachment>(
      audioProcessor.apvts, "LowShelfGain", lowShelfGainSlider);

  createSlider(highShelfFreqSlider, " Hz");
  createLabel(highShelfFreqLabel, "HighFreq", &highShelfFreqSlider);
  highShelfFreqSliderAttachment = std::make_unique<APVTS::SliderAttachment>(
      audioProcessor.apvts, "HighShelfFreq", highShelfFreqSlider);

  createSlider(highShelfGainSlider, " dB");
  createLabel(highShelfGainLabel, "HighGain", &highShelfGainSlider);
  highShelfGainSliderAttachment = std::make_unique<APVTS::SliderAttachment>(
      audioProcessor.apvts, "HighShelfGain", highShelfGainSlider);

}

PianoResAudioProcessorEditor::~PianoResAudioProcessorEditor() {
  audioProcessor.removeChangeListener(this);
  juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

//==============================================================================
void PianoResAudioProcessorEditor::paint(juce::Graphics &g) {
  g.fillAll(juce::Colour::fromRGB(252, 248, 237));
  g.setFont(32.0f);
  g.setColour(juce::Colour::fromRGB(111, 76, 91));
  g.drawFittedText("PianoRes", getWidth() - 80 - 15 - 30, 15, 100, 20, // FIXME
                   juce::Justification::centred, 1);

  g.setColour(juce::Colour::fromRGB(158, 119, 119));
  
  static juce::String lastIrFilename = "--none--";

  juce::String irFilename = audioProcessor.apvts.state.getProperty("IrFilename", "");
  if (irFilename == "") {
      irFilename = "built-in IR";
  }

  static int drawCount = 1; // FIXME: why do we need to draw 3 times at startup?

  if (drawCount <= 3 || irFilename != lastIrFilename) {
      const int waveformWidth = 80 * 3;
      const int waveformHeight = 100;

      // remove path from filename for display
      std::filesystem::path irPath(irFilename.toStdString());
      irFileLabel.setText(irPath.filename().string(), juce::dontSendNotification);

      std::vector<float> waveformValues;
      waveformValues.clear();

      juce::Path waveformPath;
      waveformPath.clear();
      waveformPath.startNewSubPath(15, waveformHeight + 60);

      auto buffer = audioProcessor.getOriginalIR();
      const float waveformResolution = 1024.0f;
      const int ratio =
          static_cast<int>(buffer.getNumSamples() / waveformResolution);

      if (buffer.getNumChannels() == 0)  return;

      auto bufferPointer = buffer.getReadPointer(0);
      for (int sample = 0; sample < buffer.getNumSamples(); sample += ratio) {
          waveformValues.push_back(juce::Decibels::gainToDecibels<float>(
              std::fabsf(bufferPointer[sample]), -72.0f));
      }
      for (int xPos = 0; xPos < waveformValues.size(); ++xPos) {
          auto yPos = juce::jmap<float>(waveformValues[xPos], -72.0f, 0.0f,
              waveformHeight + 60, 75);  // FIXME: where do these numbers come from?
          waveformPath.lineTo(15 + xPos / waveformResolution * waveformWidth, yPos);
      }

      g.strokePath(waveformPath, juce::PathStrokeType(1.0f));

      waveformPainted++;
      if (irFilename != lastIrFilename) {
          drawCount = 1;
      }
      // DBG("painted waveform: '" << irFilename << "', '" << lastIrFilename << "'");
      DBG("======== painted waveform: '" << irFilename << " " << buffer.getNumSamples() << " " << drawCount++);
      lastIrFilename = irFilename;
  }
}

void PianoResAudioProcessorEditor::resized() {
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
  const int topBottomMargin = 15;
  const int leftRightMargin = 15;

  const int dialWidth = 80;
  const int dialHeight = 90;

  openIRFileButton.setBounds(leftRightMargin, topBottomMargin, dialWidth * 3,
                             40);
  irFileLabel.setBounds(leftRightMargin, topBottomMargin + 45, dialWidth * 3,
                        20);
  bypassButton.setBounds(getWidth() - leftRightMargin - dialWidth * 3,
                         topBottomMargin, dialWidth, 20);

  // sliders planced from left to right
  int ix = 0; // x axis index
  inputGainSlider.setBounds(leftRightMargin + dialWidth * ix++,
                            getHeight() - topBottomMargin - dialHeight,
                            dialWidth, dialHeight);
  dryGainSlider.setBounds(leftRightMargin + dialWidth * ix++,
                            getHeight() - topBottomMargin - dialHeight,
                            dialWidth, dialHeight);
  wetGainSlider.setBounds(leftRightMargin + dialWidth * ix++,
                            getHeight() - topBottomMargin - dialHeight,
                            dialWidth, dialHeight);
  releaseTimeSlider.setBounds(leftRightMargin + dialWidth * ix++,
                            getHeight() - topBottomMargin - dialHeight,
                            dialWidth, dialHeight);
  outputGainSlider.setBounds(leftRightMargin + dialWidth * ix++,
                            getHeight() - topBottomMargin - dialHeight,
                            dialWidth, dialHeight);

  // sliders placed from right margin to left
  lowShelfFreqSlider.setBounds(getWidth() - leftRightMargin - dialWidth * 2,
                               topBottomMargin + dialHeight / 3 * 2, dialWidth,
                               dialHeight);
  lowShelfGainSlider.setBounds(getWidth() - leftRightMargin - dialWidth * 2,
                               getHeight() - topBottomMargin - dialHeight,
                               dialWidth, dialHeight);
  highShelfFreqSlider.setBounds(getWidth() - leftRightMargin - dialWidth,
                                topBottomMargin + dialHeight / 3 * 2, dialWidth,
                                dialHeight);
  highShelfGainSlider.setBounds(getWidth() - leftRightMargin - dialWidth,
                                getHeight() - topBottomMargin - dialHeight,
                                dialWidth, dialHeight);
}

void PianoResAudioProcessorEditor::openButtonClicked() {
  fileChooser = std::make_unique<juce::FileChooser>(
      "Choose a support IR File (WAV, AIFF, OGG, FLAC)...", juce::File(),
      "*.wav;*.aif;*.aiff;*.ogg;*.flac", true, false);
  auto chooserFlags = juce::FileBrowserComponent::openMode |
                      juce::FileBrowserComponent::canSelectFiles;
  fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser &fc) {
    auto file = fc.getResult();
    if (file != juce::File()) {
      // update text of IR file label
      audioProcessor.apvts.state.setProperty("IrFilename", file.getFullPathName(), nullptr);
	  // load IR file and update IR buffer in processor
      audioProcessor.readIrFile(file.getFullPathName().toStdString());
      repaint();
    }
  });
}

void PianoResAudioProcessorEditor::createSlider(juce::Slider &slider,
                                              juce::String textValueSuffix) {
  addAndMakeVisible(slider);
  slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  slider.setTextValueSuffix(textValueSuffix);
  slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow,
                         false, 60, 15);
  // slider.setPopupDisplayEnabled(true, true, this);
}

void PianoResAudioProcessorEditor::createLabel(juce::Label &label,
                                             juce::String text,
                                             juce::Component *slider) {
  addAndMakeVisible(label);
  label.setText(text, juce::dontSendNotification);
  label.setJustificationType(juce::Justification::centred);
  label.setBorderSize(juce::BorderSize<int>(0));
  label.attachToComponent(slider, false);
}

void PianoResAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioProcessor) {
        repaint(); // Safely called on the UI message thread
    }
}