cmake_minimum_required(VERSION 3.24)

set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS" FORCE)

project(WIDEDIST VERSION 1.0.0)

# Build Universal Binary for macOS (Apple Silicon + Intel)
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS" FORCE)
endif()

# We use FetchContent to automatically download JUCE
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4   # Use JUCE 8 for macOS 15 compatibility
)
FetchContent_MakeAvailable(juce)

# Create the plugin target
juce_add_plugin(WIDEDIST
    PLUGIN_NAME "WIDEDIST"
    COMPANY_NAME "PNK NOISE"
    BUNDLE_ID "com.pnknoise.widedist"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE PNKN
    PLUGIN_CODE Wdst
    NEEDS_WEBVIEW2 TRUE
    VST3_AUTO_MANIFEST FALSE
    FORMATS Standalone VST3 AU
)

juce_generate_juce_header(WIDEDIST)

target_sources(WIDEDIST
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/PresetManager.cpp
)

juce_add_binary_data(WebAssets
    HEADER_NAME "WebAssets.h"
    NAMESPACE WebAssets
    SOURCES
        Design/index.html
        LOGGA.png
)

target_compile_definitions(WIDEDIST
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_WIN_WEBVIEW2=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link everything together, and enable the dsp module
target_link_libraries(WIDEDIST
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
        WebAssets
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
