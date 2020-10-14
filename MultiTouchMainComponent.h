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
using namespace std;
#include "WavetableOscillator.h"
#include "BackgroundVisualisation.h"

//==============================================================================
class MultiTouchMainComponent : public juce::AudioAppComponent,
                                public juce::Timer
{
public:
    MultiTouchMainComponent()
    {
        backgroundVisualisation.reset(new BackgroundVisualisation(numberofnotes, numberofintervals, partials_ratios, amplitudes));
        addAndMakeVisible(backgroundVisualisation.get());
        backgroundVisualisation->setInterceptsMouseClicks(false, true);

        addAndMakeVisible(frequencySlider);
        addAndMakeVisible(frequencySlider2);
        addAndMakeVisible(rootSlider);
        frequencySlider2.setRange(0.0, 1.0);
        frequencySlider.setRange(0.0, 1.0);
        rootSlider.setRange(50.0, 400.0);
        frequencySlider.setValue(0);
        frequencySlider2.setValue(0.5);
        rootSlider.setValue(110);
        frequencySlider.onValueChange = [this] {
            updateFrequency();
        };
        frequencySlider2.onValueChange = [this] {
            updateFrequency();
        };
        rootSlider.onValueChange = [this] {
            updateFrequency();
            backgroundVisualisation->setRoot(rootSlider.getValue());
        };

        createWavetable();

        setSize (800, 600);
        setAudioChannels (0, 2); // no inputs, two outputs
        startTimer (300);
    }

    ~MultiTouchMainComponent() override
    {
        shutdownAudio();
    }

    void paint(juce::Graphics& g) override
    {
    }

    void resized() override
    {
        frequencySlider.setBounds(10, 40, getWidth() - 20, 20);
        frequencySlider2.setBounds(10, 70, getWidth() - 20, 20);
        rootSlider.setBounds(10, 100, getWidth() - 200, 20);
        backgroundVisualisation->setBounds(getBounds());
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        juce::Point<int> xy = event.getMouseDownPosition();
        int x = xy.getX();
        auto s = std::to_string(x);
        Logger::outputDebugString("x-position: " + s);
    }

    void timerCallback() override
    { 
        backgroundVisualisation->update();
        backgroundVisualisation->repaint(); //updates the visuals
    }

    void createWavetable()
    {
        sineTable.setSize (1, (int) tableSize + 1);
        sineTable.clear();

        auto* samples = sineTable.getWritePointer (0);

        jassert (partials_ratios.size() == amplitudes.size());

        for (auto harmonic = 0; harmonic < partials_ratios.size(); ++harmonic)
        {
            //doesn't work with non-integer harmonics => sinus wird irgendwo abgeschnitten (nicht beim nulldurchgang)
            float angleDelta = juce::MathConstants<double>::twoPi / (double)tableSize * partials_ratios[harmonic];
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
        float step1 = round(frequencySlider.getValue() * numberofnotes);
        float step2 = round(frequencySlider2.getValue() * numberofnotes);
        interval1 = pow(2, step1 / notesperoctave);
        interval2 = pow(2, step2 / notesperoctave);
        root = rootSlider.getValue();     //lowest frequency
        freq[0] = interval1 * root;
        freq[1] = interval2 * root;

        backgroundVisualisation->setIntervals(interval1, interval2, interval3);
    }

    void prepareToPlay (int, double sampleRate) override
    {
        auto numberOfOscillators = 2;
        currentSampleRate = sampleRate;
        for (auto i = 0; i < numberOfOscillators; ++i) {
            auto* oscillator = new WavetableOscillator(sineTable);
            updateFrequency();
            oscillator->setFrequency((float)220, (float)sampleRate);
            oscillators.add(oscillator);
        }
          
        level = 0.25f / (float) numberOfOscillators;
    }

    void releaseResources() override {}

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        auto* leftBuffer  = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
        auto* rightBuffer = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);

        bufferToFill.clearActiveBufferRegion();

        for (auto oscillatorIndex = 0; oscillatorIndex < oscillators.size(); ++oscillatorIndex)
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
    juce::Slider frequencySlider;
    juce::Slider frequencySlider2;
    juce::Slider rootSlider;

    std::unique_ptr<BackgroundVisualisation> backgroundVisualisation;

    const unsigned int tableSize = 1 << 7;
    float level = 0.0f;
    std::vector<float> freq = { 0.0f, 0.0f }; //interval
    std::vector<float> partials_ratios = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    std::vector<float> amplitudes = { 1, 0.5, 0.33, 0.25, 0.2, 0.4, 0.7, 0.1, 0.5, 0.6 };
    double currentSampleRate = 0.0;
    float root = 0.0f; // lowest note
    float interval1 = 0.0f;
    float interval2 = 0.0f;
    float interval3 = 0.0f;
    int notesperoctave = 12;
    int octaves = 4;
    int numberofintervals = 3;
    int numberofnotes = notesperoctave * octaves;
    int numbofpartials = partials_ratios.size();
    int numbofamplitudes = amplitudes.size();

    juce::AudioSampleBuffer sineTable;
    juce::OwnedArray<WavetableOscillator> oscillators;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiTouchMainComponent)
};
