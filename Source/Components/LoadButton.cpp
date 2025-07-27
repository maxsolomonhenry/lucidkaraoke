#include "LoadButton.h"

LoadButton::LoadButton()
    : isHovered(false)
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

LoadButton::~LoadButton()
{
}

void LoadButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto cornerSize = 6.0f;
    
    juce::Colour buttonColor = juce::Colour(0xff2d2d2d);
    
    if (isHovered)
        buttonColor = juce::Colour(0xff404040);
    
    g.setColour(buttonColor);
    g.fillRoundedRectangle(bounds.toFloat(), cornerSize);
    
    g.setColour(juce::Colour(0xff4dabf7));
    g.drawRoundedRectangle(bounds.toFloat(), cornerSize, 1.5f);
    
    // Simpler text-only design for compact header button
    g.setColour(juce::Colour(0xffe9ecef));
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("LOAD", bounds, juce::Justification::centred);
}

void LoadButton::resized()
{
}

void LoadButton::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isLeftButtonDown())
    {
        openFileChooser();
    }
}

void LoadButton::mouseEnter(const juce::MouseEvent& event)
{
    isHovered = true;
    repaint();
}

void LoadButton::mouseExit(const juce::MouseEvent& event)
{
    isHovered = false;
    repaint();
}

void LoadButton::openFileChooser()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select an audio file to load...",
        juce::File::getSpecialLocation(juce::File::userMusicDirectory),
        "*.wav;*.mp3;*.flac;*.aiff;*.ogg;*.m4a"
    );
    
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    
    fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        if (fc.getResults().size() > 0)
        {
            auto file = fc.getResult();
            if (onFileSelected)
                onFileSelected(file);
        }
    });
}

void LoadButton::drawLoadIcon(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto iconBounds = bounds.reduced(bounds.getWidth() / 3, bounds.getHeight() / 2);
    iconBounds = iconBounds.withHeight(iconBounds.getHeight() / 2);
    
    g.setColour(juce::Colour(0xff4dabf7));
    
    juce::Path folderPath;
    folderPath.addRoundedRectangle(iconBounds.toFloat(), 2.0f);
    
    auto tabBounds = iconBounds.removeFromTop(iconBounds.getHeight() / 3);
    tabBounds = tabBounds.removeFromLeft(tabBounds.getWidth() / 2);
    
    folderPath.addRoundedRectangle(tabBounds.toFloat(), 2.0f);
    
    g.fillPath(folderPath);
    
    auto arrowBounds = iconBounds.reduced(iconBounds.getWidth() / 4);
    juce::Path arrowPath;
    arrowPath.addArrow(juce::Line<float>(arrowBounds.getX(), arrowBounds.getCentreY(),
                                        arrowBounds.getRight(), arrowBounds.getCentreY()),
                      2.0f, arrowBounds.getHeight() / 3, arrowBounds.getHeight() / 4);
    
    g.fillPath(arrowPath);
}