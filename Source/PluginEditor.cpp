#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// WoolyLookAndFeel Implementation
//==============================================================================

WoolyLookAndFeel::WoolyLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, juce::Colour(0xFFD4A574));
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFF8B4513));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF2F1B14));
    setColour(juce::Label::textColourId, juce::Colour(0xFFF5DEB3));
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF654321));
    setColour(juce::ComboBox::textColourId, juce::Colour(0xFFF5DEB3));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFFD2B48C));
}

void WoolyLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                     const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto centre = bounds.getCentre();
    
    // Fix the angle calculation - this was backwards before
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    // Load the knob image from BinaryData
    auto knobImage = juce::ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);
    
    if (knobImage.isValid())
    {
        // Calculate exact integer positions to avoid floating point precision issues
        auto imageX = static_cast<int>(bounds.getX());
        auto imageY = static_cast<int>(bounds.getY());
        auto imageWidth = static_cast<int>(bounds.getWidth());
        auto imageHeight = static_cast<int>(bounds.getHeight());
        
        // Calculate the exact center using integer arithmetic for precision
        auto imageCentreX = imageX + imageWidth / 2.0f;
        auto imageCentreY = imageY + imageHeight / 2.0f;
        
        // Save the current graphics state
        juce::Graphics::ScopedSaveState saveState(g);
        
        // Apply rotation around the exact center
        g.addTransform(juce::AffineTransform::rotation(angle, imageCentreX, imageCentreY));
        
        // Draw the knob image at exact position
        g.drawImage(knobImage, imageX, imageY, imageWidth, imageHeight, 
                   0, 0, knobImage.getWidth(), knobImage.getHeight());
    }
    else
    {
        // Fallback: draw a simple circle if image fails to load
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f; // Minimal padding
        auto rx = centre.x - radius;
        auto ry = centre.y - radius;
        auto rw = radius * 2.0f;
        
        // Draw basic knob
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.fillEllipse(rx, ry, rw, rw);
        
        // Draw indicator line - fix angle calculation here too
        auto lineLength = radius * 0.8f;
        auto lineX = centre.x + std::cos(angle) * lineLength;
        auto lineY = centre.y + std::sin(angle) * lineLength;
        
        g.setColour(juce::Colour(0xFFFFF8DC));
        g.drawLine(centre.x, centre.y, lineX, lineY, 2.0f);
    }
}

void WoolyLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                 int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box)
{
    // Draw outer shadow for depth
    g.setColour(juce::Colour(0x60000000));
    g.fillRoundedRectangle(1, 1, width - 1, height - 1, 6.0f);
    
    // Main button body with realistic gradient
    juce::ColourGradient buttonGradient(
        juce::Colour(0xFF2A2A2A), 0, 0,           // Lighter at top
        juce::Colour(0xFF0A0A0A), 0, height,      // Darker at bottom
        false);
    buttonGradient.addColour(0.1, juce::Colour(0xFF404040));  // Top highlight
    buttonGradient.addColour(0.9, juce::Colour(0xFF151515));  // Bottom shadow
    
    g.setGradientFill(buttonGradient);
    g.fillRoundedRectangle(0, 0, width, height, 6.0f);
    
    // Add brushed metal texture
    g.setColour(juce::Colour(0x08FFFFFF));
    for (int i = 0; i < height; i += 3)
    {
        g.drawHorizontalLine(i, 0, width);
    }
    
    // Outer rim with gradient
    juce::ColourGradient rimGradient(
        juce::Colour(0xFF505050), 0, 0,
        juce::Colour(0xFF202020), 0, height,
        false);
    g.setGradientFill(rimGradient);
    g.drawRoundedRectangle(0, 0, width, height, 6.0f, 2.0f);
    
    // Cyan accent border
    g.setColour(juce::Colour(0xFF00BFFF));
    g.drawRoundedRectangle(1, 1, width - 2, height - 2, 5.0f, 1.5f);
    
    // Inner glow
    g.setColour(juce::Colour(0x4000BFFF));
    g.drawRoundedRectangle(2, 2, width - 4, height - 4, 4.0f, 1.0f);
    
    // Draw realistic dropdown arrow with 3D effect
    auto arrowArea = juce::Rectangle<int>(buttonX, buttonY, buttonW, buttonH);
    auto centerX = arrowArea.getCentreX();
    auto centerY = arrowArea.getCentreY();
    auto arrowSize = 5.0f;
    
    // Arrow shadow
    g.setColour(juce::Colour(0x80000000));
    juce::Path shadowArrow;
    shadowArrow.addTriangle(centerX - arrowSize + 1, centerY - arrowSize/2 + 1,
                           centerX + arrowSize + 1, centerY - arrowSize/2 + 1,
                           centerX + 1, centerY + arrowSize/2 + 1);
    g.fillPath(shadowArrow);
    
    // Main arrow with gradient
    juce::ColourGradient arrowGradient(
        juce::Colour(0xFF00FFFF), centerX, centerY - arrowSize/2,
        juce::Colour(0xFF0088BB), centerX, centerY + arrowSize/2,
        false);
    g.setGradientFill(arrowGradient);
    
    juce::Path arrow;
    arrow.addTriangle(centerX - arrowSize, centerY - arrowSize/2,
                     centerX + arrowSize, centerY - arrowSize/2,
                     centerX, centerY + arrowSize/2);
    g.fillPath(arrow);
    
    // Arrow highlight
    g.setColour(juce::Colour(0xFFFFFFFF));
    g.strokePath(arrow, juce::PathStrokeType(0.5f));
    
    // Add subtle top highlight to entire button
    g.setColour(juce::Colour(0x20FFFFFF));
    g.fillRoundedRectangle(2, 2, width - 4, height * 0.3f, 4.0f);
}

void WoolyLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto centre = bounds.getCentre();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;
    
    bool isOn = button.getToggleState();
    
    // Draw deep outer shadow for 3D depth
    g.setColour(juce::Colour(0xA0000000));
    g.fillEllipse(centre.x - radius + 2, centre.y - radius + 2, radius * 2, radius * 2);
    
    // Draw secondary shadow for more depth
    g.setColour(juce::Colour(0x60000000));
    g.fillEllipse(centre.x - radius + 1, centre.y - radius + 1, radius * 2, radius * 2);
    
    // Base metallic gradient - sophisticated multi-layer approach with LED illumination
    juce::ColourGradient baseGradient;
    if (isOn)
    {
        // ON state - metal illuminated by red LED with realistic color temperature
        baseGradient = juce::ColourGradient(
            juce::Colour(0xFF9B5513), centre.x, centre.y - radius,     // Warmed bronze at top
            juce::Colour(0xFFEC243C), centre.x, centre.y + radius,     // LED-lit crimson at bottom
            false);
        baseGradient.addColour(0.1, juce::Colour(0xFFFFBB88));        // Hot highlight from LED
        baseGradient.addColour(0.25, juce::Colour(0xFFDD753F));       // Warm metal tone
        baseGradient.addColour(0.4, juce::Colour(0xFFCC3322));        // LED-influenced red
        baseGradient.addColour(0.7, juce::Colour(0xFFFF7347));        // Bright tomato illumination
        baseGradient.addColour(0.9, juce::Colour(0xFFFF4422));        // Deep LED saturation
    }
    else
    {
        // OFF state - cool metallic steel with natural lighting
        baseGradient = juce::ColourGradient(
            juce::Colour(0xFF696969), centre.x, centre.y - radius,     // Dim gray at top
            juce::Colour(0xFF2F2F2F), centre.x, centre.y + radius,     // Dark gray at bottom
            false);
        baseGradient.addColour(0.15, juce::Colour(0xFF778899));       // Light slate gray highlight
        baseGradient.addColour(0.3, juce::Colour(0xFF4F4F4F));        // Dark gray mid
        baseGradient.addColour(0.7, juce::Colour(0xFF555555));        // Slightly lighter gray lower
    }
    
    g.setGradientFill(baseGradient);
    g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2, radius * 2);
    
    // Ultra-realistic machined metal surface simulation
    
    // 1. Variable-density brushed lines with LED illumination effects
    for (int pass = 0; pass < 3; pass++)
    {
        if (isOn)
        {
            // LED-illuminated brushed lines with red tint
            auto baseIntensity = 0x08FFCCCC + pass * 0x04FFAAAA;
            g.setColour(juce::Colour(baseIntensity));
        }
        else
        {
            // Natural aluminum brushed lines
            g.setColour(juce::Colour(0x05FFFFFF + pass * 0x02FFFFFF));
        }
        
        for (int i = -static_cast<int>(radius); i < static_cast<int>(radius); i += (1 + pass))
        {
            auto y = static_cast<int>(centre.y + i);
            auto halfWidth = std::sqrt(radius * radius - i * i);
            auto opacity = 1.0f - (std::abs(i) / radius) * 0.5f; // Fade towards edges
            
            // LED proximity effect - brighter lines near center when LED is on
            if (isOn)
            {
                auto distanceFromCenter = std::abs(i) / radius;
                auto ledProximity = 1.0f - distanceFromCenter;
                opacity *= (0.3f + ledProximity * 0.4f);
            }
            else
            {
                opacity *= 0.3f;
            }
            
            g.setOpacity(opacity);
            g.drawHorizontalLine(y, centre.x - halfWidth, centre.x + halfWidth);
        }
    }
    g.setOpacity(1.0f);
    
    // 2. Anisotropic surface reflections with LED color bleeding
    for (float r = radius * 0.15f; r < radius * 0.98f; r += radius * 0.08f)
    {
        auto thickness = (r < radius * 0.5f) ? 0.8f : 0.4f; // Thicker lines in center
        auto alpha = 1.0f - (r / radius) * 0.7f; // Fade towards edges
        
        if (isOn)
        {
            // LED-affected reflections with warm tinting
            auto distanceFromLED = r / radius;
            auto ledInfluence = 1.0f - distanceFromLED * 0.8f;
            
            // Mix white reflection with red LED color based on proximity
            auto redComponent = 0x25 + static_cast<int>(ledInfluence * 0x40);
            auto greenComponent = 0xFF - static_cast<int>(ledInfluence * 0x80);
            auto blueComponent = 0xFF - static_cast<int>(ledInfluence * 0xCC);
            
            g.setColour(juce::Colour::fromRGB(redComponent, greenComponent, blueComponent));
        }
        else
        {
            // Natural steel reflections
            g.setColour(juce::Colour(0x18FFFFFF));
        }
        
        g.setOpacity(alpha);
        g.drawEllipse(centre.x - r, centre.y - r, r * 2, r * 2, thickness);
    }
    g.setOpacity(1.0f);
    
    // 3. Realistic radial brushing with micro-variations
    g.setColour(juce::Colour(isOn ? 0x12FFFFFF : 0x0AFFFFFF));
    for (int angle = 0; angle < 360; angle += 6)
    {
        auto radians = juce::MathConstants<float>::pi * angle / 180.0f;
        auto innerRadius = radius * (0.2f + (angle % 30) * 0.01f); // Micro-variations
        auto outerRadius = radius * (0.92f + (angle % 20) * 0.005f);
        
        auto x1 = centre.x + std::cos(radians) * innerRadius;
        auto y1 = centre.y + std::sin(radians) * innerRadius;
        auto x2 = centre.x + std::cos(radians) * outerRadius;
        auto y2 = centre.y + std::sin(radians) * outerRadius;
        
        auto lineWidth = 0.2f + (angle % 15) * 0.02f; // Varying line widths
        g.drawLine(x1, y1, x2, y2, lineWidth);
    }
    
    // 4. Realistic wear patterns and surface imperfections
    // Heavy use areas (where thumb would press)
    g.setColour(juce::Colour(isOn ? 0x12000000 : 0x08000000));
    for (int i = 0; i < 12; i++)
    {
        auto angle = juce::MathConstants<float>::pi * 0.3f + i * 0.1f; // Concentrated wear area
        auto r = radius * (0.4f + (i % 3) * 0.2f);
        auto x = centre.x + std::cos(angle) * r;
        auto y = centre.y + std::sin(angle) * r;
        auto size = 1.5f + (i % 2) * 0.5f;
        g.setOpacity(0.6f - (i % 4) * 0.1f);
        g.fillEllipse(x - size, y - size, size * 2, size * 2);
    }
    g.setOpacity(1.0f);
    
    // 5. Micro-scratches and handling marks
    g.setColour(juce::Colour(isOn ? 0x08000000 : 0x05000000));
    for (int i = 0; i < 25; i++)
    {
        auto angle = juce::Random::getSystemRandom().nextFloat() * juce::MathConstants<float>::twoPi;
        auto r = radius * (0.2f + juce::Random::getSystemRandom().nextFloat() * 0.7f);
        auto x = centre.x + std::cos(angle) * r;
        auto y = centre.y + std::sin(angle) * r;
        
        auto scratchAngle = angle + (juce::Random::getSystemRandom().nextFloat() - 0.5f) * 0.5f;
        auto scratchLength = 3.0f + juce::Random::getSystemRandom().nextFloat() * 8.0f;
        auto x2 = x + std::cos(scratchAngle) * scratchLength;
        auto y2 = y + std::sin(scratchAngle) * scratchLength;
        
        g.setOpacity(0.3f + juce::Random::getSystemRandom().nextFloat() * 0.4f);
        g.drawLine(x, y, x2, y2, 0.3f);
    }
    g.setOpacity(1.0f);
    
    // 6. Tool marks from manufacturing
    g.setColour(juce::Colour(isOn ? 0x06FFFFFF : 0x04FFFFFF));
    for (int angle = 30; angle < 360; angle += 60)
    {
        auto radians = juce::MathConstants<float>::pi * angle / 180.0f;
        for (float r = radius * 0.25f; r < radius * 0.9f; r += radius * 0.12f)
        {
            auto x1 = centre.x + std::cos(radians) * r;
            auto y1 = centre.y + std::sin(radians) * r;
            auto x2 = centre.x + std::cos(radians + juce::MathConstants<float>::pi) * r;
            auto y2 = centre.y + std::sin(radians + juce::MathConstants<float>::pi) * r;
            g.setOpacity(0.4f - (r / radius) * 0.2f);
            g.drawLine(x1, y1, x2, y2, 0.15f);
        }
    }
    g.setOpacity(1.0f);
    
    // 7. Subtle oxidation/tarnishing patterns
    if (!isOn) // Only show on inactive button
    {
        g.setColour(juce::Colour(0x08444444));
        for (int i = 0; i < 15; i++)
        {
            auto angle = juce::Random::getSystemRandom().nextFloat() * juce::MathConstants<float>::twoPi;
            auto r = radius * (0.3f + juce::Random::getSystemRandom().nextFloat() * 0.4f);
            auto x = centre.x + std::cos(angle) * r;
            auto y = centre.y + std::sin(angle) * r;
            auto size = 2.0f + juce::Random::getSystemRandom().nextFloat() * 4.0f;
            g.setOpacity(0.2f + juce::Random::getSystemRandom().nextFloat() * 0.3f);
            g.fillEllipse(x - size, y - size, size * 2, size * 2);
        }
    }
    g.setOpacity(1.0f);
    
    // 8. Surface texture variation (like real metal grain)
    g.setColour(juce::Colour(isOn ? 0x04FFFFFF : 0x03FFFFFF));
    for (int i = 0; i < 50; i++)
    {
        auto angle = juce::Random::getSystemRandom().nextFloat() * juce::MathConstants<float>::twoPi;
        auto r = radius * juce::Random::getSystemRandom().nextFloat() * 0.95f;
        auto x = centre.x + std::cos(angle) * r;
        auto y = centre.y + std::sin(angle) * r;
        g.setOpacity(0.1f + juce::Random::getSystemRandom().nextFloat() * 0.2f);
        g.fillEllipse(x - 0.5f, y - 0.5f, 1.0f, 1.0f);
    }
    g.setOpacity(1.0f);
    
    // Sophisticated machined metal rim system
    
    // 1. Outer beveled edge (like machined aluminum)
    g.setColour(juce::Colour(isOn ? 0xFF8B0000 : 0xFF4A4A4A));
    g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2, radius * 2, 3.0f);
    
    // 2. Secondary bevel highlight
    g.setColour(juce::Colour(isOn ? 0xFFFF6347 : 0xFF909090));
    g.drawEllipse(centre.x - radius + 1, centre.y - radius + 1, (radius - 1) * 2, (radius - 1) * 2, 1.5f);
    
    // 3. Inner machined groove
    g.setColour(juce::Colour(isOn ? 0xFF654321 : 0xFF2A2A2A));
    g.drawEllipse(centre.x - radius + 2.5f, centre.y - radius + 2.5f, (radius - 2.5f) * 2, (radius - 2.5f) * 2, 1.0f);
    
    // 4. Raised center platform
    g.setColour(juce::Colour(isOn ? 0xFFCD853F : 0xFF606060));
    g.drawEllipse(centre.x - radius + 4, centre.y - radius + 4, (radius - 4) * 2, (radius - 4) * 2, 0.8f);
    
    // 5. Deep shadow ring for recessed appearance
    g.setColour(juce::Colour(isOn ? 0xFF4A0000 : 0xFF1A1A1A));
    g.drawEllipse(centre.x - radius + 6, centre.y - radius + 6, (radius - 6) * 2, (radius - 6) * 2, 0.5f);
    
    // Ultra-advanced physically-based lighting model
    
    // 1. Primary specular highlight (strong directional light)
    g.setColour(juce::Colour(isOn ? 0xE0FFFFFF : 0xC0FFFFFF));
    juce::Path primarySpecular;
    primarySpecular.addCentredArc(centre.x, centre.y, radius - 2, radius - 2, 0, 
                                 juce::MathConstants<float>::pi * 1.05f, juce::MathConstants<float>::pi * 1.35f, true);
    g.strokePath(primarySpecular, juce::PathStrokeType(5.0f));
    
    // 2. Fresnel effect highlight (viewing angle dependent)
    g.setColour(juce::Colour(isOn ? 0xB0FFFFFF : 0x90FFFFFF));
    juce::Path fresnelHighlight;
    fresnelHighlight.addCentredArc(centre.x, centre.y, radius - 4, radius - 4, 0, 
                                  juce::MathConstants<float>::pi * 1.15f, juce::MathConstants<float>::pi * 1.45f, true);
    g.strokePath(fresnelHighlight, juce::PathStrokeType(3.0f));
    
    // 3. Secondary specular (reflected environment light)
    g.setColour(juce::Colour(isOn ? 0x60FFFFFF : 0x50FFFFFF));
    juce::Path secondarySpecular;
    secondarySpecular.addCentredArc(centre.x, centre.y, radius - 7, radius - 7, 0, 
                                   juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 1.55f, true);
    g.strokePath(secondarySpecular, juce::PathStrokeType(2.0f));
    
    // 4. Realistic cast shadows with falloff
    for (int shadowPass = 0; shadowPass < 3; shadowPass++)
    {
        g.setColour(juce::Colour(isOn ? (0x30000000 + shadowPass * 0x20000000) : (0x40000000 + shadowPass * 0x30000000)));
        juce::Path castShadow;
        auto shadowRadius = radius - 1 - shadowPass * 2;
        auto startAngle = juce::MathConstants<float>::pi * (0.05f + shadowPass * 0.05f);
        auto endAngle = juce::MathConstants<float>::pi * (0.75f - shadowPass * 0.05f);
        castShadow.addCentredArc(centre.x, centre.y, shadowRadius, shadowRadius, 0, startAngle, endAngle, true);
        g.strokePath(castShadow, juce::PathStrokeType(2.0f + shadowPass));
    }
    
    // 5. Ambient occlusion simulation
    g.setColour(juce::Colour(isOn ? 0x25000000 : 0x35000000));
    juce::Path ambientOcclusion;
    ambientOcclusion.addCentredArc(centre.x, centre.y, radius - 12, radius - 12, 0, 
                                  juce::MathConstants<float>::pi * 0.15f, juce::MathConstants<float>::pi * 0.85f, true);
    g.strokePath(ambientOcclusion, juce::PathStrokeType(1.0f));
    
    // 6. Subsurface scattering effect (for metallic depth)
    g.setColour(juce::Colour(isOn ? 0x15FF4444 : 0x10CCCCCC));
    for (float r = radius * 0.3f; r < radius * 0.8f; r += radius * 0.15f)
    {
        g.setOpacity(0.3f - (r / radius) * 0.2f);
        g.drawEllipse(centre.x - r, centre.y - r, r * 2, r * 2, 0.5f);
    }
    g.setOpacity(1.0f);
    
    // 7. Multiple environment reflections (like real studio lighting)
    g.setColour(juce::Colour(isOn ? 0x40FFFFFF : 0x30FFFFFF));
    juce::Path envReflection1;
    envReflection1.addCentredArc(centre.x, centre.y, radius - 5, radius - 5, 0, 
                                juce::MathConstants<float>::pi * 0.8f, juce::MathConstants<float>::pi * 1.0f, true);
    g.strokePath(envReflection1, juce::PathStrokeType(1.5f));
    
    juce::Path envReflection2;
    envReflection2.addCentredArc(centre.x, centre.y, radius - 8, radius - 8, 0, 
                                juce::MathConstants<float>::pi * 1.85f, juce::MathConstants<float>::pi * 2.05f, true);
    g.strokePath(envReflection2, juce::PathStrokeType(1.0f));
    
    // 8. Surface tension highlights (like real metal edges)
    g.setColour(juce::Colour(isOn ? 0x80FFFFFF : 0x60FFFFFF));
    for (int i = 0; i < 8; i++)
    {
        auto angle = juce::MathConstants<float>::pi * i * 0.25f;
        auto r = radius - 1;
        auto x = centre.x + std::cos(angle) * r;
        auto y = centre.y + std::sin(angle) * r;
        g.setOpacity(0.6f - i * 0.05f);
        g.fillEllipse(x - 1, y - 1, 2, 2);
    }
    g.setOpacity(1.0f);
    
    // Cinema-quality LED lighting when active
    if (isOn)
    {
        // 1. Realistic LED lens with multiple refractive layers
        auto ledCoreRadius = radius * 0.12f;
        auto ledLensRadius = radius * 0.18f;
        
        // LED substrate (dark silicon base)
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.fillEllipse(centre.x - ledLensRadius, centre.y - ledLensRadius, ledLensRadius * 2, ledLensRadius * 2);
        
        // LED chip die (bright red core with temperature gradient)
        juce::ColourGradient ledCore(
            juce::Colour(0xFFFFFFFF), centre.x, centre.y - ledCoreRadius * 0.5f,  // White hot center
            juce::Colour(0xFFFF0000), centre.x, centre.y + ledCoreRadius,         // Red edge
            false);
        ledCore.addColour(0.3, juce::Colour(0xFFFFDD44));  // Yellow-white hot zone
        ledCore.addColour(0.6, juce::Colour(0xFFFF4400));  // Orange transition
        g.setGradientFill(ledCore);
        g.fillEllipse(centre.x - ledCoreRadius, centre.y - ledCoreRadius, ledCoreRadius * 2, ledCoreRadius * 2);
        
        // 2. Realistic LED lens with fresnel effects
        g.setColour(juce::Colour(0x40FF0000));
        for (int lensLayer = 0; lensLayer < 4; lensLayer++)
        {
            auto lensR = ledLensRadius - lensLayer * 2;
            auto lensOpacity = 0.15f - lensLayer * 0.03f;
            g.setOpacity(lensOpacity);
            g.drawEllipse(centre.x - lensR, centre.y - lensR, lensR * 2, lensR * 2, 1.0f + lensLayer * 0.5f);
        }
        g.setOpacity(1.0f);
        
        // 3. LED lens caustics and refraction patterns
        g.setColour(juce::Colour(0x60FFFFFF));
        for (int caustic = 0; caustic < 12; caustic++)
        {
            auto angle = juce::MathConstants<float>::pi * caustic * 0.16667f;
            auto causticR = ledLensRadius * (0.7f + (caustic % 3) * 0.1f);
            auto x = centre.x + std::cos(angle) * causticR;
            auto y = centre.y + std::sin(angle) * causticR;
            g.setOpacity(0.3f + (caustic % 2) * 0.2f);
            g.fillEllipse(x - 0.8f, y - 0.8f, 1.6f, 1.6f);
        }
        g.setOpacity(1.0f);
        
        // 4. Volumetric light scattering with realistic falloff
        for (int scatterPass = 0; scatterPass < 8; scatterPass++)
        {
            auto scatterRadius = ledLensRadius + scatterPass * 4;
            auto scatterOpacity = 0.12f * std::exp(-scatterPass * 0.3f); // Exponential falloff
            auto redIntensity = 1.0f - scatterPass * 0.05f;
            auto warmth = 0.2f + scatterPass * 0.02f; // Slight yellow tint at distance
            
            g.setColour(juce::Colour::fromFloatRGBA(redIntensity, warmth * redIntensity, 0.0f, scatterOpacity));
            g.fillEllipse(centre.x - scatterRadius, centre.y - scatterRadius, scatterRadius * 2, scatterRadius * 2);
        }
        
        // 5. Light reflection on machined metal surfaces
        g.setColour(juce::Colour(0x40FF6666));
        for (float reflectR = radius * 0.3f; reflectR < radius * 0.9f; reflectR += radius * 0.12f)
        {
            // Calculate reflection intensity based on surface angle
            auto reflectIntensity = 0.25f * (1.0f - (reflectR - radius * 0.3f) / (radius * 0.6f));
            g.setOpacity(reflectIntensity);
            
            // Draw reflection arcs on machined surface rings
            juce::Path reflection;
            reflection.addCentredArc(centre.x, centre.y, reflectR, reflectR, 0,
                                   juce::MathConstants<float>::pi * 0.9f, juce::MathConstants<float>::pi * 1.6f, true);
            g.strokePath(reflection, juce::PathStrokeType(1.5f));
        }
        g.setOpacity(1.0f);
        
        // 6. LED heat discoloration on metal (like real thermal effects)
        g.setColour(juce::Colour(0x15FFAA44)); // Subtle golden heat tint
        auto heatRadius = ledLensRadius + 8;
        g.fillEllipse(centre.x - heatRadius, centre.y - heatRadius, heatRadius * 2, heatRadius * 2);
        
        // 7. Realistic lens flare/star pattern (like camera lens effects)
        g.setColour(juce::Colour(0x80FF8888));
        for (int flare = 0; flare < 4; flare++)
        {
            auto flareAngle = juce::MathConstants<float>::pi * flare * 0.5f + juce::MathConstants<float>::pi * 0.25f;
            auto flareLength = radius * 0.6f;
            
            auto x1 = centre.x + std::cos(flareAngle) * ledCoreRadius;
            auto y1 = centre.y + std::sin(flareAngle) * ledCoreRadius;
            auto x2 = centre.x + std::cos(flareAngle) * flareLength;
            auto y2 = centre.y + std::sin(flareAngle) * flareLength;
            
            g.setOpacity(0.4f - flare * 0.08f);
            g.drawLine(x1, y1, x2, y2, 1.5f - flare * 0.2f);
            
            // Cross flare
            auto x3 = centre.x + std::cos(flareAngle + juce::MathConstants<float>::pi) * flareLength;
            auto y3 = centre.y + std::sin(flareAngle + juce::MathConstants<float>::pi) * flareLength;
            g.drawLine(x1, y1, x3, y3, 1.5f - flare * 0.2f);
        }
        g.setOpacity(1.0f);
        
        // 8. LED diffraction patterns (rainbow-like splitting)
        g.setColour(juce::Colour(0x30FF6699)); // Slight blue-purple fringe
        g.drawEllipse(centre.x - ledLensRadius - 1, centre.y - ledLensRadius - 1, 
                     (ledLensRadius + 1) * 2, (ledLensRadius + 1) * 2, 0.5f);
        
        // 9. Realistic LED driver current variations (subtle flickering zones)
        g.setColour(juce::Colour(0x20FFDDDD));
        for (int zone = 0; zone < 6; zone++)
        {
            auto zoneAngle = juce::MathConstants<float>::pi * zone * 0.333f;
            auto zoneR = ledCoreRadius * (0.8f + (zone % 2) * 0.3f);
            auto x = centre.x + std::cos(zoneAngle) * zoneR * 0.3f;
            auto y = centre.y + std::sin(zoneAngle) * zoneR * 0.3f;
            g.setOpacity(0.4f + (zone % 3) * 0.15f);
            g.fillEllipse(x - 1, y - 1, 2, 2);
        }
        g.setOpacity(1.0f);
        
        // 10. Photorealistic lens highlight (like real camera reflection)
        g.setColour(juce::Colour(0xC0FFFFFF));
        auto highlightR = ledLensRadius * 0.4f;
        g.fillEllipse(centre.x - highlightR - 3, centre.y - highlightR - 3, highlightR, highlightR);
    }
    
    // Sophisticated machined center indicator
    auto dotRadius = radius * 0.22f;
    
    // Deep center indicator shadow
    g.setColour(juce::Colour(0xA0000000));
    g.fillEllipse(centre.x - dotRadius + 1.5f, centre.y - dotRadius + 1.5f, dotRadius * 2, dotRadius * 2);
    
    // Center indicator base gradient with LED back-illumination
    juce::ColourGradient dotGradient;
    if (isOn)
    {
        // LED-illuminated indicator with realistic light transmission
        dotGradient = juce::ColourGradient(
            juce::Colour(0xFFFFAA66), centre.x, centre.y - dotRadius,     // Hot LED-lit bronze top
            juce::Colour(0xFFFF2200), centre.x, centre.y + dotRadius,     // Bright LED red bottom
            false);
        dotGradient.addColour(0.2, juce::Colour(0xFFFFDD99));            // Bright LED highlight
        dotGradient.addColour(0.5, juce::Colour(0xFFEE4433));            // Mid LED tone
        dotGradient.addColour(0.8, juce::Colour(0xFFCC1100));            // Deep LED saturation
    }
    else
    {
        // Cold metal indicator
        dotGradient = juce::ColourGradient(
            juce::Colour(0xFF909090), centre.x, centre.y - dotRadius,     // Light steel top
            juce::Colour(0xFF404040), centre.x, centre.y + dotRadius,     // Dark steel bottom
            false);
        dotGradient.addColour(0.3, juce::Colour(0xFFB0B0B0));            // Steel highlight
    }
    
    g.setGradientFill(dotGradient);
    g.fillEllipse(centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2, dotRadius * 2);
    
    // Add machined texture to center indicator with LED transmission
    if (isOn)
    {
        // LED-illuminated machined texture with light transmission
        g.setColour(juce::Colour(0x25FFCC88)); // Warm LED-affected highlights
        for (int i = -static_cast<int>(dotRadius); i < static_cast<int>(dotRadius); i += 1)
        {
            auto y = static_cast<int>(centre.y + i);
            auto halfWidth = std::sqrt(dotRadius * dotRadius - i * i);
            if (halfWidth > 0)
            {
                // Vary intensity based on distance from LED center
                auto distanceFromCenter = std::abs(i) / dotRadius;
                auto ledTransmission = 1.0f - distanceFromCenter * 0.6f;
                g.setOpacity(0.4f + ledTransmission * 0.3f);
                g.drawHorizontalLine(y, centre.x - halfWidth, centre.x + halfWidth);
            }
        }
        g.setOpacity(1.0f);
    }
    else
    {
        // Natural metal machined texture
        g.setColour(juce::Colour(0x10FFFFFF));
        for (int i = -static_cast<int>(dotRadius); i < static_cast<int>(dotRadius); i += 1)
        {
            auto y = static_cast<int>(centre.y + i);
            auto halfWidth = std::sqrt(dotRadius * dotRadius - i * i);
            if (halfWidth > 0)
            {
                g.drawHorizontalLine(y, centre.x - halfWidth, centre.x + halfWidth);
            }
        }
    }
    
    // Center indicator beveled rim with LED illumination
    g.setColour(juce::Colour(isOn ? 0xFFCC2200 : 0xFF2A2A2A));
    g.drawEllipse(centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2, dotRadius * 2, 1.5f);
    
    // Center indicator highlight rim with LED edge lighting
    g.setColour(juce::Colour(isOn ? 0xFFFFAA66 : 0xFF808080));
    g.drawEllipse(centre.x - dotRadius + 1, centre.y - dotRadius + 1, (dotRadius - 1) * 2, (dotRadius - 1) * 2, 0.8f);
    
    // Additional LED edge glow for the center indicator
    if (isOn)
    {
        g.setColour(juce::Colour(0x40FF6644));
        g.drawEllipse(centre.x - dotRadius + 0.5f, centre.y - dotRadius + 0.5f, (dotRadius - 0.5f) * 2, (dotRadius - 0.5f) * 2, 0.5f);
    }
    
    // Center dot directional highlight with LED hotspot
    if (isOn)
    {
        // Ultra-bright LED hotspot reflection
        g.setColour(juce::Colour(0xFFFFFFFF));
        auto tinyRadius = dotRadius * 0.2f;
        g.fillEllipse(centre.x - tinyRadius - 1.5f, centre.y - tinyRadius - 1.5f, tinyRadius * 1.5f, tinyRadius * 1.5f);
        
        // LED lens flare reflection
        g.setColour(juce::Colour(0x80FFDDAA));
        auto flareRadius = dotRadius * 0.35f;
        g.fillEllipse(centre.x - flareRadius - 2, centre.y - flareRadius - 2, flareRadius, flareRadius);
    }
    else
    {
        // Standard metal reflection
        g.setColour(juce::Colour(0x80FFFFFF));
        auto tinyRadius = dotRadius * 0.25f;
        g.fillEllipse(centre.x - tinyRadius - 2, centre.y - tinyRadius - 2, tinyRadius * 2, tinyRadius * 2);
    }
    
    // Ultra-fine center point with LED core
    if (isOn)
    {
        // Brilliant LED core point
        g.setColour(juce::Colour(0xFFFFFFFF));
        auto microRadius = dotRadius * 0.08f;
        g.fillEllipse(centre.x - microRadius, centre.y - microRadius, microRadius * 2, microRadius * 2);
        
        // LED phosphor glow around core
        g.setColour(juce::Colour(0x60FFCCAA));
        auto glowRadius = dotRadius * 0.15f;
        g.fillEllipse(centre.x - glowRadius, centre.y - glowRadius, glowRadius * 2, glowRadius * 2);
    }
    else
    {
        // Standard metal center point
        g.setColour(juce::Colour(0xFFCCCCCC));
        auto microRadius = dotRadius * 0.1f;
        g.fillEllipse(centre.x - microRadius, centre.y - microRadius, microRadius * 2, microRadius * 2);
    }
}

