#pragma once
#include <JuceHeader.h>
#include "PresetManager.h"

class TapeDistAudioProcessor  : public juce::AudioProcessor
{
public:
    TapeDistAudioProcessor();
    ~TapeDistAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // APVTS for our Tape Distortion parameters
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Live UI Meters
    std::atomic<float> currentBassGR { 0.0f };
    std::atomic<float> currentMasterGR { 0.0f };
    
    // Preset Manager
    std::unique_ptr<PresetManager> presetManager;

private:
    std::atomic<float>* driveParam = nullptr;
    std::atomic<float>* distTypeParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* outputParam = nullptr;
    std::atomic<float>* distOnParam = nullptr;
    std::atomic<float>* ampSimParam = nullptr;
    
    // Stage 2: Widener & Bass FX
    std::atomic<float>* crossoverParam = nullptr;
    std::atomic<float>* widthParam = nullptr;
    std::atomic<float>* wideGainParam = nullptr;
    std::atomic<float>* wideOnParam = nullptr;
    std::atomic<float>* bassFxTypeParam = nullptr;
    std::atomic<float>* bassAmountParam = nullptr;
    std::atomic<float>* bassGainParam = nullptr;
    std::atomic<float>* bassOnParam = nullptr;
    std::atomic<float>* bassScOnParam = nullptr;
    std::atomic<float>* bassScAmountParam = nullptr;
    std::atomic<float>* bassScReleaseParam = nullptr;
    
    // Stage 3: Compressor
    std::atomic<float>* peakReductionParam = nullptr;
    std::atomic<float>* compGainParam = nullptr;
    std::atomic<float>* compOnParam = nullptr;
    std::atomic<float>* finalSatParam = nullptr;
    std::atomic<float>* masterMixParam = nullptr;
    std::atomic<float>* masterSoftClipParam = nullptr;

    double currentSampleRate = 44100.0;
    
    // LR4 Crossover Filters
    juce::dsp::IIR::Filter<float> lp1Mono, lp2Mono;
    juce::dsp::IIR::Filter<float> hp1L, hp2L;
    juce::dsp::IIR::Filter<float> hp1R, hp2R;
    
    // Delay line for synthetic stereo side channel
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine { 48000 };
    
    juce::dsp::IIR::Filter<float> ampCabFilterL;
    juce::dsp::IIR::Filter<float> ampCabFilterR;
    
    // Compressor Envelopes
    float bassCompEnvelope = 0.0f;
    float sidechainEnvelope = 0.0f;
    float compEnvelope = 0.0f;
    
    // Glitch State
    

    
    void updateParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeDistAudioProcessor)
};
