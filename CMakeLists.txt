cmake_minimum_required(VERSION 3.22)
project(CreativeEcho VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(CreativeEcho
    COMPANY_NAME "c25l_"
    PLUGIN_MANUFACTURER_CODE Cslx
    PLUGIN_CODE CrEc
    FORMATS VST3
    PRODUCT_NAME "Creative Echo"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE)

target_sources(CreativeEcho PRIVATE Source/PluginProcessor.cpp Source/PluginProcessor.h)
target_compile_definitions(CreativeEcho PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(CreativeEcho PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
