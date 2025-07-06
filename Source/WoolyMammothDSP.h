#pragma once
#include <cmath>
#include <algorithm>
#include <array>

//==============================================================================
// Accurate ZVEX Woolly Mammoth Circuit Emulation
// Based on the actual schematic: 2N3904 transistors with proper bias modeling
//==============================================================================

class WoolyMammothDSP
{
public:
    WoolyMammothDSP() = default;
    
    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        updateFilterCoefficients();
        reset();
    }
    
    void reset()
    {
        // Reset all circuit state
        q1_collector = q1_base = q1_emitter = 0.0;
        q2_collector = q2_base = q2_emitter = 0.0;
        
        // Reset coupling capacitors (AC coupling between stages)
        c1_voltage = c2_voltage = c6_voltage = 0.0;
        
        // Reset DC bias points
        q1_bias = 0.5;  // Quiescent bias point
        q2_bias = 0.5;  // This will be modulated by PINCH
        
        // Reset filters
        wool_filter_z1 = 0.0;
        eq_filter_z1 = eq_filter_z2 = 0.0;
        
        // Reset DC blocking
        dc_block_in = dc_block_out = 0.0;
    }
    
    void setWool(double value)
    {
        // WOOL (2k linear) - bass roll-off before fuzz stages
        wool = std::clamp(value, 0.0, 1.0);
        updateFilterCoefficients();
    }
    
    void setPinch(double value)
    {
        // PINCH (500k linear) - controls Q2 bias, creates gated/starved effect
        pinch = std::clamp(value, 0.0, 1.0);
        // Higher pinch = more bias starvation = more gating
        q2_bias_level = 0.1 + (1.0 - pinch) * 0.7;  // 0.1 to 0.8 bias range
    }
    
    void setEQ(double value)
    {
        // EQ (10k linear) - passive tone shaping after fuzz
        eq = std::clamp(value, 0.0, 1.0);
        updateFilterCoefficients();
    }
    
    void setOutput(double value)
    {
        // OUTPUT (10k linear) - final volume control
        output = std::clamp(value, 0.0, 1.0);
        output_gain = 0.2 + (output * 2.0);  // 0.2 to 2.2 gain range
    }
    
    double process(double input)
    {
        // Input DC blocking
        double dc_blocked = dcBlockingFilter(input);
        
        // C1 coupling capacitor (220nF) - AC coupling to Q1
        double c1_coupled = acCouplingFilter(dc_blocked, c1_voltage, 0.999);
        
        // Q1 transistor stage (2N3904) - first amplification
        double q1_out = transistorQ1(c1_coupled);
        
        // Apply WOOL bass roll-off before Q2 (this is where it affects the circuit)
        double wool_filtered = woolBassFilter(q1_out);
        
        // C2 coupling capacitor (10nF) - AC coupling to Q2
        double c2_coupled = acCouplingFilter(wool_filtered, c2_voltage, 0.995);
        
        // Q2 transistor stage (2N3904) - main fuzz with bias control (PINCH)
        double q2_out = transistorQ2(c2_coupled);
        
        // C6 coupling capacitor (10nF) - AC coupling to output
        double c6_coupled = acCouplingFilter(q2_out, c6_voltage, 0.995);
        
        // EQ passive tone control (post-fuzz)
        double eq_shaped = eqToneControl(c6_coupled);
        
        // Final output gain
        double final_out = eq_shaped * output_gain;
        
        // Soft limiting to prevent harsh digital clipping
        return softLimit(final_out);
    }

