#include "PluginProcessor.h"
#include "PluginEditor.h"

TapeDistAudioProcessorEditor::TapeDistAudioProcessorEditor (TapeDistAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (1200, 480);

    setupSlider(driveSlider, driveAttachment, "DRIVE");
    
    distTypeComboBox.addItemList({"Tape", "Tube", "Hard Clip", "Foldback"}, 1);
    distTypeComboBox.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(distTypeComboBox);
    distTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "DIST_TYPE", distTypeComboBox);
    
    setupSlider(mixSlider, mixAttachment, "MIX");
    
    setupSlider(crossoverSlider, crossoverAttachment, "CROSSOVER");
    setupSlider(widthSlider, widthAttachment, "WIDTH");
    setupSlider(wideGainSlider, wideGainAttachment, "WIDE_GAIN");
    
    bassFxTypeComboBox.addItemList({"Off", "Compressor", "Soft Clip"}, 1);
    bassFxTypeComboBox.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bassFxTypeComboBox);
    bassFxTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "BASS_FX_TYPE", bassFxTypeComboBox);
    
    setupSlider(bassAmountSlider, bassAmountAttachment, "BASS_AMOUNT");
    setupSlider(bassGainSlider, bassGainAttachment, "BASS_GAIN");
    
    setupSlider(peakReductionSlider, peakReductionAttachment, "PEAK_REDUCTION");
    setupSlider(compGainSlider, compGainAttachment, "COMP_GAIN");
    setupSlider(outputSlider, outputAttachment, "OUTPUT");
    
    addAndMakeVisible(distOnButton);
    distOnAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "DIST_ON", distOnButton);
    
    addAndMakeVisible(ampSimButton);
    ampSimAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "AMP_SIM", ampSimButton);
    
    addAndMakeVisible(wideOnButton);
    wideOnAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "WIDE_ON", wideOnButton);
    addAndMakeVisible(compOnButton);
    compOnAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "COMP_ON", compOnButton);
    
    startTimerHz(30);
}

void TapeDistAudioProcessorEditor::setupSlider(juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, const juce::String& paramID)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(slider);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, paramID, slider);
}

void TapeDistAudioProcessorEditor::timerCallback()
{
    repaint();
}

TapeDistAudioProcessorEditor::~TapeDistAudioProcessorEditor()
{
}

void drawLEDMeter(juce::Graphics& g, juce::Rectangle<float> bounds, float grValueDb, float minDb = -20.0f)
{
    int numLeds = 20;
    float ledWidth = (bounds.getWidth() - (numLeds - 1)) / numLeds;
    
    // Draw background bezel
    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(bounds.expanded(4.0f), 4.0f);
    g.setColour(juce::Colour::fromRGB(60, 60, 60));
    g.drawRoundedRectangle(bounds.expanded(4.0f), 4.0f, 2.0f);
    
    // Calculate how many LEDs are "lit" (from right to left)
    float litProportion = juce::jlimit(0.0f, 1.0f, std::abs(grValueDb) / std::abs(minDb));
    int numLitLeds = juce::roundToInt(litProportion * numLeds);
    
    for (int i = 0; i < numLeds; ++i)
    {
        float x = bounds.getRight() - (i + 1) * ledWidth - i; // 1px gap
        juce::Rectangle<float> ledRect(x, bounds.getY(), ledWidth, bounds.getHeight());
        
        bool isLit = i < numLitLeds;
        
        // Color gradient from Right (yellow) to Left (red)
        float colorPos = (float)i / (float)numLeds; 
        juce::Colour ledColor;
        if (colorPos < 0.3f) ledColor = juce::Colours::yellow;
        else if (colorPos < 0.7f) ledColor = juce::Colours::orange;
        else ledColor = juce::Colours::red;
        
        if (isLit) {
            g.setColour(ledColor);
            g.fillRect(ledRect);
            // subtle glow
            g.setColour(ledColor.withAlpha(0.3f));
            g.fillRect(ledRect.expanded(1.0f));
        } else {
            g.setColour(ledColor.darker(0.8f).withAlpha(0.3f));
            g.fillRect(ledRect);
        }
    }
    
    // Draw markers
    g.setColour(juce::Colours::lightgrey);
    g.setFont(10.0f);
    g.drawText("0", bounds.getRight() - 10, bounds.getBottom() + 4, 20, 10, juce::Justification::centred);
    g.drawText("10", bounds.getX() + bounds.getWidth()/2 - 10, bounds.getBottom() + 4, 20, 10, juce::Justification::centred);
    g.drawText("20", bounds.getX() - 10, bounds.getBottom() + 4, 20, 10, juce::Justification::centred);
}

