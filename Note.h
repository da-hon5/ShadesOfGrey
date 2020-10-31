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

    std::function<void()> onUpdate;

    void updatePosition(juce::Point<float> pos)
    {
        position = pos;
        //Logger::outputDebugString("new position:" + juce::String(position.getX()));
    }

    juce::Point<float> getPosition()
    {
        return position;
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::green);
        juce::Rectangle<float> ballArea(0, 0, 25, 25);
        //ballArea.setCentre(0, 0);
        g.fillEllipse(ballArea);
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        Logger::outputDebugString("note was clicked");
        mouseDownPosition = getPosition();

    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        auto offsetX = event.getDistanceFromDragStartX();
        auto offsetY = event.getDistanceFromDragStartY();
        //Logger::outputDebugString("new Position:" + juce::String(oldPositionX + offsetX));
        juce::Point<float> newPosition = juce::Point<float> (mouseDownPosition.getX() + offsetX, mouseDownPosition.getY() + offsetY);
        updatePosition(newPosition);
        onUpdate();
    }

    void reset()
    {
        position = juce::Point<float> (-50.0f, -50.0f); //places notes outside of the window where the user can't see them
        
    }

private:
    juce::Point<float> position;
    juce::Point<float> mouseDownPosition;
};
