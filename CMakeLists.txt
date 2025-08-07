# CMakeLists.txt for The Brasscaster VST Plugin

cmake_minimum_required(VERSION 3.15)

project(BrasscasterVST VERSION 1.0.0)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)

# Find JUCE
find_package(PkgConfig REQUIRED)

# Add JUCE as a subdirectory (assuming JUCE is in a subdirectory called "JUCE")
add_subdirectory(JUCE)

# Create binary data from Resources
juce_add_binary_data(BrasscasterVSTData
    SOURCES
        Resources/harmonster_puringerdsp.png
        Resources/knob.png
        Resources/harmonster_custom_ui.png
        Resources/harmonster_custom_knob.png
        Resources/poweroff.png
        Resources/poweron.png)

# Create the plugin target
juce_add_plugin(BrasscasterVST
    COMPANY_NAME "Puringer DSP"
    BUNDLE_ID "com.PuringerDSP.Brasscaster"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE PuDa
    PLUGIN_CODE Brsc
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Brasscaster")

# Source files
target_sources(BrasscasterVST
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/BrasscasterDSP.h)

# Target compile definitions
target_compile_definitions(BrasscasterVST
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

# Link JUCE libraries
target_link_libraries(BrasscasterVST
    PRIVATE
        juce::juce_audio_utils
        BrasscasterVSTData
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Project-specific compile definitions
target_compile_definitions(BrasscasterVST PRIVATE
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
)