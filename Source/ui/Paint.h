#pragma once

#include "Skin.h"
#include <juce_gui_basics/juce_gui_basics.h>

// Drawing helpers for the hardware look: materials, screws, LEDs, keys, knobs, faders, displays.
namespace techno::paint
{
using juce::Graphics;
using Rect = juce::Rectangle<float>;
using Point = juce::Point<float>;

juce::Font font (float size, bool bold = true);
juce::Font condensed (float size);
juce::Font display (float size);

const juce::Image& noise();

void plate (Graphics&, Rect, juce::Colour top, juce::Colour bottom, float radius, bool brushed);
void band (Graphics&, Rect, const Skin&, float radius);
void stripes (Graphics&, Rect);
void screw (Graphics&, Point centre, float radius, float angle = 0.6f);
void wood (Graphics&, Rect, int seed);
void rackEar (Graphics&, Rect);
void recess (Graphics&, Rect, float radius);
void led (Graphics&, Point centre, float radius, juce::Colour, bool on);
void pad (Graphics&, Rect, juce::Colour cap, juce::Colour lit, bool on, bool playhead);
void knob (Graphics&, Rect, float pos, float startAngle, float endAngle, const Skin&, bool bipolar);
void fader (Graphics&, Rect, float thumbY, float minY, float maxY, const Skin&);
void printedBox (Graphics&, Rect, const juce::String& title, const Skin&);
void sevenSegment (Graphics&, const juce::String& text, Rect, juce::Colour on);
void vuMeter (Graphics&, Rect, float needleDb);
void icon (Graphics&, const juce::String& name, Rect, juce::Colour);
} // namespace techno::paint
