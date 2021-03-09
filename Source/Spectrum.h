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
#include <JuceHeader.h>

class Spectrum : public Component
{
public:
    Spectrum(std::vector<float>& partialRatios, std::vector<float>& amplitudes)
        :
        partialRatios(partialRatios), 
        amplitudes(amplitudes), 
        numberOfPartials(partialRatios.size())
    {
    }

    void setPartialRatios(std::vector<float>& newPartialRatios)
    {
        partialRatios = newPartialRatios;
        numberOfPartials = partialRatios.size();
    }

    void setAmplitudes(std::vector<float>& newAmplitudes)
    {
        amplitudes = newAmplitudes;
    }
   
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey);

        g.setColour(juce::Colours::orange);
        // change this: each line should be a child component which can be dragged by the user
        for (int i = 0; i < numberOfPartials; i++) {
            g.fillRect(juce::Rectangle<float>(partialRatios[i] * getWidth() / numberOfPartials, getHeight(), 1.5f, -getHeight() * amplitudes[i]));
        }

    }

   
private:
    std::vector<float> partialRatios;
    std::vector<float> amplitudes;
    int numberOfPartials;
    
};
