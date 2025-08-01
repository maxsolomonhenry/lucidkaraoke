cmake_minimum_required(VERSION 3.22)

project(lucidkaraoke VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

add_subdirectory(../JUCE JUCE)

juce_add_plugin(lucidkaraoke
    COMPANY_NAME "PassByReference"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Juce
    PLUGIN_CODE Or41
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "LucidKaraoke"
    HARDENED_RUNTIME_ENABLED FALSE
    APP_SANDBOX_ENABLED FALSE
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "LucidKaraoke needs microphone access to record your vocals for karaoke sessions."
    PLIST_TO_MERGE "${CMAKE_SOURCE_DIR}/Info.plist")

juce_generate_juce_header(lucidkaraoke)

# Include Config directory for private configuration headers
target_include_directories(lucidkaraoke PRIVATE Source)

target_sources(lucidkaraoke
    PRIVATE
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/LookAndFeel/DarkTheme.cpp
        Source/LookAndFeel/DarkTheme.h
        Source/Components/WaveformDisplay.cpp
        Source/Components/WaveformDisplay.h
        Source/Components/TransportControls.cpp
        Source/Components/TransportControls.h
        Source/Components/LoadButton.cpp
        Source/Components/LoadButton.h
        Source/Components/SplitButton.cpp
        Source/Components/SplitButton.h
        Source/Components/ProgressBar.cpp
        Source/Components/ProgressBar.h
        Source/Components/RecordButton.cpp
        Source/Components/RecordButton.h
        Source/Components/SourceToggleButton.cpp
        Source/Components/SourceToggleButton.h
        Source/Audio/HttpStemProcessor.cpp
        Source/Audio/HttpStemProcessor.h
        Source/Audio/VocalMixer.cpp
        Source/Audio/VocalMixer.h
        Source/Audio/RVCProcessor.cpp
        Source/Audio/RVCProcessor.h)

# Debug/Release specific compile definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(lucidkaraoke
        PUBLIC
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0
            JUCE_STRICT_REFCOUNTEDPOINTER=1
            DEBUG=1)
else()
    target_compile_definitions(lucidkaraoke
        PUBLIC
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0
            JUCE_STRICT_REFCOUNTEDPOINTER=1)
endif()

target_link_libraries(lucidkaraoke
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)