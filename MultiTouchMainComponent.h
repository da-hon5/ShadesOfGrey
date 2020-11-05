
/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             MultiTouchInstrument
 description:      A Multi Touch Instrument with Visual Feedback

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
#include "SineOscillator.h"
#include "BackgroundVisualisation.h"
#include "Note.h"

//==============================================================================
class MultiTouchMainComponent : public juce::AudioAppComponent,
                                public juce::Timer
{
public:
    MultiTouchMainComponent()
    {
        /********************** Initialize Member Variables ********************************/
        numbOfIntervals = 5;  //#notes you can play simultaneously
        notesPerOct = 100; //BUG: initialize notesPerOct with maxNotesPerOct => otherwise Error
        octaves = 6;
        numbOfNotes = notesPerOct * octaves;
        lowestOctave = -1;
        tuning = 440;
        root = lowestOctave * tuning;
        numbOfPartials = 5;
        freq = std::vector<float>(numbOfIntervals, 0.0f);
        intervals = std::vector<float>(numbOfIntervals, 0.0f);
        scaleSteps = std::vector<float>(numbOfIntervals, 0.0f);
        partialRatios = std::vector<float>(numbOfPartials, 0.0f);
        amplitudes = std::vector<float>(numbOfPartials, 0.0f);
        currentSampleRate = 0.0f;
        numbOfClicks = 0;

        // choose other (inharmonic) spectrum here:
        /*partialRatios = { 1, 2.3, 3.1, 3.6, 5.5, 5.6, 7.09 };
        amplitudes = { 1, 0.5, 0.33, 0.25, 0.75, 0.4, 0.2 };
        numbOfPartials = partialRatios.size();
        jassert(numbOfPartials == amplitudes.size());
        calculateLevel();*/

        /********************** backgroundVisualisation ********************************/
        backgroundVisualisation.reset(new BackgroundVisualisation(numbOfIntervals, octaves, notesPerOct, root, partialRatios, amplitudes));
        addAndMakeVisible(backgroundVisualisation.get());
        backgroundVisualisation->setInterceptsMouseClicks(false, true);

        /********************** Buttons ********************************/
        addAndMakeVisible(sawtoothButton);
        addAndMakeVisible(squareButton);
        sawtoothButton.setClickingTogglesState(true);
        squareButton.setClickingTogglesState(true);
        sawtoothButton.onClick = [this] { 
            if (sawtoothButton.getToggleState())
            {
                for (int i = 0; i < numbOfPartials; ++i)
                {
                    partialRatios[i] = (float)i + 1;
                    amplitudes[i] = 1 / ((float)i + 1);
                }
                calculateLevel();
                backgroundVisualisation->setPartialRatios(partialRatios);
                backgroundVisualisation->setAmplitudes(amplitudes);
            }
        };
        squareButton.onClick = [this] { 
            if (squareButton.getToggleState())
            {
                for (int i = 0; i < numbOfPartials; ++i)
                {
                    partialRatios[i] = 2 * (float)i + 1;
                    amplitudes[i] = 1 / ((float)i + 1);
                }
                calculateLevel();
                backgroundVisualisation->setPartialRatios(partialRatios);
                backgroundVisualisation->setAmplitudes(amplitudes);
            }
        };
        sawtoothButton.setRadioGroupId(1);
        squareButton.setRadioGroupId(1);
        sawtoothButton.triggerClick();

        /********************** ComboBoxes ********************************/
        addAndMakeVisible(selectOctaves);
        for (int i = 1; i <= 6; i++)
        {
            selectOctaves.addItem(juce::String(i), i);
        }
        selectOctaves.onChange = [this] {
            octaves = selectOctaves.getSelectedId();
            backgroundVisualisation->setOctaves(octaves);
            numbOfNotes = notesPerOct * octaves;
        };
        selectOctaves.setSelectedId(2);

        addAndMakeVisible(selectNotesPerOct);
        for (int i = 2; i <= 100; i++)
        {
            selectNotesPerOct.addItem(juce::String(i), i);
        }
        selectNotesPerOct.onChange = [this] {
            notesPerOct = selectNotesPerOct.getSelectedId();
            backgroundVisualisation->setNotesPerOctave(notesPerOct);
            numbOfNotes = notesPerOct * octaves;
        };
        selectNotesPerOct.setSelectedId(12);

        addAndMakeVisible(selectLowestOctave);
        for (int i = 1; i <= 6; i++)
        {
            selectLowestOctave.addItem(juce::String(i - 5), i);
        }
        selectLowestOctave.onChange = [this] {
            lowestOctave = selectLowestOctave.getSelectedId() - 5;
            root = tuningSlider.getValue() * std::pow(2.0, lowestOctave);
            updateFrequency();
            backgroundVisualisation->setRoot(root);
        };
        selectLowestOctave.setSelectedId(3);

        /********************** Labels ********************************/
        addAndMakeVisible(userInstructions);
        userInstructions.setText("Place up to " + juce::String(numbOfIntervals) + " notes with your mouse! Press 'blank key' to remove all notes!", juce::dontSendNotification);
        addAndMakeVisible(tuningSliderLabel);
        tuningSliderLabel.setText("Tuning", juce::dontSendNotification);
        tuningSliderLabel.attachToComponent(&tuningSlider, true);
        addAndMakeVisible(selectNotesPerOctLabel);
        selectNotesPerOctLabel.setText("Notes per Octave", juce::dontSendNotification);
        selectNotesPerOctLabel.attachToComponent(&selectNotesPerOct, false);
        addAndMakeVisible(selectOctavesLabel);
        selectOctavesLabel.setText("Number of Octaves", juce::dontSendNotification);
        selectOctavesLabel.attachToComponent(&selectOctaves, false);
        addAndMakeVisible(selectLowestOctaveLabel);
        selectLowestOctaveLabel.setText("Lowest Octave", juce::dontSendNotification);
        selectLowestOctaveLabel.attachToComponent(&selectLowestOctave, false);

        /********************** Sliders ********************************/
        addAndMakeVisible(tuningSlider);
        tuningSlider.setRange(350.0, 480.0);
        tuningSlider.setValue(440);
        tuningSlider.setTextValueSuffix(" Hz");
        tuningSlider.setNumDecimalPlacesToDisplay(1);
        tuningSlider.onValueChange = [this] {
            tuning = tuningSlider.getValue();
            root = tuning * std::pow(2.0, lowestOctave);
            updateFrequency();
            backgroundVisualisation->setRoot(root);
        };

        /********************** notes ********************************/
        for (int i = 0; i < numbOfIntervals; i++)
        {
            auto* newNote = new Note();
            addAndMakeVisible(newNote);
            notes.add(newNote);
            notes[i]->setInterceptsMouseClicks(true, true);
            notes[i]->reset();
        }
        for (int i = 0; i < numbOfIntervals; i++)
        {
            notes[i]->noteIsDragged = [i, this] {
                updateFrequency();
                notes[i]->setBounds(notes[i]->getPosition().getX() - 12.5, notes[i]->getPosition().getY() - 12.5, 25, 25);
            };
            notes[i]->noteIsClicked = [i, this] {
                notes[i]->reset();
                notes[i]->setBounds(notes[i]->getPosition().getX() - 12.5, notes[i]->getPosition().getY() - 12.5, 25, 25);
                updateFrequency();
            };
        }

        setSize (800, 600);
        setWantsKeyboardFocus(true);
        setAudioChannels (0, 2); // no inputs, two outputs
        startTimer (100);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////// END OF CONSTRUCTOR /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ~MultiTouchMainComponent() override { shutdownAudio(); }

    void paint(juce::Graphics& g) override {}

    void calculateLevel()
    {
        float sumOfAmplitudes = 0.0f;
        for (int i = 0; i < numbOfPartials; ++i)
        {
            sumOfAmplitudes += amplitudes[i];
        }
        level = 0.9f / (float)(numbOfIntervals * sumOfAmplitudes);
    }

    void resized() override
    {
        userInstructions.setBounds(10, 100, getWidth() - 20, 20);
        tuningSlider.setBounds(60, 70, getWidth() - 200, 20);
        selectNotesPerOct.setBounds(10, 30, 100, 20);
        selectOctaves.setBounds(140, 30, 100, 20);
        selectLowestOctave.setBounds(270, 30, 100, 20);
        sawtoothButton.setBounds(390, 10, 100, 20);
        squareButton.setBounds(390, 30, 100, 20);
        backgroundVisualisation->setBounds(0, 100, getWidth(), getHeight() - 100);
        for (auto i = 0; i < numbOfIntervals; i++)
        {
            notes[i]->setBounds(notes[i]->getPosition().getX() - 12.5, notes[i]->getPosition().getY() - 12.5, 25, 25);
        }
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        numbOfClicks++;
        if (numbOfClicks < numbOfIntervals) {
            notes[numbOfClicks]->updatePosition(event.getMouseDownPosition().toFloat());
            notes[numbOfClicks]->setBounds(notes[numbOfClicks]->getPosition().getX() - 12.5, notes[numbOfClicks]->getPosition().getY() - 12.5, 25, 25);
        }
        else {
            for (int i = 0; i < numbOfIntervals; ++i)
            {
                if (notes[i]->getPosition().getX() < 0) {
                    notes[i]->updatePosition(event.getMouseDownPosition().toFloat());
                    notes[i]->setBounds(notes[i]->getPosition().getX() - 12.5, notes[i]->getPosition().getY() - 12.5, 25, 25);
                    break;
                }
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

    void updateFrequency()
    {
        //root = tuningSlider.getValue();
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
        for (auto i = 0; i < numbOfIntervals * numbOfPartials; ++i) {
            auto* oscillator = new SineOscillator();
            oscillators.add(oscillator);
        } 
    }

    void releaseResources() override {}

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        auto* leftBuffer  = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
        auto* rightBuffer = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);

        bufferToFill.clearActiveBufferRegion();

        for (auto noteIndex = 0; noteIndex < numbOfIntervals; ++noteIndex)
        {
            for (int partial = 0; partial < numbOfPartials; ++partial)
            {
                auto* oscillator = oscillators.getUnchecked((noteIndex * numbOfPartials) + partial);
                oscillator->setFrequency(freq[noteIndex] * partialRatios[partial], currentSampleRate); //set frequency for each oscillator
                for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                {
                    auto levelSample = oscillator->getNextSample() * level * amplitudes[partial];
                    leftBuffer[sample] += levelSample;
                    rightBuffer[sample] += levelSample;
                }
            }
        }
    }

private:
    juce::Slider tuningSlider;
    juce::Label tuningSliderLabel;
    juce::Label userInstructions;
    juce::Label selectNotesPerOctLabel;
    juce::Label selectOctavesLabel;
    juce::Label selectLowestOctaveLabel;
    juce::ComboBox selectNotesPerOct;
    juce::ComboBox selectOctaves;
    juce::ComboBox selectLowestOctave;
    juce::ToggleButton sawtoothButton{ "Sawtooth" };
    juce::ToggleButton squareButton{ "Square" };

    std::unique_ptr<BackgroundVisualisation> backgroundVisualisation;
    juce::OwnedArray<SineOscillator> oscillators;
    juce::OwnedArray<Note> notes;

    int numbOfIntervals;
    int notesPerOct;
    int octaves;
    int numbOfNotes;
    int lowestOctave;
    float tuning;
    float root;
    int numbOfPartials;
    std::vector<float> freq;
    std::vector<float> intervals;
    std::vector<float> scaleSteps;
    std::vector<float> partialRatios;
    std::vector<float> amplitudes;
    double currentSampleRate;
    float level;
    int numbOfClicks;
        
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiTouchMainComponent)
};
