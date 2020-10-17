/*
  ==============================================================================

    Note.h
    Created: 17 Oct 2020 12:45:43pm
    Author:  hon5

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
        juce::Rectangle<float> ballArea(0, 0, 30, 30);
        ballArea.setCentre(position);
        g.fillEllipse(ballArea);

        /*float x = position.getX();
        auto s = std::to_string(x);
        Logger::outputDebugString("x-position: " + s);*/ 
    }

    void reset()
    {
        position = juce::Point<float> (0.0f, -30.0f); //places notes outside of the window where the user can't see them
        
    }

private:
    juce::Point<float> position;
};