void TapeDistAudioProcessorEditor::paint (juce::Graphics& g)
{
    // A retro Tape vibe
    g.fillAll (juce::Colour::fromRGB(40, 42, 45));
    g.setColour (juce::Colours::orange);
    g.setFont (24.0f);
    
    auto bounds = getLocalBounds();
    g.drawFittedText ("TapeDist Multi-FX", bounds.removeFromTop(40), juce::Justification::centred, 1);
    
    // Draw 4 panels
    auto distBounds = bounds.removeFromLeft(300);
    auto widBounds = bounds.removeFromLeft(300);
    auto bassBounds = bounds.removeFromLeft(300);
    auto compBounds = bounds;
    
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(distBounds.reduced(5));
    g.drawRect(widBounds.reduced(5));
    g.drawRect(bassBounds.reduced(5));
    g.drawRect(compBounds.reduced(5));
    
    g.setColour(juce::Colours::lightgrey);
    g.setFont(18.0f);
    g.drawText("1. Tape Distortion", distBounds.removeFromTop(30), juce::Justification::centred);
    g.drawText("2. Widener", widBounds.removeFromTop(30), juce::Justification::centred);
    g.drawText("3. Bass FX", bassBounds.removeFromTop(30), juce::Justification::centred);
    g.drawText("4. LA-2A Comp", compBounds.removeFromTop(30), juce::Justification::centred);
    
    g.setFont(14.0f);
    
    // Labels for Distortion
    g.drawText("Drive", distBounds.getX(), distBounds.getY(), 150, 30, juce::Justification::centred);
    g.drawText("Type", distBounds.getX() + 150, distBounds.getY(), 150, 30, juce::Justification::centred);
    g.drawText("Mix", distBounds.getX() + 75, distBounds.getY() + 120, 150, 30, juce::Justification::centred);
    
    // Labels for Widener
    g.drawText("Crossover", widBounds.getX() + 75, widBounds.getY() + 30, 150, 30, juce::Justification::centred);
    g.drawText("Width", widBounds.getX() + 75, widBounds.getY() + 130, 150, 30, juce::Justification::centred);
    g.drawText("Wide Gain", widBounds.getX() + 75, widBounds.getY() + 230, 150, 30, juce::Justification::centred);
    
    // Labels for Bass
    g.drawText("Type", bassBounds.getX() + 75, bassBounds.getY() + 30, 150, 30, juce::Justification::centred);
    g.drawText("Amount", bassBounds.getX() + 75, bassBounds.getY() + 130, 150, 30, juce::Justification::centred);
    g.drawText("Bass Gain", bassBounds.getX() + 75, bassBounds.getY() + 230, 150, 30, juce::Justification::centred);
    
    // Labels for Compressor
    g.drawText("Peak Reduction", compBounds.getX() + 75, compBounds.getY() + 60, 150, 30, juce::Justification::centred);
    g.drawText("Comp Gain", compBounds.getX() + 75, compBounds.getY() + 150, 150, 30, juce::Justification::centred);
    g.drawText("Master Out", compBounds.getX() + 75, compBounds.getY() + 240, 150, 30, juce::Justification::centred);
    
    // Draw Bass VU Meter (Only show when Compressor is active)
    if (static_cast<int>(audioProcessor.apvts.getRawParameterValue("BASS_FX_TYPE")->load()) == 1) {
        juce::Rectangle<float> bassBg(bassBounds.getX() + 85, bassBounds.getY() + 365, 130, 15);
        float bassGR = audioProcessor.currentBassGR.load(); // 0 to -20
        drawLEDMeter(g, bassBg, bassGR);
        
        g.setColour(juce::Colours::lightgrey);
        g.setFont(12.0f);
        g.drawText("BASS GR", bassBg.getX(), bassBg.getY() - 18, bassBg.getWidth(), 14, juce::Justification::centred);
    }
    
    // Draw Master VU Meter
    juce::Rectangle<float> masterBg(compBounds.getX() + 40, compBounds.getY() + 365, compBounds.getWidth() - 80, 15);
    float masterGR = audioProcessor.currentMasterGR.load(); // 0 to -20
    drawLEDMeter(g, masterBg, masterGR);
    
    g.setColour(juce::Colours::lightgrey);
    g.setFont(14.0f);
    g.drawText("MASTER GAIN REDUCTION", masterBg.getX(), masterBg.getY() - 20, masterBg.getWidth(), 20, juce::Justification::centred);
}

void TapeDistAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(40); // header
    
    auto distBounds = bounds.removeFromLeft(300);
    distBounds.removeFromTop(30); // section header
    
    auto widBounds = bounds.removeFromLeft(300);
    widBounds.removeFromTop(30); // section header
    
    auto bassBounds = bounds.removeFromLeft(300);
    bassBounds.removeFromTop(30); // section header
    
    auto compBounds = bounds;
    compBounds.removeFromTop(30); // section header
    
    // Header Buttons
    distOnButton.setBounds(distBounds.getX() + 250, 45, 40, 20);
    wideOnButton.setBounds(widBounds.getX() + 250, 45, 40, 20);
    compOnButton.setBounds(compBounds.getX() + 250, 45, 40, 20);
    
    // Distortion Layout
    driveSlider.setBounds(distBounds.getX(), distBounds.getY() + 30, 150, 90);
    distTypeComboBox.setBounds(distBounds.getX() + 160, distBounds.getY() + 65, 130, 30);
    
    ampSimButton.setBounds(distBounds.getX() + 160, distBounds.getY() + 105, 130, 20);
    
    mixSlider.setBounds(distBounds.getX() + 75, distBounds.getY() + 150, 150, 90);
    
    // Widener Layout
    crossoverSlider.setBounds(widBounds.getX() + 75, widBounds.getY() + 60, 150, 70);
    widthSlider.setBounds(widBounds.getX() + 75, widBounds.getY() + 160, 150, 70);
    wideGainSlider.setBounds(widBounds.getX() + 75, widBounds.getY() + 260, 150, 70);
    
    // Bass Layout
    bassFxTypeComboBox.setBounds(bassBounds.getX() + 85, bassBounds.getY() + 65, 130, 30);
    bassAmountSlider.setBounds(bassBounds.getX() + 75, bassBounds.getY() + 160, 150, 70);
    bassGainSlider.setBounds(bassBounds.getX() + 75, bassBounds.getY() + 260, 150, 70);
    
    // Compressor Layout
    peakReductionSlider.setBounds(compBounds.getX() + 75, compBounds.getY() + 90, 150, 90);
    compGainSlider.setBounds(compBounds.getX() + 75, compBounds.getY() + 180, 150, 90);
    outputSlider.setBounds(compBounds.getX() + 75, compBounds.getY() + 270, 150, 90);
}
