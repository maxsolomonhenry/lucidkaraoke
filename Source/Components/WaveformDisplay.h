#pragma once

#include <JuceHeader.h>

class WaveformDisplay : public juce::Component, 
                       public juce::ChangeListener,
                       public juce::Timer
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    
    void loadURL(const juce::URL& url);
    void loadFromFile(const juce::File& file);
    void setPositionRelative(double position);
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void timerCallback() override;
    
    enum class DisplayMode
    {
        Normal,
        MixedFile
    };
    
    void setDisplayMode(DisplayMode mode);
    
    std::function<void(double)> onPositionChanged;
    
private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail audioThumbnail;
    bool fileLoaded;
    double position;
    bool isDragging;
    DisplayMode displayMode;
    
    void paintIfNoFileLoaded(juce::Graphics& g);
    void paintIfFileLoaded(juce::Graphics& g);
    void paintPlayhead(juce::Graphics& g);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};