//==============================================================================
// WoolyMammothAudioProcessorEditor Implementation
//==============================================================================

WoolyMammothAudioProcessorEditor::WoolyMammothAudioProcessorEditor (WoolyMammothAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set the new HARMONSTER dimensions
    setSize (HarmonsterLayout::PLUGIN_WIDTH, HarmonsterLayout::PLUGIN_HEIGHT);
    setLookAndFeel(&woolyLF);

    // Setup title label
    titleLabel.setText ("THE HARMONSTER", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions ("Arial", 24.0f, juce::Font::bold)));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour(0xFFFF8C00)); // Orange
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (&titleLabel);

    // Setup sliders with tooltips
    auto setupSlider = [this](juce::Slider& slider, const juce::String& tooltip, double defaultValue)
    {
        slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0); // No text box - values only show when dragging
        slider.setRange (0.0, 1.0, 0.001); // Finer resolution for smoother rotation
        slider.setValue (defaultValue);
        slider.addListener (this);
        slider.setTooltip (tooltip);
        
        // Optimize rotation sensitivity for smoother turning
        slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,        // Start angle (216 degrees)
                                  juce::MathConstants<float>::pi * 2.8f,        // End angle (504 degrees) 
                                  true);                                        // Stop at end
        
        slider.setMouseDragSensitivity(100); // Lower value = more sensitive/smoother
        slider.setVelocityBasedMode(true);   // Enable velocity-based dragging
        slider.setVelocityModeParameters(0.9, 1, 0.09, false); // Smoother velocity response
        
        addAndMakeVisible (&slider);
    };

    // Setup EQ knob (leftmost)
    setupSlider(eqSlider, "Adjusts the tone - counterclockwise for darker, clockwise for brighter", 0.5);
    
    // Setup SNARL knob (second from left - this was previously WOOL)
    setupSlider(snarlSlider, "Controls the bass roll-off and fuzz character - the original Woolly Mammoth WOOL control", 0.5);
    
    // Setup PINCH knob (third from left)
    setupSlider(pinchSlider, "Controls the fuzz intensity and gating behavior", 0.3);
    
    // Setup OUTPUT knob (rightmost)
    setupSlider(outputSlider, "Controls the overall output volume of the effect", 0.5);

    // Setup footswitch button
    footswitchButton.setButtonText("");
    footswitchButton.setTooltip("Click to bypass/enable the effect");
    footswitchButton.setToggleState(false, juce::dontSendNotification); // Default to ON (not bypassed)
    addAndMakeVisible(&footswitchButton);

    // Create parameter attachments for the 4 knobs
    eqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "eq", eqSlider);
    snarlAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "wool", snarlSlider);
    pinchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "pinch", pinchSlider);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "output", outputSlider);
    
    // Create parameter attachment for footswitch
    footswitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.parameters, "bypass", footswitchButton);

    // Size already set at beginning of constructor
}

