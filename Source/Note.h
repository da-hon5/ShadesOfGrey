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

class Note : public Component
{
public:
    Note()
    {
    }

    ~Note() = default;

    void updatePosition(juce::Point<float> pos)
    {
        position = pos;
    }

    juce::Point<float> getPosition()
    {
        return position;
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::green);
        juce::Rectangle<float> ballArea(0, 0, 25, 25);
        g.fillEllipse(ballArea);
    }

    void reset()
    {
        position = juce::Point<float> (-50.0f, -50.0f); //places notes outside of the window where the user can't see them
    }

private:
    juce::Point<float> position;
};
