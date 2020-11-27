/*
  ==============================================================================

    BackgroundVisualisation.cpp
    Created: 3 Oct 2020 7:14:16pm
    Author:  hon5

  ==============================================================================
*/

#include "BackgroundVisualisation.h"

BackgroundVisualisation::BackgroundVisualisation(int numberofintervals, int octaves, int notesPerOctave,
    float root, std::vector<float>& partials_ratios, std::vector<float>& amplitudes)
: //member initializer list
root(root),
octaves(octaves),
notesPerOctave(notesPerOctave),
numbofpartials(partials_ratios.size()),
numberOfNotes(notesPerOctave * octaves),
numberofintervals(numberofintervals),
amplitudes(amplitudes),
partials_ratios(partials_ratios),
dissvector(std::vector<float>(numberOfNotes + 1)),
intervals(std::vector<float>(numberofintervals)),
allpartials(std::vector<float>((numberofintervals + 1)* numbofpartials, 0.0f)),
allamplitudes(std::vector<float>((numberofintervals + 1)* numbofpartials, 0.0f))
{
}

void BackgroundVisualisation::setIntervals(std::vector<float>& intvls)
{
    jassert(intervals.size() == numberofintervals);
    intervals = intvls;
    update();
}

void BackgroundVisualisation::update()
{    
    for (int i = 0; i < (numberofintervals + 1); i++) {
        std::copy(amplitudes.begin(), amplitudes.end(), allamplitudes.begin() + (i * amplitudes.size())); //all amplitudes + new amplitudes
    }

    auto allpartials = calculate_frequencies();

    std::vector<float> partialsnew(numbofpartials);
    for (float i = 0; i <= numberOfNotes; i++) {
        for (int j = 0; j < numbofpartials; j++) {
            partialsnew[j] = root * partials_ratios[j] * pow(2, i / notesPerOctave);
        }
        std::copy(partialsnew.begin(), partialsnew.end(), allpartials.begin() + (numberofintervals * numbofpartials));
        dissvector[i] = dissmeasure(allpartials, allamplitudes);
    }

    float dissvector_max = *max_element(dissvector.begin(), dissvector.end());
    for (int i = 0; i < dissvector.size(); i++)
        dissvector[i] = dissvector[i] / dissvector_max;
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

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
           float l_ij = std::min(amp[i], amp[j]);
           float s = x_star / (s1 * std::min(freq[i], freq[j]) + s2);
           float f_dif = std::abs(freq[i] - freq[j]);
           d = d + l_ij * (std::exp(-b1 * s * f_dif) - std::exp(-b2 * s * f_dif)); // exp-function in lookup-table (std::vector -> length 64?) ... x-values = s*f_dif
        }
    }
    return d;
}

std::vector<float> BackgroundVisualisation::calculate_frequencies()
{
    for (int j = 0; j < numberofintervals; j++) {
        for (int i = 0; i < numbofpartials; i++) {
            allpartials[i + (j * numbofpartials)] = root * partials_ratios[i] * intervals[j];
        }
    }
    return allpartials;
}