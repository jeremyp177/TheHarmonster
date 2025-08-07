# Brasscaster VST Plugin

## Overview

The **Brasscaster** is a revolutionary audio plugin designed to transform guitar signals into convincing trumpet-like sounds. Using advanced digital signal processing techniques including formant filtering, harmonic enhancement, and dynamic compression, Brasscaster makes your guitar sound like a brass instrument.

## Features

### Core Processing
- **Formant Filtering**: Three-band formant filters tuned to trumpet frequencies (500Hz, 1200Hz, 2500Hz)
- **Harmonic Enhancement**: Adds brass-like overtones and harmonic content
- **Dynamic Processing**: Compression and envelope following to mimic brass instrument dynamics
- **Tube Saturation**: Warm, musical saturation for authentic brass warmth
- **Subtle Vibrato**: Natural vibrato effect typical of brass instruments

### Controls
- **Brightness**: Controls the formant emphasis and overall brightness of the trumpet tone
- **Warmth**: Adjusts saturation amount and harmonic richness
- **Dynamics**: Controls compression ratio and dynamic response
- **Output**: Final output level control
- **Bypass**: Enable/disable the effect

### Factory Presets
- **Bright Trumpet**: Bright, cutting trumpet tone
- **Warm Brass**: Warm, mellow brass sound
- **Jazz Mute**: Muted jazz trumpet style
- **Lead Trumpet**: Powerful lead trumpet sound
- **Flugelhorn**: Soft, round flugelhorn tone
- **Big Band**: Punchy big band section sound

## Building the Plugin

### Prerequisites
- CMake 3.15 or higher
- C++17 compatible compiler
- JUCE framework (included in the JUCE/ directory)

### Build Instructions

1. **Clone/Download the Project**
   ```bash
   cd /path/to/your/projects
   # The Brasscaster folder should contain all the source files
   ```

2. **Create Build Directory**
   ```bash
   cd Brasscaster
   mkdir build
   cd build
   ```

3. **Configure and Build**
   ```bash
   cmake ..
   make
   ```

4. **Alternative: Use CMake GUI**
   - Open CMake GUI
   - Set source directory to the Brasscaster folder
   - Set build directory to Brasscaster/build
   - Configure and generate
   - Build using your IDE or make

### Supported Formats
- VST3
- Audio Unit (AU) - macOS only
- Standalone Application

## Installation

After building, the plugin files will be:
- **VST3**: Copied to system VST3 directory
- **AU**: Copied to system AU directory (macOS)
- **Standalone**: Executable in build directory

## Technical Details

### DSP Algorithm
The Brasscaster uses a sophisticated multi-stage processing chain:

1. **Input Gain & DC Blocking**: Prepares the signal and removes DC offset
2. **Envelope Following**: Tracks signal dynamics for realistic brass response
3. **Dynamic Compression**: Simulates brass instrument compression characteristics
4. **Formant Filtering**: Shapes frequency response using trumpet formant frequencies
5. **Harmonic Enhancement**: Adds brass-like overtones using polynomial waveshaping
6. **Vibrato Processing**: Adds subtle pitch modulation for realism
7. **Tube Saturation**: Provides warm, musical distortion
8. **Output Shaping**: Final EQ and soft limiting

### Formant Frequencies
- **1st Formant**: ~500 Hz (fundamental resonance)
- **2nd Formant**: ~1200 Hz (brightness and presence)
- **3rd Formant**: ~2500 Hz (brilliance and edge)

## Usage Tips

1. **For Best Results**: Use single-note melodies rather than chords
2. **Input Level**: Adjust your guitar's output level for optimal dynamics
3. **Attack Timing**: The plugin responds best to clean, sustained notes
4. **Preset Selection**: Start with factory presets and adjust to taste
5. **Mixing**: Consider adding reverb for more realistic brass section sounds

## System Requirements

- **macOS**: 10.12 or later
- **Windows**: Windows 10 or later
- **Linux**: Most modern distributions
- **CPU**: Modern multi-core processor recommended
- **RAM**: 4GB minimum, 8GB recommended
- **Host**: Any VST3 or AU compatible DAW

## License

This plugin is provided as-is for educational and creative purposes.

## Author

Created by Puringer DSP - A division focused on innovative audio processing solutions.

---

*Transform your guitar into a brass instrument with the power of advanced DSP technology.* 