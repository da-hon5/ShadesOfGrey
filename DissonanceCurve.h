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
    DissonanceCurve(int notesPerOct)
    {
    }

    void setNotesPerOctave(int newNotesPerOctave) { notesPerOct = newNotesPerOctave; }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey);
        g.setColour(juce::Colours::orange);

        juce::Path path;
        path.startNewSubPath(juce::Point<float>(10, 10));
        for (int i = 0; i < numberOfDataPoints; i++) // draw 100 points
        {
            path.lineTo(i, juce::Random::getSystemRandom().nextFloat() * getHeight());
        }
        /*path.lineTo(50, 10);
        path.lineTo(50, 50);
        path.lineTo(10, 50);
        path.closeSubPath();*/
        g.strokePath(path, PathStrokeType(1.5f));

        g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.0f, 0.0f, 0.8f));
        for (int i = 1; i < notesPerOct; i++) {
            g.fillRect(juce::Rectangle<float>(i * getWidth() / notesPerOct, 0, 1.0, getHeight()));
        }
    }

   
    
private:
    int numberOfDataPoints = 160;
    int notesPerOct = 12;
};