WoolyMammothAudioProcessorEditor::~WoolyMammothAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void WoolyMammothAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    // All parameter updates are handled by the parameter attachments
    // This method can be left empty or used for additional UI updates
}

void WoolyMammothAudioProcessorEditor::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    // No combo boxes in HARMONSTER design
}

void WoolyMammothAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Load the HARMONSTER background image from BinaryData
    auto backgroundImage = juce::ImageCache::getFromMemory(BinaryData::harmonster_puringerdsp_png, 
                                                          BinaryData::harmonster_puringerdsp_pngSize);
    
    if (backgroundImage.isValid())
    {
        // Draw the HARMONSTER background image scaled to fit the plugin window
        // Scale from original 1024x1536 to current window size (360x540)
        g.drawImage(backgroundImage, bounds.toFloat(), 
                   juce::RectanglePlacement::centred | juce::RectanglePlacement::fillDestination);
    }
    else
    {
        // Fallback: solid color background
        g.setColour(juce::Colour(0xFF2D1B3D)); // Dark purple fallback
        g.fillAll();
        
        // Error message
        g.setColour(juce::Colour(0xFFFFFFFF));
        g.setFont(juce::Font(juce::FontOptions(16.0f)));
        g.drawText("HARMONSTER", bounds, juce::Justification::centred);
        
        g.setFont(juce::Font(juce::FontOptions(12.0f)));
        auto messageArea = bounds.removeFromBottom(60);
        g.drawText("Background image could not be loaded from BinaryData", 
                   messageArea, juce::Justification::centred);
    }
}

void WoolyMammothAudioProcessorEditor::resized()
{
    using namespace HarmonsterLayout;
    
    // Position all knobs at exactly the same Y coordinate for perfect alignment
    const int knobY = KNOBS_Y;
    
    // EQ knob (leftmost)
    eqSlider.setBounds(EQ_X, knobY, KNOB_WIDTH, KNOB_HEIGHT);
    
    // SNARL knob (second from left)
    snarlSlider.setBounds(SNARL_X, knobY, KNOB_WIDTH, KNOB_HEIGHT);
    
    // PINCH knob (third from left)
    pinchSlider.setBounds(PINCH_X, knobY, KNOB_WIDTH, KNOB_HEIGHT);
    
    // OUTPUT knob (rightmost)
    outputSlider.setBounds(OUTPUT_X, knobY, KNOB_WIDTH, KNOB_HEIGHT);
    
    // Footswitch button (bottom center)
    footswitchButton.setBounds(FOOTSWITCH_X, FOOTSWITCH_Y, FOOTSWITCH_WIDTH, FOOTSWITCH_HEIGHT);
}