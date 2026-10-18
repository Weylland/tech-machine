cmake_minimum_required(VERSION 3.22)

project(TechMachine VERSION 0.6.0)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs/JUCE)

juce_add_plugin(TechMachine
    COMPANY_NAME "Nicolas Samier"
    PRODUCT_NAME "Tech Machine"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Nsmr
    PLUGIN_CODE Tchm
    VST3_CATEGORIES Instrument Synth Drum
    COPY_PLUGIN_AFTER_BUILD FALSE
    FORMATS Standalone VST3)

target_sources(TechMachine PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/core/Examples.cpp
    Source/core/Help.cpp
    Source/core/Language.cpp
    Source/core/Translations.cpp
    Source/core/MachineDefs.cpp
    Source/core/Music.cpp
    Source/core/PatternLibrary.cpp
    Source/core/Presets.cpp
    Source/core/Styles.cpp
    Source/dsp/DrumVoices.cpp
    Source/machines/Machines.cpp
    Source/ui/MachinePanel.cpp
    Source/ui/Paint.cpp)

target_include_directories(TechMachine PRIVATE Source)

target_compile_options(TechMachine PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

target_compile_definitions(TechMachine PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

# ASIO drivers (Windows): the Steinberg ASIO SDK, used under its GPLv3 option, is fetched by build.bat into tools/asiosdk.
set(ASIO_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tools/asiosdk/common")
if(WIN32 AND EXISTS "${ASIO_SDK_DIR}/iasiodrv.h")
    target_compile_definitions(TechMachine PUBLIC JUCE_ASIO=1)
    target_include_directories(TechMachine PRIVATE "${ASIO_SDK_DIR}")
    message(STATUS "ASIO support enabled")
endif()

target_link_libraries(TechMachine
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_formats
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
