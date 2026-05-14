#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <algorithm>

TapeDistAudioProcessor::TapeDistAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts (*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    driveParam = apvts.getRawParameterValue("DRIVE");
    distTypeParam = apvts.getRawParameterValue("DIST_TYPE");
    mixParam = apvts.getRawParameterValue("MIX");
    outputParam = apvts.getRawParameterValue("OUTPUT");
    distOnParam = apvts.getRawParameterValue("DIST_ON");
    ampSimParam = apvts.getRawParameterValue("AMP_SIM");
    crossoverParam = apvts.getRawParameterValue("CROSSOVER");
    widthParam = apvts.getRawParameterValue("WIDTH");
    wideGainParam = apvts.getRawParameterValue("WIDE_GAIN");
    wideOnParam = apvts.getRawParameterValue("WIDE_ON");
    bassFxTypeParam = apvts.getRawParameterValue("BASS_FX_TYPE");
    bassAmountParam = apvts.getRawParameterValue("BASS_AMOUNT");
    bassGainParam = apvts.getRawParameterValue("BASS_GAIN");
    peakReductionParam = apvts.getRawParameterValue("PEAK_REDUCTION");
    compGainParam = apvts.getRawParameterValue("COMP_GAIN");
    compOnParam = apvts.getRawParameterValue("COMP_ON");
    finalSatParam = apvts.getRawParameterValue("FINAL_SAT");
    masterMixParam = apvts.getRawParameterValue("MASTER_MIX");
    bassOnParam = apvts.getRawParameterValue("BASS_ON");
    masterSoftClipParam = apvts.getRawParameterValue("MASTER_SOFT_CLIP");
    
    presetManager = std::make_unique<PresetManager>(apvts);
    presetManager->loadPreset("808-POWER");
}

TapeDistAudioProcessor::~TapeDistAudioProcessor()
{
}

const juce::String TapeDistAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TapeDistAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TapeDistAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TapeDistAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TapeDistAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TapeDistAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TapeDistAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TapeDistAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TapeDistAudioProcessor::getProgramName (int index)
{
    return {};
}

void TapeDistAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void TapeDistAudioProcessor::updateParameters()
{
    // Handled in processBlock now
}

void TapeDistAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    lp1Mono.prepare(spec); lp2Mono.prepare(spec);
    hp1L.prepare(spec); hp2L.prepare(spec);
    hp1R.prepare(spec); hp2R.prepare(spec);
    
    lp1Mono.reset(); lp2Mono.reset();
    hp1L.reset(); hp2L.reset();
    hp1R.reset(); hp2R.reset();
    
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(sampleRate * 0.1); // 100ms
    
    auto cabCoefs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 4500.0f, 0.707f);
    ampCabFilterL.coefficients = cabCoefs;
    ampCabFilterR.coefficients = cabCoefs;
    ampCabFilterL.reset();
    ampCabFilterR.reset();
    
    updateParameters();
}

void TapeDistAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TapeDistAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TapeDistAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateParameters();
    
    // -- STAGE 1: TAPE DISTORTION PARAMETERS --
    float driveDb = driveParam->load();
    float drive = juce::Decibels::decibelsToGain(driveDb);
    float autoGain = juce::Decibels::decibelsToGain(-driveDb * 0.45f);
    float makeUp = juce::Decibels::decibelsToGain(outputParam->load());
    float mix = mixParam->load() / 100.0f;
    int distType = static_cast<int>(distTypeParam->load());
    bool distOn = distOnParam->load() > 0.5f;
    bool ampSimOn = ampSimParam->load() > 0.5f;
    
    // -- STAGE 2: WIDENER & BASS FX PARAMETERS --
    float crossoverFreq = crossoverParam->load();
    float widthAmt = widthParam->load() / 100.0f; // 0 to 2.0
    float wideGain = juce::Decibels::decibelsToGain(wideGainParam->load());
    bool wideOn = wideOnParam->load() > 0.5f;
    int bassFxType = static_cast<int>(bassFxTypeParam->load());
    float bassAmount = bassAmountParam->load() / 100.0f; // 0.0 to 1.0
    float bassGain = juce::Decibels::decibelsToGain(bassGainParam->load());
    
    // -- STAGE 3: LA-2A COMPRESSOR PARAMETERS --
    float peakReduction = peakReductionParam->load(); // 0 to 100
    float compGainLinear = juce::Decibels::decibelsToGain(compGainParam->load());
    bool compOn = compOnParam->load() > 0.5f;
    
    // Compressor tuning approximations
    float thresholdDb = juce::jmap(peakReduction, 0.0f, 100.0f, 0.0f, -40.0f);
    float attackTime = 10.0f; // ms
    float attackCoef = std::exp(-1000.0f / (attackTime * currentSampleRate));
    // Simulated Opto Envelope: Fast primary release, very slow secondary release. We'll use a ~60ms primary.
    float releaseCoef = std::exp(-1000.0f / (60.0f * currentSampleRate));

    // LR4 Coefficients Update
    auto lpCoefs = juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, crossoverFreq, 0.70710678f);
    auto hpCoefs = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, crossoverFreq, 0.70710678f);
    lp1Mono.coefficients = lpCoefs; lp2Mono.coefficients = lpCoefs;
    hp1L.coefficients = hpCoefs; hp2L.coefficients = hpCoefs;
    hp1R.coefficients = hpCoefs; hp2R.coefficients = hpCoefs;

    // 1. Process Widener, Bass FX & Distortion
    if (wideOn && buffer.getNumChannels() == 2)
    {
        auto* leftPtr = buffer.getWritePointer(0);
        auto* rightPtr = buffer.getWritePointer(1);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float rawL = leftPtr[sample];
            float rawR = rightPtr[sample];
            
            // Extract Mono
            float cleanMono = (rawL + rawR) * 0.5f;
            
            // Apply Crossover
            float lowMono = lp2Mono.processSample(lp1Mono.processSample(cleanMono));
            float highL = hp2L.processSample(hp1L.processSample(rawL));
            float highR = hp2R.processSample(hp1R.processSample(rawR));
            
            // --- Bass FX Engine on lowMono ---
            bool bassOn = bassOnParam->load() > 0.5f;
            if (bassOn)
            {
                if (bassFxType == 1) // Compressor
                {
                    float thresholdDb = juce::jmap(bassAmount, 0.0f, 1.0f, 0.0f, -30.0f);
                    float makeup = juce::jmap(bassAmount, 0.0f, 1.0f, 1.0f, 4.0f); // Auto makeup
                    float peakDb = juce::Decibels::gainToDecibels(std::abs(lowMono), -100.0f);
                    float targetGrDb = 0.0f;
                    if (peakDb > thresholdDb) targetGrDb = (thresholdDb - peakDb) * 0.75f; // 4:1 ratio
                    
                    // Fast Attack (~2ms), Medium Release (~40ms)
                    float attCoef = std::exp(-1000.0f / (2.0f * currentSampleRate));
                    float relCoef = std::exp(-1000.0f / (40.0f * currentSampleRate));
                    
                    if (targetGrDb < bassCompEnvelope) bassCompEnvelope = targetGrDb + attCoef * (bassCompEnvelope - targetGrDb);
                    else bassCompEnvelope = targetGrDb + relCoef * (bassCompEnvelope - targetGrDb);
                    
                    lowMono *= juce::Decibels::decibelsToGain(bassCompEnvelope) * makeup;
                }
                else if (bassFxType == 2) // Soft Clip
                {
                    float driveBass = 1.0f + bassAmount * 10.0f; // Up to 11x drive
                    lowMono = std::tanh(lowMono * driveBass) * (1.0f / std::sqrt(driveBass));
                }
            }
            
            lowMono *= bassGain;
            
            float cleanHighL = highL;
            float cleanHighR = highR;
            
            // --- Distortion Engine on highL / highR ---
            if (distOn) {
                // Distort Left
                float wetL = highL * drive;
                switch (distType) {
                    case 0: wetL = std::tanh(wetL); break;
                    case 1: wetL = wetL > 0 ? std::tanh(wetL) : std::tanh(wetL * 0.5f); break;
                    case 2: wetL = std::clamp(wetL, -1.0f, 1.0f); break;
                    case 3: wetL = std::sin(wetL); break;
                }
                wetL *= autoGain;
                float cabL = ampCabFilterL.processSample(wetL) * 1.5f;
                if (ampSimOn) wetL = cabL;
                highL = cleanHighL * (1.0f - mix) + wetL * mix;
                
                // Distort Right
                float wetR = highR * drive;
                switch (distType) {
                    case 0: wetR = std::tanh(wetR); break;
                    case 1: wetR = wetR > 0 ? std::tanh(wetR) : std::tanh(wetR * 0.5f); break;
                    case 2: wetR = std::clamp(wetR, -1.0f, 1.0f); break;
                    case 3: wetR = std::sin(wetR); break;
                }
                wetR *= autoGain;
                float cabR = ampCabFilterR.processSample(wetR) * 1.5f;
                if (ampSimOn) wetR = cabR;
                highR = cleanHighR * (1.0f - mix) + wetR * mix;
            }
            
            // Synthetic Width (Delay the Mid of the High-Band)
            float distMid = (highL + highR) * 0.5f;
            delayLine.pushSample(0, distMid);
            float syntheticSide = delayLine.popSample(0, 15.0f * currentSampleRate / 1000.0f, true);
            
            // Recombine: LR4 perfectly sums lowMono and highL/R !
            leftPtr[sample] = lowMono + highL + (syntheticSide * widthAmt * wideGain);
            rightPtr[sample] = lowMono + highR - (syntheticSide * widthAmt * wideGain);
        }
        
        if (bassFxType == 1) currentBassGR.store(bassCompEnvelope);
        else currentBassGR.store(0.0f);
    }
    else
    {
        // Widener OFF: Process full stereo signal normally
        if (distOn) {
        auto* leftPtr = buffer.getWritePointer(0);
        auto* rightPtr = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : leftPtr;
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float cleanL = leftPtr[sample];
            float cleanR = rightPtr[sample];
            
            float outL = cleanL;
            float outR = cleanR;
            
            if (distOn) {
                float wetL = cleanL * drive;
                switch (distType) {
                    case 0: wetL = std::tanh(wetL); break;
                    case 1: wetL = wetL > 0 ? std::tanh(wetL) : std::tanh(wetL * 0.5f); break;
                    case 2: wetL = std::clamp(wetL, -1.0f, 1.0f); break;
                    case 3: wetL = std::sin(wetL); break;
                }
                wetL *= autoGain;
                float cabL = ampCabFilterL.processSample(wetL) * 1.5f;
                if (ampSimOn) wetL = cabL;
                outL = cleanL * (1.0f - mix) + wetL * mix;
                
                float wetR = cleanR * drive;
                switch (distType) {
                    case 0: wetR = std::tanh(wetR); break;
                    case 1: wetR = wetR > 0 ? std::tanh(wetR) : std::tanh(wetR * 0.5f); break;
                    case 2: wetR = std::clamp(wetR, -1.0f, 1.0f); break;
                    case 3: wetR = std::sin(wetR); break;
                }
                wetR *= autoGain;
                float cabR = ampCabFilterR.processSample(wetR) * 1.5f;
                if (ampSimOn) wetR = cabR;
                outR = cleanR * (1.0f - mix) + wetR * mix;
            }
            
            leftPtr[sample] = outL;
            rightPtr[sample] = outR;
        }
        }
    }
    
    // --- STAGE 3: LA-2A Compressor ---
    if (compOn) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Stereo link peak detect
            float maxPeak = 0.0f;
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                float absSample = std::abs(buffer.getSample(channel, sample));
                if (absSample > maxPeak)
                    maxPeak = absSample;
            }
            
            float peakDb = juce::Decibels::gainToDecibels(maxPeak, -100.0f);
            
            float targetGrDb = 0.0f;
            if (peakDb > thresholdDb)
            {
                // Soft knee approximation
                targetGrDb = thresholdDb - peakDb; 
            }
            
            if (targetGrDb < compEnvelope) // Attack (getting quieter)
            {
                compEnvelope = targetGrDb + attackCoef * (compEnvelope - targetGrDb);
            }
            else // Release (getting louder)
            {
                compEnvelope = targetGrDb + releaseCoef * (compEnvelope - targetGrDb);
            }
            
            float gainReductionLinear = juce::Decibels::decibelsToGain(compEnvelope);
            float totalCompGain = gainReductionLinear * compGainLinear;
            
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                float input = buffer.getSample(channel, sample);
                buffer.setSample(channel, sample, input * totalCompGain);
            }
        }
        currentMasterGR.store(compEnvelope);
    } else {
        currentMasterGR.store(0.0f);
    }
    
    // --- STAGE 4: MASTER OUTPUT TRIM & SATURATION ---
    float finalSat = finalSatParam->load() / 100.0f;
    float glueDrive = 1.0f + (finalSat * 4.0f); // Up to 5x gain into the saturator
    float compensation = 1.0f / std::sqrt(glueDrive); // Simple makeup for the saturation
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        float* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float in = channelData[sample];
            
            if (finalSat > 0.01f) {
                in = std::tanh(in * glueDrive) * compensation;
            }
            
            channelData[sample] = in * makeUp;
        }
    }
    
    // --- STAGE 5: MASTER MIX ---
    float masterMix = masterMixParam->load() / 100.0f;
    if (masterMix < 1.0f) {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            auto* outData = buffer.getWritePointer(channel);
            auto* dryData = dryBuffer.getReadPointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                outData[sample] = (dryData[sample] * (1.0f - masterMix)) + (outData[sample] * masterMix);
            }
        }
    }
    
    // --- STAGE 6: SAFETY SOFT CLIP ---
    if (masterSoftClipParam->load() > 0.5f) {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            auto* outData = buffer.getWritePointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                outData[sample] = std::tanh(outData[sample]);
            }
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout TapeDistAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("DIST_ON", 1), "Distortion On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("DRIVE", 1), "Drive (dB)", juce::NormalisableRange<float>(0.0f, 60.0f, 0.1f), 0.0f));
    
    juce::StringArray distTypes { "Tape", "Tube", "Hard Clip", "Foldback" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("DIST_TYPE", 1), "Dist Type", distTypes, 0));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("AMP_SIM", 1), "Amp Sim", false));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("MIX", 1), "Mix (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("OUTPUT", 1), "Output (dB)", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("WIDE_ON", 1), "Widener On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("CROSSOVER", 1), "Crossover (Hz)", juce::NormalisableRange<float>(20.0f, 2000.0f, 1.0f, 0.5f), 200.0f));
    
    juce::StringArray bassFxTypes { "Off", "Compressor", "Soft Clip" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("BASS_FX_TYPE", 1), "Bass FX Type", bassFxTypes, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("BASS_AMOUNT", 1), "Bass Amount", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("BASS_GAIN", 1), "Bass Gain (dB)", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("BASS_ON", 1), "Bass Comp On", true));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("WIDTH", 1), "Width (%)", juce::NormalisableRange<float>(0.0f, 200.0f, 1.0f), 150.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("WIDE_GAIN", 1), "Wide Gain (dB)", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("COMP_ON", 1), "Compressor On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("PEAK_REDUCTION", 1), "Peak Reduction", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("COMP_GAIN", 1), "Comp Gain (dB)", juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("FINAL_SAT", 1), "Final Sat (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("MASTER_MIX", 1), "Master Mix (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("MASTER_SOFT_CLIP", 1), "Master Soft Clip", false));
    
    return { params.begin(), params.end() };
}

bool TapeDistAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TapeDistAudioProcessor::createEditor()
{
    return new TapeDistAudioProcessorEditor (*this);
}

void TapeDistAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    if (auto xmlState = apvts.copyState().createXml())
        copyXmlToBinary (*xmlState, destData);
}

void TapeDistAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    if (auto xmlState = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapeDistAudioProcessor();
}