private:
    // Parameters
    double sampleRate = 44100.0;
    double wool = 0.5;      // WOOL knob (2k linear)
    double pinch = 0.5;     // PINCH knob (500k linear)
    double eq = 0.5;        // EQ knob (10k linear)
    double output = 0.5;    // OUTPUT knob (10k linear)
    
    // Derived parameters
    double q2_bias_level = 0.5;
    double output_gain = 1.0;
    double wool_cutoff = 200.0;
    double eq_cutoff = 2000.0;
    
    // Circuit state variables
    double q1_collector = 0.0, q1_base = 0.0, q1_emitter = 0.0;
    double q2_collector = 0.0, q2_base = 0.0, q2_emitter = 0.0;
    double q1_bias = 0.5, q2_bias = 0.5;
    
    // Coupling capacitors
    double c1_voltage = 0.0;   // C1: 220nF
    double c2_voltage = 0.0;   // C2: 10nF
    double c6_voltage = 0.0;   // C6: 10nF
    
    // Filter states
    double wool_filter_z1 = 0.0;
    double eq_filter_z1 = 0.0, eq_filter_z2 = 0.0;
    
    // DC blocking
    double dc_block_in = 0.0, dc_block_out = 0.0;
    
    void updateFilterCoefficients()
    {
        // WOOL control - bass roll-off before fuzz (high-pass filter)
        // More wool = less bass roll-off = more bass into fuzz
        wool_cutoff = 50.0 + (wool * 300.0);  // 50Hz to 350Hz cutoff
        
        // EQ control - passive tone shaping after fuzz
        // CCW = more bass, CW = more treble
        eq_cutoff = 800.0 + (eq * 2200.0);  // 800Hz to 3000Hz
    }
    
    double dcBlockingFilter(double input)
    {
        // High-pass filter for DC blocking
        double filtered = input - dc_block_in + 0.995 * dc_block_out;
        dc_block_in = input;
        dc_block_out = filtered;
        return filtered;
    }
    
    double acCouplingFilter(double input, double& capacitor_voltage, double time_constant)
    {
        // Simulate AC coupling capacitor
        capacitor_voltage = capacitor_voltage * time_constant + input * (1.0 - time_constant);
        return input - capacitor_voltage;
    }
    
    double transistorQ1(double input)
    {
        // Q1 (2N3904) - First transistor stage
        // Moderate gain, sets up signal for Q2
        
        // Base current (input signal affects bias)
        q1_base = input + q1_bias;
        
        // Collector current (amplified and clipped)
        double gain = 8.0;  // Moderate gain for first stage
        double collector_current = q1_base * gain;
        
        // Transistor saturation modeling
        if (collector_current > 1.0) {
            collector_current = 1.0 + (collector_current - 1.0) * 0.1;  // Soft saturation
        } else if (collector_current < -0.8) {
            collector_current = -0.8 + (collector_current + 0.8) * 0.1;  // Asymmetrical
        }
        
        // Add some nonlinear distortion characteristic of BJT
        collector_current += collector_current * collector_current * collector_current * 0.05;
        
        q1_collector = collector_current;
        return collector_current;
    }
    
    double woolBassFilter(double input)
    {
        // WOOL control - bass roll-off before Q2
        // This is a high-pass filter that removes bass before the main fuzz stage
        
        double alpha = 1.0 / (1.0 + (2.0 * M_PI * wool_cutoff / sampleRate));
        wool_filter_z1 = wool_filter_z1 * alpha + input * (1.0 - alpha);
        
        return input - wool_filter_z1;  // High-pass response
    }
    
    double transistorQ2(double input)
    {
        // Q2 (2N3904) - Main fuzz transistor with bias control (PINCH)
        // This is where the characteristic gated fuzz happens
        
        // Base current with bias control from PINCH
        q2_base = input + (q2_bias_level * 0.8);  // Bias point controlled by PINCH
        
        // When bias is starved (high PINCH), transistor cuts off with low signals
        double bias_threshold = q2_bias_level * 0.5;
        double input_level = std::abs(input);
        
        // Transistor cutoff behavior when bias is starved
        double transistor_activity = 1.0;
        if (input_level < bias_threshold) {
            // Transistor starts to cut off - creates the gated effect
            transistor_activity = input_level / bias_threshold;
            transistor_activity = transistor_activity * transistor_activity;  // Sharper cutoff
        }
        
        // Main amplification with heavy fuzz characteristics
        double gain = 20.0 * transistor_activity;  // Gain depends on transistor activity
        double collector_current = q2_base * gain;
        
        // Hard clipping characteristic of overdriven transistor
        if (collector_current > 0.4) {
            collector_current = 0.4 + (collector_current - 0.4) * 0.02;  // Hard clipping
        } else if (collector_current < -0.3) {
            collector_current = -0.3 + (collector_current + 0.3) * 0.01;  // Asymmetrical hard clipping
        }
        
        // Add fuzz harmonic distortion
        collector_current = addFuzzHarmonics(collector_current);
        
        // Apply the gated effect when transistor is starved
        if (transistor_activity < 0.8) {
            // Add some noise/instability when gating
            double noise_factor = 0.95 + 0.05 * std::sin(input_level * 200.0);
            collector_current *= noise_factor;
        }
        
        q2_collector = collector_current;
        return collector_current;
    }
    
    double addFuzzHarmonics(double input)
    {
        // Add the characteristic fuzz harmonics and texture
        
        // Asymmetrical waveshaping (creates even harmonics)
        double shaped = input;
        if (shaped > 0.0) {
            shaped = shaped / (1.0 + shaped * 2.0);  // Softer positive clipping
        } else {
            shaped = shaped / (1.0 - shaped * 3.0);  // Harder negative clipping
        }
        
        // Add some high-frequency fuzz texture
        shaped += shaped * std::sin(shaped * 50.0) * 0.1;
        
        // Light bit crushing for digital fuzz character
        double bit_depth = 64.0;  // Not too extreme
        shaped = std::round(shaped * bit_depth) / bit_depth;
        
        return shaped;
    }
    
    double eqToneControl(double input)
    {
        // EQ control - passive tone shaping after fuzz
        // Simulates the passive RC filter network
        
        // Two-pole low-pass filter
        double alpha = 1.0 / (1.0 + (2.0 * M_PI * eq_cutoff / sampleRate));
        
        eq_filter_z1 = eq_filter_z1 * alpha + input * (1.0 - alpha);
        eq_filter_z2 = eq_filter_z2 * alpha + eq_filter_z1 * (1.0 - alpha);
        
        // Blend between full-range and filtered based on EQ setting
        double bass_response = eq_filter_z2;
        double treble_response = input - eq_filter_z1;  // High-pass component
        
        // EQ blend: CCW = more bass, CW = more treble
        return bass_response * (1.0 - eq) + treble_response * eq * 0.7;
    }
    
    double softLimit(double input)
    {
        // Soft limiting to prevent harsh digital clipping
        return std::tanh(input * 0.9) * 0.95;
    }
};

