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
    DissonanceCurve(int notesPerOct, float root, std::vector<float>& partialRatios, std::vector<float>& amplitudes)
        : //member initializer list
        root(root),
        notesPerOct(notesPerOct),
        numberOfPartials(partialRatios.size()),
        amplitudes(amplitudes),
        partialRatios(partialRatios),
        dissvector(std::vector<float>(numberOfDataPoints)),
        allPartials(std::vector<float>(2 * numberOfPartials, 0.0f)),
        allAmplitudes(std::vector<float>(2 * numberOfPartials, 0.0f))
    {
    }

    void setNotesPerOctave(int newNotesPerOctave) { notesPerOct = newNotesPerOctave; }

    void setPartialRatios(std::vector<float>& newPartialRatios)
    {
        partialRatios = newPartialRatios;
        numberOfPartials = partialRatios.size();
        allPartials = std::vector<float>(2 * numberOfPartials, 0.0f);
    }

    void setAmplitudes(std::vector<float>& newAmplitudes)
    {
        amplitudes = newAmplitudes;
        allAmplitudes = std::vector<float>(2 * numberOfPartials, 0.0f);
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

        g.setColour(juce::Colours::grey);
        for (int i = 1; i < notesPerOct; i++) {
            g.fillRect(juce::Rectangle<float>(i * getWidth() / notesPerOct, 0, 0.75f, getHeight()));
        }

        g.setColour(juce::Colours::blue);
        g.fillRect(juce::Rectangle<float>((701.96f / 1200) * getWidth(), 0, 1.3f, getHeight())); //perfect fifth = 701.96 cents
        g.fillRect(juce::Rectangle<float>((498.04f / 1200) * getWidth(), 0, 1.3f, getHeight())); //perfect forth = 498.04 cents
        g.fillRect(juce::Rectangle<float>((386.31f / 1200) * getWidth(), 0, 1.3f, getHeight())); //major third = 386.31 cents
    }

    void update()
    {
        std::copy(amplitudes.begin(), amplitudes.end(), allAmplitudes.begin()); //amplitudes with amplitudes appended
        std::copy(amplitudes.begin(), amplitudes.end(), allAmplitudes.begin() + amplitudes.size());
        
        calculate_frequencies();

        std::vector<float> newPartials(numberOfPartials);
        for (float i = 0; i < numberOfDataPoints; i++) {
            for (int j = 0; j < numberOfPartials; j++) {
                newPartials[j] = root * partialRatios[j] * pow(2, i / numberOfDataPoints);
            }
            std::copy(newPartials.begin(), newPartials.end(), allPartials.begin() + numberOfPartials);
            dissvector[i] = dissmeasure(allPartials, allAmplitudes);
        }

        float dissvector_max = *max_element(dissvector.begin(), dissvector.end());
        for (int i = 0; i < dissvector.size(); i++)
            dissvector[i] = dissvector[i] / dissvector_max;

        repaint();
    }

    float dissmeasure(std::vector<float>& freq, std::vector<float>& amp)
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
        for (int j = 0; j < N; j++) {
            float SPL = 2 * std::log10((amp[j] / juce::MathConstants<float>::sqrt2) / 0.00002);
            loudness[j] = 0.0625 * std::pow(2, SPL);
        }

        //write exp-function in lookup-table? (std::vector -> length 64?) ... x-values = s * f_dif
        for (int i = 0; i < N; i++) {
            if (freq[i] >= 0) {
                for (int j = 0; j < N; j++) {
                    if (freq[j] >= 0) {
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

    void calculate_frequencies()
    {
        for (int i = 0; i < numberOfPartials; i++)
            allPartials[i] = root * partialRatios[i];
    }
    
private:
    const int numberOfDataPoints = 100;
    int notesPerOct;
    float root;
    int numberOfPartials;
    std::vector<float> amplitudes;
    std::vector<float> partialRatios;
    std::vector<float> dissvector;
    std::vector<float> allPartials = std::vector<float>(2 * numberOfPartials, 0.0f);
    std::vector<float> allAmplitudes = std::vector<float>(2 * numberOfPartials, 0.0f);
};

