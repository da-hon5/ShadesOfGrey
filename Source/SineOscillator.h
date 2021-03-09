/*
  ==============================================================================

    Author:  Hannes Bradl, hbradl@gmx.at

    This is free source code: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
class SineOscillator
{
public:
    SineOscillator() {}

    void setFrequency(float frequency, float sampleRate)
    {
        auto cyclesPerSample = frequency / sampleRate;
        angleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;
    }

    forcedinline void updateAngle() noexcept
    {
        currentAngle += angleDelta;
        if (currentAngle >= juce::MathConstants<float>::twoPi)
            currentAngle -= juce::MathConstants<float>::twoPi;
    }

    forcedinline float getNextSample() noexcept
    {
        auto currentSample = std::sin(currentAngle);
        updateAngle();
        return currentSample;
    }

private:
    float currentAngle = 0.0f, angleDelta = 0.0f;
};