//==============================================================================
// Authentic Wooly Mammoth Presets
//==============================================================================

class WoolyMammothPresets
{
public:
    struct Preset
    {
        std::string name;
        double wool;
        double pinch;
        double eq;
        double output;
        std::string description;
    };
    
    static std::vector<Preset> getFactoryPresets()
    {
        return {
            {"Classic Wooly", 0.6, 0.4, 0.3, 0.7, "The authentic Wooly Mammoth sound"},
            {"Velcro Rip", 0.7, 0.8, 0.2, 0.6, "Extreme gated fuzz with velcro texture"},
            {"Bass Destroyer", 0.8, 0.6, 0.1, 0.8, "Maximum bass fuzz destruction"},
            {"Gated Synth", 0.4, 0.9, 0.4, 0.5, "Heavily gated synth bass tones"},
            {"Smooth Fuzz", 0.5, 0.2, 0.6, 0.8, "Less gated, more sustained fuzz"},
            {"Sputtery Gate", 0.3, 0.7, 0.2, 0.6, "Unstable gated fuzz sputter"},
            {"Mild Mammoth", 0.4, 0.3, 0.5, 0.7, "Tamed but still fuzzy"},
            {"Extreme Pinch", 0.5, 1.0, 0.3, 0.4, "Maximum bias starvation"}
        };
    }
};