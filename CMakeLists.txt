# CMakeLists.txt for The Harmonster VST Plugin

cmake_minimum_required(VERSION 3.15)

project(TheHarmonsterVST VERSION 1.0.0)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)

# Find JUCE
find_package(PkgConfig REQUIRED)

# Add JUCE as a subdirectory (assuming JUCE is in a subdirectory called "JUCE")
# You can also use CPM or other package managers
# Download JUCE from: https://github.com/juce-framework/JUCE
add_subdirectory(JUCE)

# Create binary data from Resources
juce_add_binary_data(TheHarmonsterVSTData
    SOURCES
        Resources/harmonster_puringerdsp.png
        Resources/knob.png)

# Create the plugin target
juce_add_plugin(TheHarmonsterVST
    COMPANY_NAME "Puringer DSP"
    BUNDLE_ID "com.PuringerDSP.TheHarmonster"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE PuDa
    PLUGIN_CODE Harm
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "The Harmonster")

# Source files
target_sources(TheHarmonsterVST
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/WoolyMammothDSP.h)

# Target compile definitions
target_compile_definitions(TheHarmonsterVST
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

# Link JUCE libraries
target_link_libraries(TheHarmonsterVST
    PRIVATE
        juce::juce_audio_utils
        TheHarmonsterVSTData
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Resources are now embedded using BinaryData, no need for file copying

# Create directory structure script
# File: create_project_structure.bat (Windows) or create_project_structure.sh (Unix)

# Windows batch file content (save as create_project_structure.bat):
# @echo off
# mkdir Source
# mkdir JUCE
# echo Project structure created!
# echo Please download JUCE from https://github.com/juce-framework/JUCE
# echo and extract it to the JUCE folder
# echo Then copy the source files to the Source folder
# pause

# Unix shell script content (save as create_project_structure.sh):
#!/bin/bash
# mkdir -p Source
# mkdir -p JUCE
# echo "Project structure created!"
# echo "Please download JUCE from https://github.com/juce-framework/JUCE"
# echo "and extract it to the JUCE folder"
# echo "Then copy the source files to the Source folder"

# Alternative: Using CPM (C++ Package Manager) to automatically download JUCE
# Uncomment the following if you want to use CPM instead of manual JUCE installation:

# include(cmake/CPM.cmake)
# 
# CPMAddPackage(
#     NAME juce
#     GITHUB_REPOSITORY juce-framework/JUCE
#     GIT_TAG 7.0.5
#     SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/JUCE
# )

# Project-specific compile definitions
target_compile_definitions(TheHarmonsterVST PRIVATE
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
)