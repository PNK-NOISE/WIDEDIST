#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TapeDistAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                      public juce::Timer
{
public:
    TapeDistAudioProcessorEditor (TapeDistAudioProcessor&);
    ~TapeDistAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;

private:
    TapeDistAudioProcessor& audioProcessor;
    
    juce::Slider driveSlider;
    juce::ComboBox distTypeComboBox;
    juce::Slider mixSlider;
    juce::Slider outputSlider;
    
    juce::Slider crossoverSlider;
    juce::Slider widthSlider;
    juce::Slider wideGainSlider;
    
    juce::ComboBox bassFxTypeComboBox;
    juce::Slider bassAmountSlider;
    juce::Slider bassGainSlider;
    
    juce::Slider peakReductionSlider;
    juce::Slider compGainSlider;
    
    juce::ToggleButton distOnButton { "On" };
    juce::ToggleButton ampSimButton { "Amp Sim (Cab)" };
    juce::ToggleButton wideOnButton { "On" };
    juce::ToggleButton compOnButton { "On" };
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> distTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> crossoverAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wideGainAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> bassFxTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassGainAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> peakReductionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compGainAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> distOnAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> ampSimAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> wideOnAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> compOnAttachment;
    
    // Quick helper factory method
    void setupSlider(juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, const juce::String& paramID);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeDistAudioProcessorEditor)
};
