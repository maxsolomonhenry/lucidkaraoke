/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LucidkaraokeAudioProcessor::LucidkaraokeAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
      state(Stopped)
{
    formatManager.registerBasicFormats();
    transportSource.addChangeListener(this);
    
    mixerSource.addInputSource(&transportSource, false);
}

LucidkaraokeAudioProcessor::~LucidkaraokeAudioProcessor()
{
    transportSource.removeChangeListener(this);
    mixerSource.removeAllInputs();
    transportSource.setSource(nullptr);
    readerSource.reset();
}

//==============================================================================
const juce::String LucidkaraokeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LucidkaraokeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LucidkaraokeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LucidkaraokeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LucidkaraokeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LucidkaraokeAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LucidkaraokeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LucidkaraokeAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LucidkaraokeAudioProcessor::getProgramName (int index)
{
    return {};
}

void LucidkaraokeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LucidkaraokeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
    mixerSource.prepareToPlay(samplesPerBlock, sampleRate);
}

void LucidkaraokeAudioProcessor::releaseResources()
{
    transportSource.releaseResources();
    mixerSource.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LucidkaraokeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void LucidkaraokeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (readerSource != nullptr)
    {
        juce::AudioSourceChannelInfo channelInfo(&buffer, 0, buffer.getNumSamples());
        mixerSource.getNextAudioBlock(channelInfo);
    }
    else
    {
        buffer.clear();
    }
}

//==============================================================================
bool LucidkaraokeAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LucidkaraokeAudioProcessor::createEditor()
{
    return new LucidkaraokeAudioProcessorEditor (*this);
}

//==============================================================================
void LucidkaraokeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void LucidkaraokeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
//==============================================================================
// Audio file handling implementation
void LucidkaraokeAudioProcessor::loadFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);
    
    if (reader != nullptr)
    {
        std::unique_ptr<juce::AudioFormatReaderSource> newSource(
            new juce::AudioFormatReaderSource(reader, true)
        );
        
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource = std::move(newSource);
        lastFileURL = juce::URL(file);
        
        changeState(Stopped);
    }
}

void LucidkaraokeAudioProcessor::play()
{
    if (readerSource != nullptr)
    {
        changeState(Playing);
    }
}

void LucidkaraokeAudioProcessor::pause()
{
    if (state == Playing)
    {
        changeState(Paused);
    }
}

void LucidkaraokeAudioProcessor::stop()
{
    if (state == Playing || state == Paused)
    {
        changeState(Stopped);
    }
}

void LucidkaraokeAudioProcessor::setPosition(double position)
{
    if (readerSource != nullptr)
    {
        auto lengthInSamples = readerSource->getTotalLength();
        auto samplePosition = static_cast<juce::int64>(position * lengthInSamples);
        transportSource.setPosition(samplePosition);
    }
}

double LucidkaraokeAudioProcessor::getPosition() const
{
    if (readerSource != nullptr)
    {
        auto lengthInSamples = readerSource->getTotalLength();
        if (lengthInSamples > 0)
            return transportSource.getCurrentPosition() / static_cast<double>(lengthInSamples);
    }
    return 0.0;
}

double LucidkaraokeAudioProcessor::getLength() const
{
    if (readerSource != nullptr)
        return readerSource->getTotalLength();
    return 0.0;
}

bool LucidkaraokeAudioProcessor::isPlaying() const
{
    return state == Playing;
}

bool LucidkaraokeAudioProcessor::isLoaded() const
{
    return readerSource != nullptr;
}

void LucidkaraokeAudioProcessor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        if (transportSource.isPlaying())
            changeState(Playing);
        else
            changeState(Stopped);
    }
}

void LucidkaraokeAudioProcessor::changeState(TransportState newState)
{
    if (state != newState)
    {
        state = newState;
        
        switch (state)
        {
            case Stopped:
                transportSource.setPosition(0.0);
                [[fallthrough]];
            case Paused:
                transportSource.stop();
                break;
                
            case Playing:
                transportSource.start();
                break;
        }
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LucidkaraokeAudioProcessor();
}
