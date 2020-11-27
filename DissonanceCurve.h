/*
  ==============================================================================

    DissonanceCurve.h
    Created: 25 Nov 2020 5:03:05pm
    Author:  hon5

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class DissonanceCurve : public Component
{
public:
    DissonanceCurve(int notesPerOct, float root, std::vector<float>& partials_ratios, std::vector<float>& amplitudes)
        : //member initializer list
        root(root),
        notesPerOct(notesPerOct),
        numbofpartials(partials_ratios.size()),
        amplitudes(amplitudes),
        partials_ratios(partials_ratios),
        dissvector(std::vector<float>(numberOfDataPoints)),
        allpartials(std::vector<float>(2 * numbofpartials, 0.0f)),
        allamplitudes(std::vector<float>(2 * numbofpartials, 0.0f))
    {
    }

    void setNotesPerOctave(int newNotesPerOctave) { notesPerOct = newNotesPerOctave; }

    void setPartialRatios(std::vector<float>& newPartialRatios)
    {
        partials_ratios = newPartialRatios;
        numbofpartials = partials_ratios.size();
        allpartials = std::vector<float>(2 * numbofpartials, 0.0f);
    }
    void setAmplitudes(std::vector<float>& newAmplitudes)
    {
        amplitudes = newAmplitudes;
        allamplitudes = std::vector<float>(2 * numbofpartials, 0.0f);
    }
    void setRoot(float newRoot) { root = newRoot; }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey);
        g.setColour(juce::Colours::orange);
        juce::Path path;
        path.startNewSubPath(juce::Point<float>(0, getHeight()));
        for (int i = 0; i < numberOfDataPoints; i++)
        {
            path.lineTo(i * getWidth() / numberOfDataPoints, (1 - dissvector[i]) * getHeight());
        }
        g.strokePath(path, PathStrokeType(1.5f));

        g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.0f, 0.0f, 0.8f));
        for (int i = 1; i < notesPerOct; i++) {
            g.fillRect(juce::Rectangle<float>(i * getWidth() / notesPerOct, 0, 1.0, getHeight()));
        }
    }

    void update()
    {
        std::copy(amplitudes.begin(), amplitudes.end(), allamplitudes.begin()); //amplitudes with amplitudes appended
        std::copy(amplitudes.begin(), amplitudes.end(), allamplitudes.begin() + amplitudes.size());
        
        auto allpartials = calculate_frequencies();

        std::vector<float> partialsnew(numbofpartials);
        for (float i = 0; i < numberOfDataPoints; i++) {
            for (int j = 0; j < numbofpartials; j++) {
                partialsnew[j] = root * partials_ratios[j] * pow(2, i / numberOfDataPoints);
            }
            std::copy(partialsnew.begin(), partialsnew.end(), allpartials.begin() + numbofpartials);
            dissvector[i] = dissmeasure(allpartials, allamplitudes);
        }

        float dissvector_max = *max_element(dissvector.begin(), dissvector.end());
        for (int i = 0; i < dissvector.size(); i++)
            dissvector[i] = dissvector[i] / dissvector_max;

        repaint();
    }

    float dissmeasure(std::vector<float>& freq, std::vector<float>& amp)
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
                d += l_ij * (std::exp(-b1 * s * f_dif) - std::exp(-b2 * s * f_dif));
                // exp-function in lookup-table (std::vector -> length 64?) ... x-values = s * f_dif
            }
        }
        return d;
    }

    std::vector<float> calculate_frequencies()
    {
        for (int i = 0; i < numbofpartials; i++) {
            allpartials[i] = root * partials_ratios[i];
        }
        return allpartials;
    }
    
private:
    const int numberOfDataPoints = 160;
    int notesPerOct;
    float root;
    int numbofpartials;
    std::vector<float> amplitudes;
    std::vector<float> partials_ratios;
    std::vector<float> dissvector;
    std::vector<float> allpartials = std::vector<float>(2 * numbofpartials, 0.0f);
    std::vector<float> allamplitudes = std::vector<float>(2 * numbofpartials, 0.0f);
};

