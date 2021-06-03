/*
  ==============================================================================

    Author:  Hannes Bradl, hbradl@gmx.at

    This is free source code: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

  ==============================================================================
*/

#include "BackgroundVisualisation.h"

BackgroundVisualisation::BackgroundVisualisation(int numberOfIntervals, int octaves, int notesPerOctave,
    float root, std::vector<float>& partialRatios, std::vector<float>& amplitudes)
    : numberOfIntervals(numberOfIntervals),
    octaves(octaves),
    notesPerOctave(notesPerOctave),
    root(root),
    partialRatios(partialRatios),
    amplitudes(amplitudes),
    currentDissonance(0.0f)
{
    numberOfNotes = notesPerOctave * octaves;
    numberOfPartials = partialRatios.size();
    dissvector.resize(numberOfNotes, 0.0f);
    intervals.resize(numberOfIntervals, 0.0f);
    allPartials.resize((size_t)((size_t)numberOfIntervals + 1) * numberOfPartials, 0.0f);
    allAmplitudes.resize((size_t)((size_t)numberOfIntervals + 1) * numberOfPartials, 0.0f);
}

void BackgroundVisualisation::update()
{    
    for (int i = 0; i < (numberOfIntervals + 1); i++)
        std::copy(amplitudes.begin(), amplitudes.end(), allAmplitudes.begin() + (i * amplitudes.size())); //all amplitudes + new amplitudes

    calculateFrequencies();

    std::vector<float> newPartials(numberOfPartials, -1.0f);
    std::copy(newPartials.begin(), newPartials.end(), allPartials.begin() + (numberOfIntervals * numberOfPartials));
    currentDissonance = dissmeasure(allPartials, allAmplitudes);

    for (int i = 0; i < numberOfNotes; i++) 
    {
        for (int j = 0; j < numberOfPartials; j++) 
        {
            newPartials[j] = root * partialRatios[j] * std::pow(2.0f, (float)i / notesPerOctave);
        }
        std::copy(newPartials.begin(), newPartials.end(), allPartials.begin() + (numberOfIntervals * numberOfPartials));
        dissvector[i] = dissmeasure(allPartials, allAmplitudes);
    }

    float dissvector_min = *min_element(dissvector.begin(), dissvector.begin() + numberOfNotes);
    for (int i = 0; i < numberOfNotes; i++)
        dissvector[i] = dissvector[i] - dissvector_min;

    float dissvector_max = *max_element(dissvector.begin(), dissvector.begin() + numberOfNotes);
    for (int i = 0; i < numberOfNotes; i++)
        dissvector[i] = dissvector[i] / dissvector_max;

    repaint();
}

void BackgroundVisualisation::paint(Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::beige);
    float rectwidth = getWidth() / (float)numberOfNotes;
    float rectheight = (float)getHeight();
    for (int i = 0; i < numberOfNotes; i++) 
    {
        g.setColour(juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.0f, dissvector[i]));
        juce::Rectangle<float> rectangle (0.0f + (i * rectwidth), 0.0f, rectwidth, rectheight);
        g.fillRect(rectangle);
        g.setColour(juce::Colours::red);
        g.setFont(10.0f);
        g.drawText(juce::String(std::roundf(100 * dissvector[i])), rectangle, juce::Justification::centredBottom, true);
    }
    g.setColour(juce::Colours::red);
    for (int i = 1; i < octaves; i++)
        g.fillRect(juce::Rectangle<float>(i * (float)getWidth() / octaves, 0.0f, 1.5f, (float)getHeight()));
}

float BackgroundVisualisation::dissmeasure(std::vector<float>& freq, std::vector<float>& amp)
{
    const float x_star = 0.24f; 
    const float s1 = 0.0207f; 
    const float s2 = 18.96f;
    const float b1 = 3.51f; 
    const float b2 = 5.75f;

    float d = 0.0f; 
    
    const int N = freq.size();
    jassert(freq.size() == amp.size());

    //convert amplitudes of sine waves to loudnesses => Sethares (p. 346)
    std::vector<float> loudness(N);
    for (int j = 0; j < N; j++) 
    {
        float SPL = 2 * std::log10((amp[j] / juce::MathConstants<float>::sqrt2) / 0.00002f);
        loudness[j] = 0.0625f * std::pow(2.0f, SPL); 
    }

    //write exp-function in lookup-table? (std::vector -> length 64?) ... x-values = s * f_dif
    for (int i = 0; i < N; i++) 
    {
        if (freq[i] >= 0) 
        {
            for (int j = 0; j < N; j++) 
            {
                if (freq[j] >= 0) 
                {
                    float l_ij = std::min(loudness[i], loudness[j]);
                    float s = x_star / (s1 * std::min(freq[i], freq[j]) + s2);
                    float f_dif = std::abs(freq[i] - freq[j]);
                    d += l_ij * (std::exp(-b1 * s * f_dif) - std::exp(-b2 * s * f_dif));
                }
            }
        }
    }
    return d;
}

void BackgroundVisualisation::calculateFrequencies()
{
    for (int j = 0; j < numberOfIntervals; j++) 
    {
        for (int i = 0; i < numberOfPartials; i++)

            allPartials[i + (j * numberOfPartials)] = root * partialRatios[i] * intervals[j];
    }
}
