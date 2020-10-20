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
    BackgroundVisualisation(int numberofnotes, int numberofintervals, int octaves, int notesPerOctave, float root, std::vector<float>& partials_ratios, std::vector<float>& amplitudes);

    void setPartialRatios(std::vector<float>& newPartialRatios) { partials_ratios = newPartialRatios; }
    void setRoot(float newRoot) { root = newRoot; }
    void setOctaves(float newOctaves) { octaves = newOctaves; }
    void setNotesPerOctave(int newNotesPerOctave) { notesPerOctave = newNotesPerOctave;  }
    void setIntervals(std::vector<float>& intvls);
    void update();

private:
    void paint(Graphics& g) override;

    float dissmeasure(std::vector<float>& freq, std::vector<float>& amp);
    std::vector<float> calculate_frequencies();

    float root;
    int octaves;
    int notesPerOctave;
    int numbofpartials;
    int numberofnotes;
    int numberofintervals;
    std::vector<float> dissvector;
    std::vector<float> partials_ratios;
    std::vector<float> amplitudes;
    std::vector<float> intervals;
    std::vector<float> allpartials = std::vector<float>((numberofintervals + 1) * numbofpartials, 0.0f);
    std::vector<float> allamplitudes = std::vector<float>((numberofintervals + 1) * numbofpartials, 0.0f);
};