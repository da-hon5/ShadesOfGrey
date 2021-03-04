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
    BackgroundVisualisation(int numberofintervals, int octaves, int notesPerOctave, float root,
        std::vector<float>& partials_ratios, std::vector<float>& amplitudes);

    void setPartialRatios(std::vector<float>& newPartialRatios) 
    { 
        partialRatios = newPartialRatios; 
        numberOfPartials = partialRatios.size();
        allPartials = std::vector<float>((numberOfIntervals + 1) * numberOfPartials, 0.0f);
    }
    void setAmplitudes(std::vector<float>& newAmplitudes) 
    { 
        amplitudes = newAmplitudes;
        allAmplitudes = std::vector<float>((numberOfIntervals + 1) * numberOfPartials, 0.0f);
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
    void calculate_frequencies();

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
    std::vector<float> allPartials = std::vector<float>((numberOfIntervals + 1) * numberOfPartials, 0.0f);
    std::vector<float> allAmplitudes = std::vector<float>((numberOfIntervals + 1) * numberOfPartials, 0.0f);
};