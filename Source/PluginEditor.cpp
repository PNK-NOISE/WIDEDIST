#include "PluginProcessor.h"
#include "PluginEditor.h"

TapeDistAudioProcessorEditor::TapeDistAudioProcessorEditor (TapeDistAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      driveAttachment(*p.apvts.getParameter("DRIVE"), driveRelay, p.apvts.undoManager),
      distTypeAttachment(*p.apvts.getParameter("DIST_TYPE"), distTypeRelay, p.apvts.undoManager),
      mixAttachment(*p.apvts.getParameter("MIX"), mixRelay, p.apvts.undoManager),
      outputAttachment(*p.apvts.getParameter("OUTPUT"), outputRelay, p.apvts.undoManager),
      distOnAttachment(*p.apvts.getParameter("DIST_ON"), distOnRelay, p.apvts.undoManager),
      ampSimAttachment(*p.apvts.getParameter("AMP_SIM"), ampSimRelay, p.apvts.undoManager),
      crossoverAttachment(*p.apvts.getParameter("CROSSOVER"), crossoverRelay, p.apvts.undoManager),
      widthAttachment(*p.apvts.getParameter("WIDTH"), widthRelay, p.apvts.undoManager),
      wideGainAttachment(*p.apvts.getParameter("WIDE_GAIN"), wideGainRelay, p.apvts.undoManager),
      wideOnAttachment(*p.apvts.getParameter("WIDE_ON"), wideOnRelay, p.apvts.undoManager),
      bassFxTypeAttachment(*p.apvts.getParameter("BASS_FX_TYPE"), bassFxTypeRelay, p.apvts.undoManager),
      bassAmountAttachment(*p.apvts.getParameter("BASS_AMOUNT"), bassAmountRelay, p.apvts.undoManager),
      bassGainAttachment(*p.apvts.getParameter("BASS_GAIN"), bassGainRelay, p.apvts.undoManager),
      bassOnAttachment(*p.apvts.getParameter("BASS_ON"), bassOnRelay, p.apvts.undoManager),
      peakReductionAttachment(*p.apvts.getParameter("PEAK_REDUCTION"), peakReductionRelay, p.apvts.undoManager),
      compGainAttachment(*p.apvts.getParameter("COMP_GAIN"), compGainRelay, p.apvts.undoManager),
      finalSatAttachment(*p.apvts.getParameter("FINAL_SAT"), finalSatRelay, p.apvts.undoManager),
      compOnAttachment(*p.apvts.getParameter("COMP_ON"), compOnRelay, p.apvts.undoManager),
      masterMixAttachment(*p.apvts.getParameter("MASTER_MIX"), masterMixRelay, p.apvts.undoManager),
      masterSoftClipAttachment(*p.apvts.getParameter("MASTER_SOFT_CLIP"), masterSoftClipRelay, p.apvts.undoManager),
      webBrowser(juce::WebBrowserComponent::Options{}
                 .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
                 .withNativeIntegrationEnabled()
                 .withOptionsFrom(driveRelay)
                 .withOptionsFrom(distTypeRelay)
                 .withOptionsFrom(mixRelay)
                 .withOptionsFrom(outputRelay)
                 .withOptionsFrom(distOnRelay)
                 .withOptionsFrom(ampSimRelay)
                 .withOptionsFrom(crossoverRelay)
                 .withOptionsFrom(widthRelay)
                 .withOptionsFrom(wideGainRelay)
                 .withOptionsFrom(wideOnRelay)
                 .withOptionsFrom(bassFxTypeRelay)
                 .withOptionsFrom(bassAmountRelay)
                 .withOptionsFrom(bassGainRelay)
                 .withOptionsFrom(bassOnRelay)
                 .withOptionsFrom(peakReductionRelay)
                 .withOptionsFrom(compGainRelay)
                 .withOptionsFrom(finalSatRelay)
                 .withOptionsFrom(compOnRelay)
                 .withOptionsFrom(masterMixRelay)
                 .withOptionsFrom(masterSoftClipRelay)
                 .withKeepPageLoadedWhenBrowserIsHidden()
                 .withNativeFunction("savePreset", [this](const juce::Array<juce::var>& args, auto complete) {
                     if (args.size() > 0) {
                         audioProcessor.presetManager->savePreset(args[0].toString());
                     }
                     complete(juce::var());
                 })
                 .withNativeFunction("loadPreset", [this](const juce::Array<juce::var>& args, auto complete) {
                     if (args.size() > 0) {
                         audioProcessor.presetManager->loadPreset(args[0].toString());
                     }
                     complete(juce::var());
                 })
                 .withNativeFunction("setKeyboardFocus", [this](const juce::Array<juce::var>& args, auto complete) {
                     if (args.size() > 0) {
                         webBrowser.setWantsKeyboardFocus(args[0]);
                     }
                     complete(juce::var());
                 })
                 .withNativeFunction("getPresets", [this](const juce::Array<juce::var>& args, auto complete) {
                     juce::Array<juce::var> presets;
                     for (const auto& preset : audioProcessor.presetManager->getAllPresets()) {
                         presets.add(preset);
                     }
                     complete(juce::var(presets));
                 })
                 .withNativeFunction("getCurrentPreset", [this](const juce::Array<juce::var>& args, auto complete) {
                     complete(juce::var(audioProcessor.presetManager->getCurrentPresetName()));
                 })
                 .withResourceProvider([this](const auto& url) { return getResource(url); },
                                       juce::WebBrowserComponent::getResourceProviderRoot()))
{
    setSize(1060, 660);
    webBrowser.setWantsKeyboardFocus(false);
    addAndMakeVisible(webBrowser);
    webBrowser.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
    
    startTimerHz(30);
}

TapeDistAudioProcessorEditor::~TapeDistAudioProcessorEditor()
{
    stopTimer();
}

void TapeDistAudioProcessorEditor::timerCallback()
{
    float bassGr = audioProcessor.currentBassGR.load();
    float masterGr = audioProcessor.currentMasterGR.load();
    
    juce::DynamicObject::Ptr obj(new juce::DynamicObject());
    obj->setProperty("bass", bassGr);
    obj->setProperty("master", masterGr);
    webBrowser.emitEventIfBrowserIsVisible("meterData", obj.get());
}

std::optional<juce::WebBrowserComponent::Resource> TapeDistAudioProcessorEditor::getResource(const juce::String& url)
{
    const auto urlToRetrieve = url == "/" ? juce::String("index.html") : url.fromFirstOccurrenceOf("/", false, false);

    if (urlToRetrieve == "index.html")
    {
        return juce::WebBrowserComponent::Resource { 
            std::vector<std::byte>(reinterpret_cast<const std::byte*>(WebAssets::index_html), 
                                   reinterpret_cast<const std::byte*>(WebAssets::index_html) + WebAssets::index_htmlSize), 
            "text/html" 
        };
    }
    
    if (urlToRetrieve == "LOGGA.png")
    {
        return juce::WebBrowserComponent::Resource { 
            std::vector<std::byte>(reinterpret_cast<const std::byte*>(WebAssets::LOGGA_png), 
                                   reinterpret_cast<const std::byte*>(WebAssets::LOGGA_png) + WebAssets::LOGGA_pngSize), 
            "image/png" 
        };
    }

    return std::nullopt;
}

void TapeDistAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Painting is handled by the web browser
    g.fillAll(juce::Colours::black);
}

void TapeDistAudioProcessorEditor::resized()
{
    webBrowser.setBounds(getLocalBounds());
}
