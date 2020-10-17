/*
  ==============================================================================

    BackgroundVisualisation.cpp
    Created: 3 Oct 2020 7:14:16pm
    Author:  hon5

  ==============================================================================
*/

#include "BackgroundVisualisation.h"

BackgroundVisualisation::BackgroundVisualisation(int numberofnotes, int numberofintervals, std::vector<float>& partials_ratios, std::vector<float>& amplitudes)
:
root(50.0f),
notesPerOctave(12),
numbofpartials(partials_ratios.size()),
numberofnotes(numberofnotes),
numberofintervals(numberofintervals),
amplitudes(amplitudes),
partials_ratios(partials_ratios),
dissvector(std::vector<float>(numberofnotes + 1)),
intervals(std::vector<float>(numberofintervals))
{
    int x = 0;
}

void BackgroundVisualisation::setIntervals(std::vector<float>& intvls)
{
    jassert(intervals.size() == numberofintervals);
    intervals = intvls;

    update();
}

void BackgroundVisualisation::update()
{
    // preallocate vector memory with defined vector lengths !!!!!!!!!!!!!
    auto allpartials = calculate_frequencies();
    std::vector<float> allamplitudes;
    std::vector<float> amplitudes1 = amplitudes;
    std::vector<float> amplitudes2 = amplitudes;
    std::vector<float> amplitudes3 = amplitudes;
    allamplitudes.insert(allamplitudes.end(), amplitudes1.begin(), amplitudes1.end());
    allamplitudes.insert(allamplitudes.end(), amplitudes2.begin(), amplitudes2.end());
    allamplitudes.insert(allamplitudes.end(), amplitudes3.begin(), amplitudes3.end());

    std::vector<float> partialsnew(numbofpartials);
    std::vector<float> amplitudesnew = amplitudes;
    allamplitudes.insert(allamplitudes.end(), amplitudesnew.begin(), amplitudesnew.end());

    for (float i = 0; i <= numberofnotes; i++) {
        for (int j = 0; j < numbofpartials; j++) {
            partialsnew[j] = root * partials_ratios[j] * pow(2, i / notesPerOctave);
        }
        allpartials.insert(allpartials.end(), partialsnew.begin(), partialsnew.end());
        dissvector[i] = dissmeasure(allpartials, allamplitudes);
        allpartials.erase(allpartials.end() - numbofpartials, allpartials.end());
    }

    float dissvector_max = *max_element(dissvector.begin(), dissvector.end());
    for (int i = 0; i < dissvector.size(); i++)
        dissvector[i] = dissvector[i] / dissvector_max;
}

void BackgroundVisualisation::paint(Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::beige);
    float rectwidth = getWidth() / (float)numberofnotes;
    float rectheight = getHeight();
    for (int i = 0; i < numberofnotes; i++) {
        g.setColour(juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.0f, dissvector[i]));
        g.fillRect(juce::Rectangle<float>(0 + (i * rectwidth), 0, rectwidth, rectheight));
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
    float l_ij = 0.0f; 
    float s = 0.0f; 
    float f_dif = 0.0f;

    const int N = freq.size();

    jassert(freq.size() == amp.size());

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            l_ij = std::min(amp[i], amp[j]);
            s = x_star / (s1 * std::min(freq[i], freq[j]) + s2);
            f_dif = abs(freq[i] - freq[j]);
            d = d + l_ij * (exp(-b1 * s * f_dif) - exp(-b2 * s * f_dif));
        }
    }
    return d / 2;
}

std::vector<float> BackgroundVisualisation::calculate_frequencies()
{
    std::vector<float> allpartials;
    std::vector<float> partials1(numbofpartials);
    for (int i = 0; i < numbofpartials; i++) {
        partials1[i] = root * partials_ratios[i] * intervals[0];
    }
    std::vector<float> partials2(numbofpartials);
    for (int i = 0; i < numbofpartials; i++) {
        partials2[i] = root * partials_ratios[i] * intervals[1];
    }
    std::vector<float> partials3(numbofpartials);
    for (int i = 0; i < numbofpartials; i++) {
        partials3[i] = root * partials_ratios[i] * intervals[2];
    }
    allpartials.insert(allpartials.end(), partials1.begin(), partials1.end());
    allpartials.insert(allpartials.end(), partials2.begin(), partials2.end());
    allpartials.insert(allpartials.end(), partials3.begin(), partials3.end());

    return allpartials;
}