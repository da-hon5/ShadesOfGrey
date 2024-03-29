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

class BackgroundVisualisation : public Component
{
public:
    BackgroundVisualisation(int numberofintervals, int octaves, int notesPerOctave, float root,
        std::vector<float>& partials_ratios, std::vector<float>& amplitudes);

    void setPartialRatios(std::vector<float>& newPartialRatios) 
    { 
        partialRatios = newPartialRatios; 
        numberOfPartials = partialRatios.size();
        allPartials = std::vector<float>(((size_t)numberOfIntervals + 1) * (size_t)numberOfPartials, 0.0f);
    }

    void setAmplitudes(std::vector<float>& newAmplitudes) 
    { 
        amplitudes = newAmplitudes;
        allAmplitudes = std::vector<float>(((size_t)numberOfIntervals + 1) * (size_t)numberOfPartials, 0.0f);
    }

    void setRoot(float newRoot) { root = newRoot; }
    void setOctaves(int newOctaves) { octaves = newOctaves; numberOfNotes = notesPerOctave * octaves; }
    void setNotesPerOctave(int newNotesPerOctave) { notesPerOctave = newNotesPerOctave; numberOfNotes = notesPerOctave * octaves; }
    void setIntervals(std::vector<float>& intvls) { jassert(intervals.size() == numberOfIntervals); intervals = intvls; };
    float getCurrentDissonance() { return currentDissonance; };
    void update();

private:
    void paint(Graphics& g) override;
    float dissmeasure(std::vector<float>& freq, std::vector<float>& amp);
    void calculateFrequencies();
    float root;
    int octaves;
    int notesPerOctave;
    int numberOfPartials;
    int numberOfNotes;
    int numberOfIntervals;
    float currentDissonance;
    std::vector<float> amplitudes;
    std::vector<float> partialRatios;
    std::vector<float> dissvector;
    std::vector<float> intervals;
    std::vector<float> allPartials;
    std::vector<float> allAmplitudes;
};
