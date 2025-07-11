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
    backgroundThread.startThread();

    // Initialize recording device manager
    recordingDeviceManager.initialiseWithDefaultDevices(1, 0); // 1 input, 0 outputs

    juce::AudioDeviceManager::AudioDeviceSetup setup;
    recordingDeviceManager.getAudioDeviceSetup(setup);

    if (setup.inputDeviceName.isNotEmpty())
    {
        setup.inputChannels.setBit(0);
        recordingDeviceManager.setAudioDeviceSetup(setup, true);
    }

    recordingCallback = std::make_unique<RecordingCallback>(*this);
}

LucidkaraokeAudioProcessor::~LucidkaraokeAudioProcessor()
{
    stopRecording();
    recordingDeviceManager.removeAudioCallback(recordingCallback.get());
    backgroundThread.stopThread(5000);
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
    if (readerSource != nullptr && (state == Stopped || state == Paused))
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
    else if (state == Paused)
    {
        changeState(Playing);
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
        auto lengthInSeconds = readerSource->getTotalLength() / readerSource->getAudioFormatReader()->sampleRate;
        auto timePosition = position * lengthInSeconds;
        transportSource.setPosition(timePosition);
    }
}

double LucidkaraokeAudioProcessor::getPosition() const
{
    if (readerSource != nullptr)
    {
        auto lengthInSeconds = readerSource->getTotalLength() / readerSource->getAudioFormatReader()->sampleRate;
        if (lengthInSeconds > 0)
            return transportSource.getCurrentPosition() / lengthInSeconds;
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

bool LucidkaraokeAudioProcessor::isPaused() const
{
    return state == Paused;
}

bool LucidkaraokeAudioProcessor::isLoaded() const
{
    return readerSource != nullptr;
}

void LucidkaraokeAudioProcessor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    // We manually control the state through our transport methods
    // so we don't need to automatically change state based on transport source
    juce::ignoreUnused(source);
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
                transportSource.stop();
                break;
                
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
// Recording functionality
void LucidkaraokeAudioProcessor::startRecording()
{
    stopRecording(); // Stop any existing recording

    // Create a unique filename in temp directory
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
    auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
    recordingFile = tempDir.getChildFile("LucidKaraoke_Recording_" + timestamp + ".wav");

    // Create WAV writer for mono recording
    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::FileOutputStream> fileStream(recordingFile.createOutputStream());

    if (fileStream != nullptr)
    {
        auto* currentDevice = recordingDeviceManager.getCurrentAudioDevice();
        auto sampleRate = currentDevice ? currentDevice->getCurrentSampleRate() : 44100.0;
        auto bitDepth = 16;

        if (auto writer = wavFormat.createWriterFor(fileStream.release(),
                                                    sampleRate,
                                                    1, // Mono
                                                    bitDepth,
                                                    {},
                                                    0))
        {
            // Passes responsibility for deleting the stream to the writer object
            threadedWriter.reset(new juce::AudioFormatWriter::ThreadedWriter(writer, backgroundThread, 32768));

            // Start the audio device callback *before* setting the flag
            recordingDeviceManager.addAudioCallback(recordingCallback.get());

            // Now, swap over our active writer pointer so that the audio callback will start using it..
            const juce::ScopedLock sl(writerLock);
            activeWriter = threadedWriter.get();
        }
    }
}

void LucidkaraokeAudioProcessor::stopRecording()
{
    // First, clear this pointer to stop the audio callback from using our writer object..
    { 
        const juce::ScopedLock sl(writerLock);
        activeWriter = nullptr;
    }

    // Now we can delete the writer object. It's done in this order because the deletion could
    // take a little time while remaining data gets flushed to disk, so it's best to avoid blocking
    // the audio callback while this happens.
    threadedWriter.reset();

    recordingDeviceManager.removeAudioCallback(recordingCallback.get());
}

bool LucidkaraokeAudioProcessor::isRecording() const
{
    return activeWriter.load() != nullptr;
}

//==============================================================================
// RecordingCallback implementation
void LucidkaraokeAudioProcessor::RecordingCallback::audioDeviceIOCallbackWithContext(
    const float* const* inputChannelData,
    int numInputChannels,
    float* const* outputChannelData,
    int numOutputChannels,
    int numSamples, const juce::AudioIODeviceCallbackContext& context)
{
    juce::ignoreUnused(context);

    const juce::ScopedLock sl(owner.writerLock);

    if (owner.activeWriter.load() != nullptr && numInputChannels > 0 && inputChannelData[0] != nullptr)
    {
        owner.activeWriter.load()->write(inputChannelData, numSamples);
    }

    // Clear output buffers (we don't want to output anything)
    for (int i = 0; i < numOutputChannels; ++i)
    {
        if (outputChannelData[i] != nullptr)
            juce::FloatVectorOperations::clear(outputChannelData[i], numSamples);
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LucidkaraokeAudioProcessor();
}
