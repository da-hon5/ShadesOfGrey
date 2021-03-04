/*
  ==============================================================================

    Spectrum.h
    Created: 29 Nov 2020 7:30:49pm
    Author:  hon5

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