cmake_minimum_required(VERSION 3.20)
project(TapeDist VERSION 1.0.0)

# We use FetchContent to automatically download JUCE
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4   # Use JUCE 8 for macOS 15 compatibility
)
FetchContent_MakeAvailable(juce)

# Create the plugin target
juce_add_plugin(TapeDist
    COMPANY_NAME "Olle"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Olle
    PLUGIN_CODE Tdst
    FORMATS Standalone VST3 AU
)

juce_generate_juce_header(TapeDist)

target_sources(TapeDist
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_definitions(TapeDist
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link everything together, and enable the dsp module
target_link_libraries(TapeDist
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
