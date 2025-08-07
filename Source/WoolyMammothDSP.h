#pragma once
#include <cmath>
#include <algorithm>
#include <array>

//==============================================================================
// Clean ZVEX Woolly Mammoth Circuit Emulation
// Based on original but with minimal changes to prevent cutouts
//==============================================================================

class WoolyMammothDSP
{
public:
    WoolyMammothDSP() = default;
    
    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        
        // Initialize simple anti-aliasing filter
        initializeAntiAliasingFilter();
        
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
        
        // Reset anti-aliasing filter
        antiAlias_x1 = antiAlias_x2 = 0.0;
        antiAlias_y1 = antiAlias_y2 = 0.0;
        
        // Reset supply sag modeling
        current_supply_voltage = nominal_supply_voltage;
        supply_sag_filter = 0.0;
        average_current_draw = 0.0;
        
        // Reset smooth gating (only addition to prevent cutouts)
        gating_smoother = 1.0;
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
        // SMALL CHANGE: Slightly less extreme range to prevent total cutouts
        q2_bias_level = 0.15 + (1.0 - pinch) * 0.65;  // 0.15 to 0.8 bias range (was 0.1 to 0.8)
    }
    
    void setEQ(double value)
    {
        // EQ (10k linear) - passive tone shaping after fuzz
        eq = std::clamp(value, 0.0, 1.0);
        updateFilterCoefficients();
    }
    
    void setOutput(double value)
    {
        // OUTPUT (10k linear) - final volume control with good range
        output = std::clamp(value, 0.0, 1.0);
        output_gain = 0.2 + (output * 3.0);  // More reasonable range: 0.2 to 3.2 gain
    }
    
    double process(double input)
    {
        // MASSIVE INPUT OVERDRIVE STAGE - Built-in aggressive pre-saturation
        double overdriven_input = aggressiveInputOverdrive(input);
        
        // Input DC blocking
        double dc_blocked = dcBlockingFilter(overdriven_input);
        
        // Estimate current consumption from input signal level
        double instantaneous_current = std::abs(dc_blocked) * 0.02;
        
        // Update average current draw with smoothing
        average_current_draw = average_current_draw * 0.999 + instantaneous_current * 0.001;
        
        // Calculate supply voltage with sag
        double supply_voltage = calculateSupplySag(average_current_draw + instantaneous_current * 0.1);
        
        // C1 coupling capacitor (220nF) - AC coupling to Q1
        double c1_coupled = acCouplingFilter(dc_blocked, c1_voltage, 0.999);
        
        // Q1 transistor stage (2N3904) - first amplification with supply-dependent bias
        double q1_out = transistorQ1(c1_coupled, supply_voltage);
        
        // Apply WOOL bass roll-off before Q2 (this is where it affects the circuit)
        double wool_filtered = woolBassFilter(q1_out);
        
        // ADDITIONAL OVERDRIVE between Q1 and Q2 for maximum aggression
        double inter_stage_overdrive = interStageOverdrive(wool_filtered);
        
        // C2 coupling capacitor (10nF) - AC coupling to Q2
        double c2_coupled = acCouplingFilter(inter_stage_overdrive, c2_voltage, 0.995);
        
        // Q2 transistor stage (2N3904) - main fuzz with bias control (PINCH) and supply effects
        double q2_out = transistorQ2Improved(c2_coupled, supply_voltage);
        
        // C6 coupling capacitor (10nF) - AC coupling to output
        double c6_coupled = acCouplingFilter(q2_out, c6_voltage, 0.995);
        
        // EQ passive tone control (post-fuzz)
        double eq_shaped = eqToneControl(c6_coupled);
        
        // Anti-aliasing filter to reduce high-frequency artifacts from nonlinear processing
        double anti_aliased = antiAliasingFilter(eq_shaped);
        
        // Final output gain (also affected by supply voltage)
        double supply_gain_factor = supply_voltage / nominal_supply_voltage;
        double final_out = anti_aliased * output_gain * supply_gain_factor;
        
        // Enhanced soft limiting with more aggressive character
        return aggressiveSoftLimit(final_out);
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
    
    // Supply sag modeling
    static constexpr double nominal_supply_voltage = 9.0;  // Fresh 9V battery
    static constexpr double minimum_supply_voltage = 6.0;  // Dead battery threshold
    static constexpr double battery_internal_resistance = 2.5;  // Ohms (varies with battery age)
    double current_supply_voltage = nominal_supply_voltage;
    double supply_sag_filter = 0.0;  // For supply voltage smoothing
    double average_current_draw = 0.0;  // Running average of current consumption
    
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
    
    // Anti-aliasing filter states
    double antiAlias_x1 = 0.0, antiAlias_x2 = 0.0;
    double antiAlias_y1 = 0.0, antiAlias_y2 = 0.0;
    double antiAlias_b0 = 1.0, antiAlias_b1 = 0.0, antiAlias_b2 = 0.0;
    double antiAlias_a1 = 0.0, antiAlias_a2 = 0.0;
    
    // ONLY ADDITION: Simple gating smoother to prevent cutouts
    double gating_smoother = 1.0;
    
    // NEW: Moderate input overdrive stage - more musical
    double aggressiveInputOverdrive(double input)
    {
        // Moderate input boost and saturation - musical overdrive
        
        // Stage 1: Reasonable input gain boost
        double boosted = input * 3.5;  // Reduced from 8.0 to 3.5
        
        // Stage 2: Softer asymmetric clipping
        double clipped;
        if (boosted > 0.0) {
            clipped = 0.9 * std::tanh(boosted * 1.5);  // Softer positive clipping
        } else {
            clipped = 0.8 * std::tanh(boosted * 1.8);  // Moderate negative clipping
        }
        
        // Stage 3: Moderate harmonic distortion
        double squared = clipped * clipped;
        clipped += squared * 0.15;  // Reduced harmonic content
        
        // Stage 4: Skip bit crushing - too harsh
        
        // Stage 5: Gentle final saturation
        clipped = std::tanh(clipped * 1.2) * 0.85;  // Gentler final stage
        
        return clipped;
    }
    
    // REMOVED: Inter-stage overdrive - was too much
    double interStageOverdrive(double input)
    {
        // Just a gentle boost instead of aggressive overdrive
        return input * 1.3;  // Simple 1.3x boost instead of complex overdrive
    }
    
    // NEW: More reasonable soft limiting
    double aggressiveSoftLimit(double input)
    {
        // Moderate limiting with character but not extreme
        
        // Stage 1: Gentle compression
        double compressed = input / (1.0 + std::abs(input) * 0.5);
        
        // Stage 2: Moderate asymmetric saturation
        double limited;
        if (compressed > 0.0) {
            limited = 0.9 * std::tanh(compressed * 1.8);
        } else {
            limited = 0.85 * std::tanh(compressed * 2.0);
        }
        
        // Stage 3: Add subtle harmonics
        limited += limited * limited * 0.04;  // Gentle harmonic enhancement
        
        return limited;
    }
    
    void initializeAntiAliasingFilter()
    {
        // Design a simple 2nd-order Butterworth low-pass filter
        // Cutoff at about 80% of Nyquist to prevent aliasing
        double cutoff = sampleRate * 0.4;  // 40% of sample rate
        double omega = 2.0 * M_PI * cutoff / sampleRate;
        double cos_omega = std::cos(omega);
        double sin_omega = std::sin(omega);
        double alpha = sin_omega / (2.0 * 0.707);  // Q = 0.707 for Butterworth
        
        // Low-pass biquad coefficients
        antiAlias_b0 = (1.0 - cos_omega) / 2.0;
        antiAlias_b1 = 1.0 - cos_omega;
        antiAlias_b2 = (1.0 - cos_omega) / 2.0;
        double a0 = 1.0 + alpha;
        antiAlias_a1 = -2.0 * cos_omega;
        antiAlias_a2 = 1.0 - alpha;
        
        // Normalize coefficients
        antiAlias_b0 /= a0;
        antiAlias_b1 /= a0;
        antiAlias_b2 /= a0;
        antiAlias_a1 /= a0;
        antiAlias_a2 /= a0;
    }
    
    double antiAliasingFilter(double input)
    {
        // Direct Form II biquad filter
        double result = antiAlias_b0 * input + antiAlias_b1 * antiAlias_x1 + antiAlias_b2 * antiAlias_x2 
                       - antiAlias_a1 * antiAlias_y1 - antiAlias_a2 * antiAlias_y2;
        
        // Update history
        antiAlias_x2 = antiAlias_x1;
        antiAlias_x1 = input;
        antiAlias_y2 = antiAlias_y1;
        antiAlias_y1 = result;
        
        return result;
    }
    
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
    
    double transistorQ1(double input, double supply_voltage)
    {
        // Q1 (2N3904) - First transistor stage with ENHANCED OVERDRIVE
        // Much more aggressive for authentic Woolly Mammoth character
        
        // Supply voltage affects bias point and available headroom
        double supply_factor = supply_voltage / nominal_supply_voltage;
        double bias_adjustment = (1.0 - supply_factor) * 0.3;
        
        // Base-emitter voltage with input signal and supply-dependent bias
        double vbe = input + (q1_bias * 0.7 - bias_adjustment);
        
        // MODERATE GAIN for good overdrive character
        double base_gain = 18.0 * supply_factor;  // Reduced from 25.0 to 18.0 - more reasonable
        
        // Enhanced thermal effects for more saturation
        double thermal_factor = 1.0 + (vbe - 0.7) * 0.2;  // Increased from 0.1
        double effective_gain = base_gain * thermal_factor;
        
        // Base collector current before saturation
        double ic_linear = vbe * effective_gain;
        
        // MORE AGGRESSIVE SATURATION for overdrive character
        double saturation_level = 0.9 * supply_factor;  // Reduced from 1.2 for earlier saturation
        double compression_factor = 0.6 + (1.0 - supply_factor) * 0.2;  // More aggressive compression
        
        // Enhanced saturation curve for more overdrive
        double ic_compressed;
        if (std::abs(ic_linear) > saturation_level * 0.3) {
            // Multi-stage compression for more character
            double stage1 = saturation_level * std::tanh(ic_linear / (saturation_level * compression_factor));
            double stage2 = stage1 / (1.0 + std::abs(stage1) * 0.5);  // Additional compression stage
            ic_compressed = stage2;
        } else {
            ic_compressed = ic_linear;  // Linear region
        }
        
        // ENHANCED ASYMMETRY for more character
        double asymmetry_factor = 1.2 + (1.0 - supply_factor) * 0.3;  // Increased asymmetry
        if (ic_compressed > 0.0) {
            ic_compressed *= (0.9 + (1.0 - supply_factor) * 0.2);  // More positive compression
        } else {
            ic_compressed *= (1.2 * asymmetry_factor);  // Much more negative compression
            ic_compressed = std::max(ic_compressed, -0.8 * supply_factor);
        }
        
        // INCREASED HARMONIC CONTENT for overdrive character
        double harmonic_strength = 0.08 * supply_factor;  // Increased from 0.03
        double harmonic_content = ic_compressed * ic_compressed * harmonic_strength;
        // Add third harmonic for more overdrive character
        double third_harmonic = ic_compressed * ic_compressed * ic_compressed * harmonic_strength * 0.3;
        ic_compressed += harmonic_content + third_harmonic;
        
        // Earlier collector-emitter saturation for more overdrive
        if (std::abs(ic_compressed) > 0.6 * supply_factor) {  // Reduced from 0.8
            double vce_sat = 0.25 + (1.0 - supply_factor) * 0.2;
            double sat_factor = 1.0 - (std::abs(ic_compressed) - 0.6 * supply_factor) * 3.0;  // More aggressive
            ic_compressed *= std::max(sat_factor, vce_sat);
        }
        
        q1_collector = ic_compressed;
        return ic_compressed;
    }
    
    double woolBassFilter(double input)
    {
        // WOOL control - bass roll-off before Q2
        // This is a high-pass filter that removes bass before the main fuzz stage
        
        double alpha = 1.0 / (1.0 + (2.0 * M_PI * wool_cutoff / sampleRate));
        wool_filter_z1 = wool_filter_z1 * alpha + input * (1.0 - alpha);
        
        return input - wool_filter_z1;  // High-pass response
    }
    
    // IMPROVED VERSION: Enhanced overdrive character with minimal changes to prevent cutouts
    double transistorQ2Improved(double input, double supply_voltage)
    {
        // Q2 (2N3904) - Main fuzz transistor with MAXIMUM OVERDRIVE CHARACTER
        // This stage creates the characteristic Woolly Mammoth heavy fuzz
        
        // Supply voltage significantly affects Q2 behavior (fuzz stage more sensitive)
        double supply_factor = supply_voltage / nominal_supply_voltage;
        double supply_bias_shift = (1.0 - supply_factor) * 0.4;
        
        // Base-emitter voltage with bias control from PINCH and supply effects
        double bias_voltage = q2_bias_level * 0.8 - supply_bias_shift;
        double vbe = input + bias_voltage;
        
        // Enhanced gating behavior based on bias starvation AND supply voltage
        double effective_bias_level = q2_bias_level * supply_factor;
        double bias_threshold = effective_bias_level * 0.6;
        double input_amplitude = std::abs(input);
        
        // Transistor activity based on bias point and supply (creates more complex gating)
        double transistor_activity = 1.0;
        if (input_amplitude < bias_threshold) {
            transistor_activity = std::pow(input_amplitude / bias_threshold, 1.5);
            transistor_activity = std::clamp(transistor_activity, 0.05, 1.0);  // Prevent complete cutouts
        }
        
        // Supply sag makes gating more prominent
        transistor_activity *= (0.8 + supply_factor * 0.2);
        
        // Smooth the gating to prevent abrupt changes
        gating_smoother = gating_smoother * 0.98 + transistor_activity * 0.02;
        double smoothed_activity = gating_smoother;
        
        // STRONG GAIN for heavy fuzz character but not extreme
        double base_gain = 50.0 * supply_factor;  // Reduced from 60.0 to 50.0 - still aggressive but more musical
        double bias_gain_factor = 0.3 + effective_bias_level * 2.0;  // More dramatic bias effects
        double effective_gain = base_gain * smoothed_activity * bias_gain_factor;
        
        // Enhanced temperature effects for more aggressive behavior
        double thermal_factor = 1.0 + (1.0 - effective_bias_level) * 0.5 * (2.0 - supply_factor);  // More aggressive
        effective_gain *= thermal_factor;
        
        // Collector current with enhanced modeling
        double ic_linear = vbe * effective_gain;
        
        // MUCH MORE AGGRESSIVE SATURATION for heavy fuzz
        double saturation_level = 0.4 * supply_factor;  // Reduced from 0.6 for earlier, harder saturation
        double compression_factor = 0.25 + (1.0 - supply_factor) * 0.2;  // Much more aggressive compression
        
        // Multi-stage fuzz saturation with supply effects
        double ic_saturated;
        if (ic_linear > 0.0) {
            // Positive saturation with multiple compression stages
            double stage1 = saturation_level * std::tanh(ic_linear / (saturation_level * compression_factor));
            double stage2 = stage1 / (1.0 + stage1 * stage1 * 2.0);  // Additional fuzz compression
            ic_saturated = stage2;
        } else {
            // Negative clipping much more affected by supply sag and more aggressive
            double neg_compression = compression_factor * (0.4 + supply_factor * 0.3);  // More aggressive
            double stage1 = -saturation_level * 0.6 * std::tanh(-ic_linear / (saturation_level * neg_compression));
            double stage2 = stage1 / (1.0 + std::abs(stage1) * 1.5);  // Additional compression
            ic_saturated = stage2;
        }
        
        // ENHANCED FUZZ HARMONIC GENERATION for maximum character
        ic_saturated = addAggressiveFuzzHarmonics(ic_saturated, smoothed_activity, supply_factor);
        
        // More subtle instability effects (no rattling)
        if (smoothed_activity < 0.3) {
            double supply_instability_factor = 1.0 + (1.0 - supply_factor) * 0.3;
            double instability = 0.008 * supply_instability_factor *  // Reduced for no rattling
                                std::sin(input_amplitude * 120.0 + effective_bias_level * 40.0);
            ic_saturated += instability * (0.3 - smoothed_activity) * 0.3;
        }
        
        // Earlier collector-emitter saturation for more fuzz
        if (std::abs(ic_saturated) > 0.3 * supply_factor) {  // Much earlier saturation
            double vce_sat = 0.2 + (1.0 - supply_factor) * 0.25;
            double sat_compression = 1.0 - (std::abs(ic_saturated) - 0.3 * supply_factor) * 3.0;  // More aggressive
            ic_saturated *= std::max(sat_compression, vce_sat);
        }
        
        q2_collector = ic_saturated;
        return ic_saturated;
    }
    
    double addEnhancedFuzzHarmonics(double input, double transistor_activity, double supply_factor)
    {
        // MODERATE harmonic generation for musical overdrive character
        
        double shaped = input;
        
        // Gentler asymmetrical waveshaping
        double positive_compression = 1.6 + transistor_activity * 0.6 * supply_factor;  // Reduced aggression
        double negative_compression = 2.2 - transistor_activity * 0.4 * supply_factor;  // More moderate
        
        if (shaped > 0.0) {
            shaped = shaped / (1.0 + shaped * positive_compression);
        } else {
            shaped = shaped / (1.0 - shaped * negative_compression);
        }
        
        // Moderate harmonic content for musical character
        double harmonic_strength = 0.1 + (1.0 - transistor_activity) * 0.06 + (1.0 - supply_factor) * 0.04;  // Reduced
        
        // Moderate second harmonic for warmth
        shaped += shaped * shaped * harmonic_strength * supply_factor;
        
        // Gentle third harmonic for character
        shaped += shaped * shaped * shaped * harmonic_strength * 0.6 * (2.0 - supply_factor);
        
        // Reduced high-frequency fuzz texture
        double hf_texture_freq = 40.0 + transistor_activity * 30.0 - (1.0 - supply_factor) * 15.0;
        double hf_texture_amount = 0.06 + (1.0 - transistor_activity) * 0.08 + (1.0 - supply_factor) * 0.05;  // Reduced
        shaped += shaped * std::sin(shaped * hf_texture_freq) * hf_texture_amount;
        
        // Less aggressive bit crushing
        double bit_depth = 48.0 + transistor_activity * 24.0 - (1.0 - supply_factor) * 12.0;  // Higher bit depth
        bit_depth = std::max(bit_depth, 20.0);  // Higher minimum for less harshness
        shaped = std::round(shaped * bit_depth) / bit_depth;
        
        return shaped;
    }
    
    // MODERATE: Fuzz harmonics for Q2 stage - musical but characterful
    double addAggressiveFuzzHarmonics(double input, double transistor_activity, double supply_factor)
    {
        // Strong fuzz character but more musical than extreme
        
        double shaped = input;
        
        // Moderate waveshaping for fuzz
        double drive_factor = 1.8 + (1.0 - transistor_activity) * 1.0;  // Reduced drive
        shaped = shaped / (1.0 + std::abs(shaped) * drive_factor);
        
        // Balanced harmonic generation
        double base_strength = 0.12 + (1.0 - transistor_activity) * 0.08;  // Reduced from 0.2
        
        // Strong but musical second harmonic
        shaped += shaped * shaped * base_strength * 1.5;
        
        // Moderate third harmonic for fuzz edge
        shaped += shaped * shaped * shaped * base_strength * 1.0;
        
        // Skip fifth harmonic - was too complex
        
        // Simplified intermodulation
        static double im_delay = 0.0;
        im_delay = im_delay * 0.95 + shaped * 0.05;
        shaped += shaped * im_delay * 0.04;  // Reduced from 0.08
        
        // Gentler crossover distortion
        if (std::abs(shaped) < 0.12) {
            shaped *= 0.7 + 0.3 * transistor_activity;
        }
        
        // Moderate high-frequency saturation
        double hf_sat_freq = 30.0 + transistor_activity * 15.0;  // Reduced frequency
        double hf_sat_amount = 0.06 * (1.3 - transistor_activity);  // Reduced amount
        shaped += shaped * std::sin(shaped * hf_sat_freq) * hf_sat_amount;
        
        // Less aggressive bit reduction
        double bit_depth = 32.0 + transistor_activity * 16.0;  // Higher bit depth
        bit_depth = std::max(bit_depth, 16.0);  // Higher minimum
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

    double calculateSupplySag(double current_load)
    {
        // Simulate supply voltage sagging due to internal resistance and load
        // This is a simplified model. A more accurate simulation would involve
        // a battery model and load changes over time.
        
        // Calculate the voltage drop across the internal resistance
        double voltage_drop = current_load * battery_internal_resistance;
        
        // Apply a smoothing filter to the voltage drop to prevent sudden changes
        supply_sag_filter = supply_sag_filter * 0.99 + voltage_drop * 0.01;
        
        // The actual supply voltage is the nominal voltage minus the smoothed voltage drop
        double supply_voltage = nominal_supply_voltage - supply_sag_filter;
        
        // Ensure the supply voltage doesn't drop below the minimum threshold
        return std::max(supply_voltage, minimum_supply_voltage);
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
            {"Extreme Pinch", 0.5, 1.0, 0.3, 0.4, "Maximum bias starvation"},
            {"Midnight Mass", 1.0, 0.84, 0.64, 0.5, "Aggressively gated, ripping fuzz"}
        };
    }
};