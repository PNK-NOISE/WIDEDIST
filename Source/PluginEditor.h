#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "WebAssets.h"

class TapeDistAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    TapeDistAudioProcessorEditor (TapeDistAudioProcessor&);
    ~TapeDistAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);
    void timerCallback() override;

private:
    TapeDistAudioProcessor& audioProcessor;
    
    // Web Relays
    juce::WebSliderRelay driveRelay { "drive" };
    juce::WebComboBoxRelay distTypeRelay { "distType" };
    juce::WebSliderRelay mixRelay { "mix" };
    juce::WebSliderRelay outputRelay { "output" };
    juce::WebToggleButtonRelay distOnRelay { "distOn" };
    juce::WebToggleButtonRelay ampSimRelay { "ampSim" };
    
    juce::WebSliderRelay crossoverRelay { "crossover" };
    juce::WebSliderRelay widthRelay { "width" };
    juce::WebSliderRelay wideGainRelay { "wideGain" };
    juce::WebToggleButtonRelay wideOnRelay { "wideOn" };
    
    juce::WebComboBoxRelay bassFxTypeRelay { "bassFxType" };
    juce::WebSliderRelay bassAmountRelay { "bassAmount" };
    juce::WebSliderRelay bassGainRelay { "bassGain" };
    juce::WebToggleButtonRelay bassOnRelay { "bassOn" };
    juce::WebToggleButtonRelay bassScOnRelay { "bassScOn" };
    juce::WebSliderRelay bassScAmountRelay { "bassScAmount" };
    juce::WebSliderRelay bassScReleaseRelay { "bassScRelease" };
    
    juce::WebSliderRelay peakReductionRelay { "peakReduction" };
    juce::WebSliderRelay compGainRelay { "compGain" };
    juce::WebSliderRelay finalSatRelay { "finalSat" };
    juce::WebToggleButtonRelay compOnRelay { "compOn" };
    
    juce::WebSliderRelay masterMixRelay { "masterMix" };
    juce::WebToggleButtonRelay masterSoftClipRelay { "masterSoftClip" };
    
    // APVTS Attachments
    juce::WebSliderParameterAttachment driveAttachment;
    juce::WebComboBoxParameterAttachment distTypeAttachment;
    juce::WebSliderParameterAttachment mixAttachment;
    juce::WebSliderParameterAttachment outputAttachment;
    juce::WebToggleButtonParameterAttachment distOnAttachment;
    juce::WebToggleButtonParameterAttachment ampSimAttachment;
    
    juce::WebSliderParameterAttachment crossoverAttachment;
    juce::WebSliderParameterAttachment widthAttachment;
    juce::WebSliderParameterAttachment wideGainAttachment;
    juce::WebToggleButtonParameterAttachment wideOnAttachment;
    
    juce::WebComboBoxParameterAttachment bassFxTypeAttachment;
    juce::WebSliderParameterAttachment bassAmountAttachment;
    juce::WebSliderParameterAttachment bassGainAttachment;
    juce::WebToggleButtonParameterAttachment bassOnAttachment;
    juce::WebToggleButtonParameterAttachment bassScOnAttachment;
    juce::WebSliderParameterAttachment bassScAmountAttachment;
    juce::WebSliderParameterAttachment bassScReleaseAttachment;
    
    juce::WebSliderParameterAttachment peakReductionAttachment;
    juce::WebSliderParameterAttachment compGainAttachment;
    juce::WebSliderParameterAttachment finalSatAttachment;
    juce::WebToggleButtonParameterAttachment compOnAttachment;
    
    juce::WebSliderParameterAttachment masterMixAttachment;
    juce::WebToggleButtonParameterAttachment masterSoftClipAttachment;
    
    // The Web Browser
    juce::WebBrowserComponent webBrowser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeDistAudioProcessorEditor)
};
