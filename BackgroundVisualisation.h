/*
  ==============================================================================

    BackgroundVisualisation.h
    Created: 3 Oct 2020 7:14:16pm
    Author:  hon5

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class BackgroundVisualisation : public Component
{
public:
    BackgroundVisualisation(int numberofnotes, int numberofintervals, std::vector<float>& partials_ratios, std::vector<float>& amplitudes);

    void setPartialRatios(std::vector<float>& newPartialRatios) { partials_ratios = newPartialRatios; }
    void setRoot(float newRoot) { root = newRoot; }
    void setNotesPerOctave(int newNotesPerOctave) { notesPerOctave = newNotesPerOctave;  }
    void setIntervals(float int1, float int2, float int3);
    void update();

private:
    void paint(Graphics& g) override;

    float dissmeasure(std::vector<float>& freq, std::vector<float>& amp);
    std::vector<float> calculate_frequencies();

    float root;
    int notesPerOctave;
    int numbofpartials;
    int numberofnotes;
    int numberofintervals;
    std::vector<float> dissvector;
    std::vector<float> partials_ratios;
    std::vector<float> amplitudes;
    std::vector<float> intervals;
};