
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
#include "DissonanceCurve.h"
#include "Spectrum.h"

//==============================================================================
class MultiTouchMainComponent : public juce::AudioAppComponent,
                                public juce::MultiTimer
{
    const int maxNumberOfPartials = 20;
public:
    MultiTouchMainComponent()
    {
        /********************** Initialize Member Variables ********************************/
        numberOfIntervals = 10;  //#notes you can play simultaneously
        notesPerOct = 120; //BUG: initialize notesPerOct with maxNotesPerOct => otherwise Error
        octaves = 6;
        numberOfNotes = notesPerOct * octaves;
        lowestOctave = -1;
        tuning = 440;
        root = lowestOctave * tuning;
        numberOfPartials = 8;
        freq = std::vector<float>(numberOfIntervals, 0.0f);
        intervals = std::vector<float>(numberOfIntervals, 0.0f);
        maxPartialRatios = std::vector<float>(maxNumberOfPartials, 0.0f);
        maxAmplitudes = std::vector<float>(maxNumberOfPartials, 0.0f);
        currentSampleRate = 0.0f;
        spectrumId = 1;

        // choose other (inharmonic) spectrum here:
        /*partialRatios = { 1, 2.3, 3.1, 3.6, 5.5, 5.6, 7.09 };
        amplitudes = { 1, 0.5, 0.33, 0.25, 0.75, 0.4, 0.2 };
        numbOfPartials = partialRatios.size();
        jassert(numbOfPartials == amplitudes.size());
        calculateLevel();*/

        /********************** backgroundVisualisation ********************************/
        std::vector<float> partialRatios = {maxPartialRatios.begin(), maxPartialRatios.begin() + numberOfPartials};
        std::vector<float> amplitudes = {maxAmplitudes.begin(), maxAmplitudes.begin() + numberOfPartials };
        backgroundVisualisation.reset(new BackgroundVisualisation(numberOfIntervals, octaves, notesPerOct, root, partialRatios, amplitudes));
        addAndMakeVisible(backgroundVisualisation.get());
        backgroundVisualisation->setInterceptsMouseClicks(false, true);

        /********************** Buttons ********************************/
        addAndMakeVisible(sawtoothButton);
        addAndMakeVisible(squareButton);
        addAndMakeVisible(triangleButton);
        addAndMakeVisible(randomButton);
        addAndMakeVisible(optimizeSpectrumButton);
        sawtoothButton.setClickingTogglesState(true);
        squareButton.setClickingTogglesState(true);
        triangleButton.setClickingTogglesState(true);
        randomButton.setClickingTogglesState(true);
        optimizeSpectrumButton.setClickingTogglesState(true);
        sawtoothButton.onClick = [this] { 
            if (sawtoothButton.getToggleState())
            {
                spectrumId = 1;
                calculateSpectrum();
            }
        };
        squareButton.onClick = [this] { 
            if (squareButton.getToggleState())
            {
                spectrumId = 2;
                calculateSpectrum();
            }
        };
        triangleButton.onClick = [this] {
            if (triangleButton.getToggleState())
            {
                spectrumId = 3;
                calculateSpectrum();
            }
        };
        randomButton.onClick = [this] {
            spectrumId = 4;
            calculateSpectrum();
        };
        optimizeSpectrumButton.onClick = [this] {
            if (optimizeSpectrumButton.getToggleState())
            {
                spectrumId = 5;
                calculateSpectrum();
            }
        };
        sawtoothButton.setRadioGroupId(1);
        squareButton.setRadioGroupId(1);
        triangleButton.setRadioGroupId(1);
        randomButton.setRadioGroupId(1);
        optimizeSpectrumButton.setRadioGroupId(1);
        sawtoothButton.triggerClick(); //sawtooth = default

        /********************** ComboBoxes ********************************/
        addAndMakeVisible(selectOctaves);
        for (int i = 1; i <= 6; i++)
        {
            selectOctaves.addItem(juce::String(i), i);
        }
        selectOctaves.onChange = [this] {
            octaves = selectOctaves.getSelectedId();
            backgroundVisualisation->setOctaves(octaves);
            numberOfNotes = notesPerOct * octaves;
        };
        selectOctaves.setSelectedId(2);

        addAndMakeVisible(selectNotesPerOct);
        for (int i = 2; i <= 120; i++)
        {
            selectNotesPerOct.addItem(juce::String(i), i);
        }
        selectNotesPerOct.onChange = [this] {
            notesPerOct = selectNotesPerOct.getSelectedId();
            backgroundVisualisation->setNotesPerOctave(notesPerOct);
            dissonanceCurve->setNotesPerOctave(notesPerOct);
            numberOfNotes = notesPerOct * octaves;
            if (optimizeSpectrumButton.getToggleState())
            {
                spectrumId = 5;
                calculateSpectrum();
            }
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
            dissonanceCurve->setRoot(root);
        };
        selectLowestOctave.setSelectedId(3);

        addAndMakeVisible(selectNumbOfPartials);
        for (int i = 1; i <= 20; i++)
        {
            selectNumbOfPartials.addItem(juce::String(i), i);
        }
        selectNumbOfPartials.onChange = [this] {
            numberOfPartials = std::min(maxNumberOfPartials, selectNumbOfPartials.getSelectedId());
            std::vector<float> partialRatios = { maxPartialRatios.begin(), maxPartialRatios.begin() + numberOfPartials };
            std::vector<float> amplitudes = { maxAmplitudes.begin(), maxAmplitudes.begin() + numberOfPartials };
            backgroundVisualisation->setPartialRatios(partialRatios);
            backgroundVisualisation->setAmplitudes(amplitudes);
            dissonanceCurve->setPartialRatios(partialRatios);
            dissonanceCurve->setAmplitudes(amplitudes);
            calculateSpectrum();
        };
        selectNumbOfPartials.setSelectedId(8);

        /********************** Labels ********************************/
        addAndMakeVisible(userInstructions);
        userInstructions.setText("Play up to " + juce::String(numberOfIntervals) + " notes with your fingers!", juce::dontSendNotification);
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
        addAndMakeVisible(selectNumbOfPartialsLabel);
        selectNumbOfPartialsLabel.setText("#Partials for Calculation", juce::dontSendNotification);
        selectNumbOfPartialsLabel.attachToComponent(&selectNumbOfPartials, false);
        addAndMakeVisible(currentDissonanceLabel);

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
            dissonanceCurve->setRoot(root);
        };
        
        /********************** dissonanceCurve ********************************/
        dissonanceCurve.reset(new DissonanceCurve(notesPerOct, root, partialRatios, amplitudes));
        addAndMakeVisible(dissonanceCurve.get());

        /********************** spectrum ********************************/
        spectrum.reset(new Spectrum(maxPartialRatios, maxAmplitudes));
        addAndMakeVisible(spectrum.get());

        /********************** notes ********************************/
        for (int i = 0; i < numberOfIntervals; i++)
        {
            auto* newNote = new Note();
            addAndMakeVisible(newNote);
            notes.add(newNote);
            notes[i]->setInterceptsMouseClicks(false, false);
            notes[i]->reset();
        }
     
        setSize(1200, 800);
        setWantsKeyboardFocus(true);
        setAudioChannels (0, 2); // no inputs, two outputs
        startTimer(1, 60);
        startTimer(2, 1000);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////// END OF CONSTRUCTOR /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ~MultiTouchMainComponent() override { shutdownAudio(); }

    void paint(juce::Graphics& g) override {}

    void calculateLevel()
    {
        float sumOfAmplitudes = 0.0f;
        for (int i = 0; i < maxNumberOfPartials; ++i)
        {
            sumOfAmplitudes += maxAmplitudes[i];
        }
        level = 0.9f / (float)(numberOfIntervals * sumOfAmplitudes);
    }

    void calculateSpectrum()
    {
        if (spectrumId == 1) { //sawtooth
            for (int i = 0; i < maxNumberOfPartials; ++i)
            {
                maxPartialRatios[i] = (float)i + 1;
                maxAmplitudes[i] = 1 / ((float)i + 1);
            }
        }
        else if (spectrumId == 2) { //square
            for (int i = 0; i < maxNumberOfPartials; ++i)
            {
                maxPartialRatios[i] = 2 * (float)i + 1;
                maxAmplitudes[i] = 1 / ((float)i + 1);
            }
        }
        else if (spectrumId == 3) { //triangle
            for (int i = 0; i < maxNumberOfPartials; ++i)
            {
                maxPartialRatios[i] = 2 * (float)i + 1;
                maxAmplitudes[i] = 1 / (float)std::pow(i + 1, 2);
            }
        }
        else if (spectrumId == 4) { //random
            for (int i = 0; i < maxNumberOfPartials; ++i)
            {
                maxPartialRatios[i] = juce::Random::getSystemRandom().nextFloat() * maxNumberOfPartials;
                maxAmplitudes[i] = juce::Random::getSystemRandom().nextFloat();
            }
        }
        else if (spectrumId == 5) { //optimize Spectrum for Equal Temperaments (Sethares p. 247)
            const float s = std::pow(2, 1 / (float)notesPerOct);
            for (int i = 0; i < maxNumberOfPartials; ++i)
            {
                float exponent = std::round(std::log10(i + 1) / std::log10(s)); //s^x = z  =>  x = log(z)/log(s)
                //Logger::outputDebugString(String(exponent));
                maxPartialRatios[i] = std::pow(s, exponent);
                //Logger::outputDebugString(String(maxPartialRatios[i]));
                maxAmplitudes[i] = 1 / ((float)i + 1);
            }
        }
        std::vector<float> partialRatios = { maxPartialRatios.begin(), maxPartialRatios.begin() + numberOfPartials };
        std::vector<float> amplitudes = { maxAmplitudes.begin(), maxAmplitudes.begin() + numberOfPartials };
        backgroundVisualisation->setPartialRatios(partialRatios);
        backgroundVisualisation->setAmplitudes(amplitudes);
        dissonanceCurve->setPartialRatios(partialRatios);
        dissonanceCurve->setAmplitudes(amplitudes);
        spectrum->setPartialRatios(maxPartialRatios);
        spectrum->setAmplitudes(maxAmplitudes);
        spectrum->repaint();
        calculateLevel();
    }

    void resized() override
    {
        userInstructions.setBounds(10, 160, getWidth() - 20, 30);
        tuningSlider.setBounds(60, 80, 430, 30);
        selectNotesPerOct.setBounds(10, 30, 120, 30);
        selectOctaves.setBounds(140, 30, 120, 30);
        selectLowestOctave.setBounds(270, 30, 120, 30);
        selectNumbOfPartials.setBounds(400, 30, 120, 30);
        sawtoothButton.setBounds(530, 10, 70, 30);
        squareButton.setBounds(530, 50, 70, 30);
        triangleButton.setBounds(615, 10, 70, 30);
        randomButton.setBounds(615, 50, 70, 30);
        optimizeSpectrumButton.setBounds(530, 90, 160, 30);
        backgroundVisualisation->setBounds(0, 160, getWidth(), getHeight() - 160);
        dissonanceCurve->setBounds(700, 10, 280, 140);
        spectrum->setBounds(990, 10, 280, 140);
        currentDissonanceLabel.setBounds(10, 120, 170, 30);
        for (auto i = 0; i < numberOfIntervals; i++)
        {
            notes[i]->setBounds(notes[i]->getPosition().getX() - 12.5, notes[i]->getPosition().getY() - 12.5, 25, 25);
        }
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        //Logger::outputDebugString(juce::String(event.source.getIndex()) + "is clicked");
        notes[event.source.getIndex()]->updatePosition(event.position);
        notes[event.source.getIndex()]->setBounds(notes[event.source.getIndex()]->getPosition().getX() - 12.5, notes[event.source.getIndex()]->getPosition().getY() - 12.5, 25, 25);
        updateFrequency();
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        notes[event.source.getIndex()]->updatePosition(event.position);
        updateFrequency();
        notes[event.source.getIndex()]->setBounds(notes[event.source.getIndex()]->getPosition().getX() - 12.5, notes[event.source.getIndex()]->getPosition().getY() - 12.5, 25, 25);
    }

    void mouseUp(const juce::MouseEvent& event) override
    {
        //Logger::outputDebugString(juce::String(event.source.getIndex()) + "is released");
        notes[event.source.getIndex()]->reset();
        updateFrequency();
        notes[event.source.getIndex()]->setBounds(notes[event.source.getIndex()]->getPosition().getX() - 12.5, notes[event.source.getIndex()]->getPosition().getY() - 12.5, 25, 25);
    }

    void timerCallback(int timerID) override
    {
        if (timerID == 1) {
            backgroundVisualisation->update();
            float currentDissonance = backgroundVisualisation->getCurrentDissonance();
            currentDissonanceLabel.setText("Current Dissonance: " + juce::String(currentDissonance, 4), juce::dontSendNotification);
        }
        else {
            dissonanceCurve->update();
        }
    }

    void updateFrequency()
    {
        for (int i = 0; i < numberOfIntervals; i++) {
            if (notes[i]->getPosition().getX() < 0) {
                intervals[i] = -1.0f;
                freq[i] = 0.0f;
            }
            else {
                float scaleStep = std::floor(numberOfNotes * (notes[i]->getPosition().getX()) / getWidth());
                intervals[i] = std::pow(2, scaleStep / notesPerOct);
                freq[i] = intervals[i] * root;
            }
        }
        backgroundVisualisation->setIntervals(intervals);
    }

    void prepareToPlay (int, double sampleRate) override
    {
        currentSampleRate = sampleRate;
        for (auto i = 0; i < numberOfIntervals * maxNumberOfPartials; ++i) {
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

        for (auto noteIndex = 0; noteIndex < numberOfIntervals; ++noteIndex)
        {
            for (int partial = 0; partial < maxNumberOfPartials; ++partial) //play all 20 partials
            {
                auto* oscillator = oscillators.getUnchecked((noteIndex * maxNumberOfPartials) + partial);
                oscillator->setFrequency(freq[noteIndex] * maxPartialRatios[partial], currentSampleRate);
                for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                {
                    auto levelSample = oscillator->getNextSample() * level * maxAmplitudes[partial];
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
    juce::Label selectNumbOfPartialsLabel;
    juce::Label currentDissonanceLabel;
    juce::ComboBox selectNotesPerOct;
    juce::ComboBox selectOctaves;
    juce::ComboBox selectLowestOctave;
    juce::ComboBox selectNumbOfPartials;
    juce::TextButton sawtoothButton{ "Sawtooth" };
    juce::TextButton squareButton{ "Square" };
    juce::TextButton triangleButton{ "Triangle" };
    juce::TextButton randomButton{ "Random" };
    juce::TextButton optimizeSpectrumButton{ "Optimize Spectrum" };

    std::unique_ptr<BackgroundVisualisation> backgroundVisualisation;
    std::unique_ptr<DissonanceCurve> dissonanceCurve;
    std::unique_ptr<Spectrum> spectrum;
    juce::OwnedArray<SineOscillator> oscillators;
    juce::OwnedArray<Note> notes;

    int numberOfIntervals;
    int notesPerOct;
    int octaves;
    int numberOfNotes;
    int lowestOctave;
    float tuning;
    float root;
    int numberOfPartials;
    std::vector<float> freq;
    std::vector<float> intervals;
    std::vector<float> maxPartialRatios;
    std::vector<float> maxAmplitudes;
    double currentSampleRate;
    float level;
    int spectrumId;
        
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiTouchMainComponent)
};
