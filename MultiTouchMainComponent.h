/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             WavetableSynthTutorial
 version:          4.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Wavetable synthesiser.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once
#include "WavetableOscillator.h"
#include "BackgroundVisualisation.h"
#include "Note.h"

//==============================================================================
class MultiTouchMainComponent : public juce::AudioAppComponent,
                                public juce::Timer
{
public:
    MultiTouchMainComponent()
    {
        setWantsKeyboardFocus(true);
        backgroundVisualisation.reset(new BackgroundVisualisation(numbOfNotes, numbOfIntervals, octaves, notesPerOct, root, partialRatios, amplitudes));
        addAndMakeVisible(backgroundVisualisation.get());
        backgroundVisualisation->setInterceptsMouseClicks(false, true);

        for (int i = 0; i < numbOfIntervals; i++)
        {
            auto* newNote = new Note();
            addAndMakeVisible(newNote);
            notes.add(newNote);
            notes[i]->setInterceptsMouseClicks(true, true);
            notes[i]->reset();
            //notes[i]->addMouseListener(this, false);
        }

        userInstructions.setText("Place up to " + juce::String(numbOfIntervals) + " notes with your mouse! Press 'blank key' to remove all notes!", juce::dontSendNotification);
        addAndMakeVisible(userInstructions);
        addAndMakeVisible(rootSlider);
        rootSlider.setRange(50.0, 400.0);
        rootSlider.setValue(80);
        rootSlider.onValueChange = [this] {
            updateFrequency();
            backgroundVisualisation->setRoot(rootSlider.getValue());
        };
        for (int i = 0; i < numbOfIntervals; i++)
        {
            notes[i]->onUpdate = [this] {
                updateFrequency();
                for (auto i = 0; i < numbOfIntervals; i++)
                {
                    notes[i]->setBounds(notes[i]->getPosition().getX() - 12.5, notes[i]->getPosition().getY() - 12.5, 25, 25);
                }
            };
        }

        createWavetable();
        setSize (800, 600);
        setAudioChannels (0, 2); // no inputs, two outputs
        startTimer (100);
    }

    ~MultiTouchMainComponent() override
    {
        shutdownAudio();
    }

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
        userInstructions.setBounds(10, 10, getWidth() - 20, 20);
        rootSlider.setBounds(10, 50, getWidth() - 200, 20);
        backgroundVisualisation->setBounds(getBounds());
        for (auto i = 0; i < numbOfIntervals; i++)
        {
            notes[i]->setBounds(notes[i]->getPosition().getX() - 12.5, notes[i]->getPosition().getY() - 12.5, 25, 25);
        }
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        numbOfClicks++;
        Logger::outputDebugString(juce::String(numbOfClicks));
        for (int i = 0; i < numbOfIntervals; ++i)
        {
            if (numbOfClicks == (i + 1)) {
                notes[i]->updatePosition(event.getMouseDownPosition().toFloat());
                notes[i]->setBounds(notes[i]->getPosition().getX() - 12.5, notes[i]->getPosition().getY() - 12.5, 25, 25);
            }
        }
        updateFrequency();
    }

    bool keyPressed(const KeyPress& k) override 
    {
        if (k.getTextCharacter() == ' ') {
            numbOfClicks = 0;
            for (auto i = 0; i < numbOfIntervals; i++)
            {
                notes[i]->reset();
                notes[i]->setBounds(notes[i]->getPosition().getX() - 12.5, notes[i]->getPosition().getY() - 12.5, 25, 25);
            }
            std::fill(scaleSteps.begin(), scaleSteps.end(), 0.0f);
            std::fill(intervals.begin(), intervals.end(), 0.0f);
            std::fill(freq.begin(), freq.end(), 0.0f);

        } 
        updateFrequency();
        return 0;
    }

    void timerCallback() override
    { 
        backgroundVisualisation->update();
        backgroundVisualisation->repaint();
    }

    void createWavetable()
    {
        sineTable.setSize (1, (int) tableSize + 1);
        sineTable.clear();

        auto* samples = sineTable.getWritePointer (0);

        jassert (partialRatios.size() == amplitudes.size());

        for (auto harmonic = 0; harmonic < partialRatios.size(); ++harmonic)
        {
            //doesn't work with non-integer harmonics => sinus wird irgendwo abgeschnitten (nicht beim nulldurchgang)
            float angleDelta = juce::MathConstants<double>::twoPi / (double)tableSize * partialRatios[harmonic];
            auto currentAngle = 0.0;

            for (unsigned int i = 0; i < tableSize; ++i)
            {
                auto sample = std::sin (currentAngle);
                samples[i] += (float) sample * amplitudes[harmonic];
                currentAngle += angleDelta;
            }
        }
        samples[tableSize] = samples[0];
    }

    void updateFrequency()
    {
        root = rootSlider.getValue();
        for (int i = 0; i < numbOfIntervals; i++) {
            if (notes[i]->getPosition().getX() < 0) {
                intervals[i] = 0.0f;
                freq[i] = 0.0f;
            }
            else {
                scaleSteps[i] = floor(numbOfNotes * (notes[i]->getPosition().getX()) / getWidth());
                intervals[i] = pow(2, scaleSteps[i] / notesPerOct);
                freq[i] = intervals[i] * root;
            }
        }
        backgroundVisualisation->setIntervals(intervals);
    }

    void prepareToPlay (int, double sampleRate) override
    {
        currentSampleRate = sampleRate;
        for (auto i = 0; i < numbOfIntervals; ++i) {
            auto* oscillator = new WavetableOscillator(sineTable);
            oscillators.add(oscillator);
        } 
    }

    void releaseResources() override {}

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        auto* leftBuffer  = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
        auto* rightBuffer = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);

        bufferToFill.clearActiveBufferRegion();

        for (auto oscillatorIndex = 0; oscillatorIndex < numbOfIntervals; ++oscillatorIndex)
        {
            auto* oscillator = oscillators.getUnchecked (oscillatorIndex);
            oscillator->setFrequency((float)freq[oscillatorIndex], (float)currentSampleRate); //set frequency for each oscillator

            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                auto levelSample = oscillator->getNextSample() * level;

                leftBuffer[sample]  += levelSample;
                rightBuffer[sample] += levelSample;
            }
        }
    }

private:
    juce::Slider rootSlider;
    juce::Label userInstructions;

    std::unique_ptr<BackgroundVisualisation> backgroundVisualisation;

    const unsigned int tableSize = 1 << 7;
    const int numbOfIntervals = 8;  //#notes you can play simultaneously
    int notesPerOct = 19;
    int octaves = 2;
    float level = 0.25f / (float) numbOfIntervals;
    std::vector<float> freq = std::vector<float>(numbOfIntervals, 0.0f); //vector with length numbOfIntervals and all zeros
    std::vector<float> intervals = std::vector<float>(numbOfIntervals, 0.0f);
    std::vector<float> scaleSteps = std::vector<float>(numbOfIntervals, 0.0f);
    std::vector<float> partialRatios = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    std::vector<float> amplitudes = { 1, 0.5, 0.33, 0.25, 0.2, 0.4, 0.7, 0.1, 0.5, 0.6 };
    double currentSampleRate = 0.0f;
    float root = 0.0f; 
    int numbOfNotes = notesPerOct * octaves;
    int numbOfPartials = partialRatios.size();
    int numbOfAmplitudes = amplitudes.size();
    int numbOfClicks = 0;
    
    juce::AudioSampleBuffer sineTable;
    juce::OwnedArray<WavetableOscillator> oscillators;
    juce::OwnedArray<Note> notes;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiTouchMainComponent)
};
