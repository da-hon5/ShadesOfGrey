/*
  ==============================================================================

    BackgroundVisualisation.cpp
    Created: 3 Oct 2020 7:14:16pm
    Author:  hon5

  ==============================================================================
*/

#include "BackgroundVisualisation.h"

BackgroundVisualisation::BackgroundVisualisation(int numberOfIntervals, int octaves, int notesPerOctave,
    float root, std::vector<float>& partialRatios, std::vector<float>& amplitudes)
: //member initializer list
root(root),
octaves(octaves),
notesPerOctave(notesPerOctave),
numberOfPartials(partialRatios.size()),
numberOfNotes(notesPerOctave * octaves),
numberOfIntervals(numberOfIntervals),
currentDissonance(0.0f),
amplitudes(amplitudes),
partialRatios(partialRatios),
dissvector(std::vector<float>(numberOfNotes)),
intervals(std::vector<float>(numberOfIntervals)),
allPartials(std::vector<float>((numberOfIntervals + 1)* numberOfPartials, 0.0f)),
allAmplitudes(std::vector<float>((numberOfIntervals + 1)* numberOfPartials, 0.0f))
{
}

void BackgroundVisualisation::update()
{    
    for (int i = 0; i < (numberOfIntervals + 1); i++) {
        std::copy(amplitudes.begin(), amplitudes.end(), allAmplitudes.begin() + (i * amplitudes.size())); //all amplitudes + new amplitudes
    }

    calculate_frequencies();

    std::vector<float> newPartials(numberOfPartials, -1.0f);
    currentDissonance = dissmeasure(allPartials, allAmplitudes);
    for (float i = 0; i < numberOfNotes; i++) {
        for (int j = 0; j < numberOfPartials; j++) {
            newPartials[j] = root * partialRatios[j] * std::pow(2, i / notesPerOctave);
        }
        std::copy(newPartials.begin(), newPartials.end(), allPartials.begin() + (numberOfIntervals * numberOfPartials));
        dissvector[i] = dissmeasure(allPartials, allAmplitudes);
    }

    float dissvector_max = *max_element(dissvector.begin(), dissvector.end());
    for (int i = 0; i < dissvector.size(); i++)
        dissvector[i] = dissvector[i] / dissvector_max;

    repaint();
}

void BackgroundVisualisation::paint(Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::beige);
    float rectwidth = getWidth() / (float)numberOfNotes;
    float rectheight = getHeight();
    for (int i = 0; i < numberOfNotes; i++) {
        g.setColour(juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.0f, dissvector[i]));
        g.fillRect(juce::Rectangle<float>(0 + (i * rectwidth), 0, rectwidth, rectheight));
    }
    g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.0f, 0.0f, 0.8f));
    for (int i = 1; i < octaves; i++) {
        g.fillRect(juce::Rectangle<float>(i * getWidth() / octaves, 0, 1.5, getHeight()));
    }
}

float BackgroundVisualisation::dissmeasure(std::vector<float>& freq, std::vector<float>& amp)
// To Do: calculate loudness out of amplitude first??
{
    const float x_star = 0.24f; 
    const float s1 = 0.0207f; 
    const float s2 = 18.96f;
    const float b1 = 3.51f; 
    const float b2 = 5.75f;

    float d = 0.0f; 

    const int N = freq.size();
    jassert(freq.size() == amp.size());

    /*std::vector<float> SPL(N);
    std::vector<float> loudn(N);
    for (int j = 0; j < N; j++) {
        SPL[j] = 2 * log10((amp[j] / 1.41421356) / 0.00002);
        loudn[j] = 0.0625 * pow(2, SPL[j]); 
    }*/

    // exp-function in lookup-table? (std::vector -> length 64?) ... x-values = s * f_dif
    for (int i = 0; i < N; i++) {
        if (freq[i] >= 0) {
            for (int j = 0; j < N; j++) {
                if (freq[j] >= 0) {
                    float l_ij = std::min(amp[i], amp[j]);
                    float s = x_star / (s1 * std::min(freq[i], freq[j]) + s2);
                    float f_dif = std::abs(freq[i] - freq[j]);
                    d += l_ij * (std::exp(-b1 * s * f_dif) - std::exp(-b2 * s * f_dif));
                }
            }
        }
    }
    return d;
}

void BackgroundVisualisation::calculate_frequencies()
{
    for (int j = 0; j < numberOfIntervals; j++) {
        for (int i = 0; i < numberOfPartials; i++) {
            allPartials[i + (j * numberOfPartials)] = root * partialRatios[i] * intervals[j];
        }
    }
}