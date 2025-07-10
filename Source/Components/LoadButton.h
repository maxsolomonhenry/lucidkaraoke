#pragma once

#include <JuceHeader.h>

class LoadButton : public juce::Component
{
public:
    LoadButton();
    ~LoadButton() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    
    std::function<void(const juce::File&)> onFileSelected;
    
private:
    bool isHovered;
    std::unique_ptr<juce::FileChooser> fileChooser;
    
    void openFileChooser();
    void drawLoadIcon(juce::Graphics& g, juce::Rectangle<int> bounds);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoadButton)
};