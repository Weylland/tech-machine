#include "Paint.h"

#include <BinaryData.h>
#include <cstring>

namespace techno::paint
{
using juce::Colour;
namespace Colours = juce::Colours;

namespace
{

// Bahnschrift comes with Windows. Elsewhere (macOS, Linux) the embedded DejaVu Sans Condensed takes its place,
// slightly smaller because it is wider. TECHMACHINE_EMBEDDED_FONT=1 forces it, to check the layout on Windows.
juce::Font bahnschrift (const char* style, float size)
{
    static const bool available = juce::Font::findAllTypefaceNames().contains ("Bahnschrift")
                                  && juce::SystemStats::getEnvironmentVariable ("TECHMACHINE_EMBEDDED_FONT", {}).isEmpty();
    if (available) return juce::FontOptions ("Bahnschrift", style, size);

    static const auto regular = juce::Typeface::createSystemTypefaceFor (BinaryData::DejaVuSansCondensed_ttf,
                                                                         (size_t) BinaryData::DejaVuSansCondensed_ttfSize);
    static const auto bold = juce::Typeface::createSystemTypefaceFor (BinaryData::DejaVuSansCondensedBold_ttf,
                                                                      (size_t) BinaryData::DejaVuSansCondensedBold_ttfSize);
    const bool isRegular = std::strcmp (style, "Regular") == 0;
    return juce::Font (juce::FontOptions (isRegular ? regular : bold).withHeight (size * 0.9f));
}

void knurl (Graphics& g, Point c, float r, int count, Colour colour)
{
    g.setColour (colour);
    for (int i = 0; i < count; ++i)
    {
        const float a = juce::MathConstants<float>::twoPi * (float) i / (float) count;
        g.drawLine ({ c.getPointOnCircumference (r * 0.84f, a), c.getPointOnCircumference (r * 0.99f, a) }, 1.0f);
    }
}

void disc (Graphics& g, Point c, float r, Colour light, Colour dark)
{
    juce::ColourGradient grad (light, c.x - r * 0.6f, c.y - r * 0.8f, dark, c.x + r * 0.5f, c.y + r, true);
    g.setGradientFill (grad);
    g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);
}

void pointer (Graphics& g, Point c, float from, float to, float angle, float thickness, Colour colour)
{
    g.setColour (colour);
    g.drawLine ({ c.getPointOnCircumference (from, angle), c.getPointOnCircumference (to, angle) }, thickness);
}
} // namespace

juce::Font font (float size, bool bold) { return bahnschrift (bold ? "SemiBold" : "Regular", size); }
juce::Font condensed (float size) { return bahnschrift ("SemiBold", size * 0.94f); }
juce::Font display (float size) { return bahnschrift ("Bold", size); }

const juce::Image& noise()
{
    static const juce::Image image = []
    {
        juce::Image img (juce::Image::ARGB, 128, 128, true);
        juce::Random r (1983);
        for (int y = 0; y < img.getHeight(); ++y)
            for (int x = 0; x < img.getWidth(); ++x)
            {
                const bool light = r.nextBool();
                img.setPixelAt (x, y, (light ? Colours::white : Colours::black).withAlpha ((float) r.nextInt (14) / 255.0f));
            }
        return img;
    }();
    return image;
}

void plate (Graphics& g, Rect r, Colour top, Colour bottom, float radius, bool brushed)
{
    g.setColour (Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (r.translated (0.0f, 3.0f), radius);

    g.setGradientFill (juce::ColourGradient (top, r.getX(), r.getY(), bottom, r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, radius);

    if (brushed)
    {
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (r.toNearestInt());
        juce::Random rnd (7);
        for (float y = r.getY(); y < r.getBottom(); y += 1.5f)
        {
            g.setColour ((rnd.nextBool() ? Colours::white : Colours::black).withAlpha (0.015f + rnd.nextFloat() * 0.03f));
            g.drawHorizontalLine ((int) y, r.getX(), r.getRight());
        }
    }

    g.setTiledImageFill (noise(), 0, 0, 1.0f);
    g.fillRoundedRectangle (r, radius);

    g.setColour (Colours::white.withAlpha (0.22f));
    g.drawHorizontalLine ((int) r.getY() + 1, r.getX() + radius, r.getRight() - radius);
    g.setColour (Colours::black.withAlpha (0.65f));
    g.drawRoundedRectangle (r.reduced (0.5f), radius, 1.0f);
}

void band (Graphics& g, Rect r, const Skin& skin, float radius)
{
    juce::Path p;
    p.addRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(), radius, radius, true, true, false, false);
    g.setGradientFill (juce::ColourGradient (skin.band.brighter (0.12f), r.getX(), r.getY(), skin.band.darker (0.25f), r.getX(), r.getBottom(), false));
    g.fillPath (p);
    g.setTiledImageFill (noise(), 0, 0, 1.0f);
    g.fillPath (p);
    g.setColour (skin.accent);
    g.fillRect (r.getX(), r.getBottom() - 2.0f, r.getWidth(), 2.0f);
    g.setColour (Colours::black.withAlpha (0.5f));
    g.fillRect (r.getX(), r.getBottom(), r.getWidth(), 1.0f);
}

void stripes (Graphics& g, Rect r)
{
    const Colour colours[] { Colour (0xff3e8ede), Colour (0xffe8432e), Colour (0xfff08a24) };
    const float h = r.getHeight() / 3.0f;
    for (int i = 0; i < 3; ++i)
    {
        g.setColour (colours[i]);
        g.fillRect (r.getX(), r.getY() + h * (float) i, r.getWidth(), h - 1.0f);
    }
}

void screw (Graphics& g, Point c, float r, float angle)
{
    g.setColour (Colours::black.withAlpha (0.55f));
    g.fillEllipse (c.x - r - 0.5f, c.y - r + 0.8f, (r + 0.5f) * 2.0f, (r + 0.5f) * 2.0f);
    disc (g, c, r, Colour (0xffe6e6e6), Colour (0xff6d6d6d));
    g.setColour (Colours::black.withAlpha (0.4f));
    g.drawEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f, 0.8f);
    const auto d = Point (std::cos (angle), std::sin (angle)) * (r * 0.75f);
    g.setColour (Colour (0xff3a3a3a));
    g.drawLine ({ c - d, c + d }, juce::jmax (1.2f, r * 0.32f));
    g.setColour (Colours::white.withAlpha (0.5f));
    g.drawLine ({ c - d + Point (0.0f, 0.8f), c + d + Point (0.0f, 0.8f) }, 0.6f);
}

void wood (Graphics& g, Rect r, int seed)
{
    g.setGradientFill (juce::ColourGradient (Colour (0xff6b3d22), r.getX(), r.getY(), Colour (0xff3d2213), r.getRight(), r.getY(), false));
    g.fillRect (r);

    juce::Random rnd (seed);
    juce::Graphics::ScopedSaveState state (g);
    g.reduceClipRegion (r.toNearestInt());
    for (int i = 0; i < 18; ++i)
    {
        juce::Path grain;
        float x = r.getX() + rnd.nextFloat() * r.getWidth();
        grain.startNewSubPath (x, r.getY());
        for (float y = r.getY(); y < r.getBottom(); y += 12.0f)
        {
            x += (rnd.nextFloat() - 0.5f) * 2.2f;
            grain.lineTo (x, y);
        }
        g.setColour ((rnd.nextBool() ? Colour (0xff2a170c) : Colour (0xff8a5532)).withAlpha (0.25f + rnd.nextFloat() * 0.25f));
        g.strokePath (grain, juce::PathStrokeType (0.6f + rnd.nextFloat() * 1.2f));
    }
    g.setTiledImageFill (noise(), 0, 0, 1.0f);
    g.fillRect (r);
    g.setColour (Colours::white.withAlpha (0.12f));
    g.fillRect (r.getX(), r.getY(), 1.0f, r.getHeight());
    g.setColour (Colours::black.withAlpha (0.45f));
    g.fillRect (r.getRight() - 1.0f, r.getY(), 1.0f, r.getHeight());
}

void rackEar (Graphics& g, Rect r)
{
    g.setGradientFill (juce::ColourGradient (Colour (0xff4a4b50), r.getX(), r.getY(), Colour (0xff2c2d31), r.getRight(), r.getY(), false));
    g.fillRect (r);
    g.setTiledImageFill (noise(), 0, 0, 1.0f);
    g.fillRect (r);
    g.setColour (Colours::black.withAlpha (0.5f));
    g.fillRect (r.getRight() - 1.0f, r.getY(), 1.0f, r.getHeight());
    g.setColour (Colours::white.withAlpha (0.1f));
    g.fillRect (r.getX(), r.getY(), 1.0f, r.getHeight());

    const float cx = r.getCentreX();
    screw (g, { cx, r.getY() + 16.0f }, 4.5f, 0.3f);
    screw (g, { cx, r.getBottom() - 16.0f }, 4.5f, 1.2f);
}

void recess (Graphics& g, Rect r, float radius)
{
    g.setColour (Colours::white.withAlpha (0.1f));
    g.fillRoundedRectangle (r.translated (0.0f, 1.0f), radius);
    g.setColour (Colour (0xff09090b));
    g.fillRoundedRectangle (r, radius);
    g.setGradientFill (juce::ColourGradient (Colours::black.withAlpha (0.8f), r.getX(), r.getY(), Colours::transparentBlack, r.getX(), r.getY() + 6.0f, false));
    g.fillRoundedRectangle (r, radius);
}

void led (Graphics& g, Point c, float r, Colour colour, bool on)
{
    if (on)
    {
        g.setGradientFill (juce::ColourGradient (colour.withAlpha (0.55f), c.x, c.y, colour.withAlpha (0.0f), c.x + r * 3.0f, c.y, true));
        g.fillEllipse (c.x - r * 3.0f, c.y - r * 3.0f, r * 6.0f, r * 6.0f);
        disc (g, c, r, colour.brighter (0.8f), colour);
    }
    else
    {
        disc (g, c, r, colour.withMultipliedBrightness (0.38f), colour.withMultipliedBrightness (0.15f));
    }
    g.setColour (Colours::black.withAlpha (0.6f));
    g.drawEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f, 0.8f);
    g.setColour (Colours::white.withAlpha (on ? 0.8f : 0.25f));
    g.fillEllipse (c.x - r * 0.45f, c.y - r * 0.55f, r * 0.5f, r * 0.4f);
}

void pad (Graphics& g, Rect r, Colour cap, Colour lit, bool on, bool playhead)
{
    const auto body = r.reduced (1.5f);
    const float radius = juce::jmin (4.0f, body.getHeight() * 0.3f);

    if (on)
    {
        g.setColour (lit.withAlpha (0.3f));
        g.fillRoundedRectangle (body.expanded (2.0f), radius + 2.0f);
    }

    g.setColour (Colours::black.withAlpha (0.55f));
    g.fillRoundedRectangle (body.translated (0.0f, 1.5f), radius);

    const Colour base = on ? lit : cap;
    g.setGradientFill (juce::ColourGradient (base.brighter (on ? 0.35f : 0.15f), body.getX(), body.getY(),
                                             base.darker (on ? 0.1f : 0.25f), body.getX(), body.getBottom(), false));
    g.fillRoundedRectangle (body, radius);

    g.setColour (Colours::white.withAlpha (on ? 0.55f : 0.28f));
    g.drawHorizontalLine ((int) body.getY() + 1, body.getX() + radius, body.getRight() - radius);
    g.setColour (Colours::black.withAlpha (0.6f));
    g.drawRoundedRectangle (body, radius, 0.8f);

    if (playhead)
    {
        g.setColour (Colours::white.withAlpha (on ? 0.45f : 0.16f));
        g.fillRoundedRectangle (body, radius);
        g.setColour (Colours::white.withAlpha (0.7f));
        g.drawRoundedRectangle (body.expanded (1.0f), radius + 1.0f, 1.2f);
    }
}

void knob (Graphics& g, Rect area, float pos, float start, float end, const Skin& skin, bool bipolar)
{
    const float size = juce::jmin (area.getWidth(), area.getHeight());
    const auto c = area.getCentre();
    const float ring = size * 0.5f - 1.5f;
    const float r = ring - juce::jmax (5.0f, size * 0.13f);
    const float angle = start + pos * (end - start);

    for (int i = 0; i <= 10; ++i)
    {
        const float a = start + (end - start) * (float) i / 10.0f;
        g.setColour (skin.textDim.withAlpha (i % 5 == 0 ? 0.9f : 0.55f));
        g.drawLine ({ c.getPointOnCircumference (ring - (i % 5 == 0 ? 4.0f : 2.5f), a), c.getPointOnCircumference (ring, a) }, 1.0f);
    }

    const float from = bipolar ? (start + end) * 0.5f : start;
    juce::Path arc;
    arc.addCentredArc (c.x, c.y, ring - 1.0f, ring - 1.0f, 0.0f, juce::jmin (from, angle), juce::jmax (from, angle), true);
    g.setColour (skin.accent);
    g.strokePath (arc, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour (Colours::black.withAlpha (0.5f));
    g.fillEllipse (c.x - r * 1.04f, c.y - r * 0.92f, r * 2.08f, r * 2.08f);

    const float thick = juce::jmax (1.8f, r * 0.13f);
    switch (skin.knob)
    {
        case KnobStyle::Classic:
            disc (g, c, r, skin.knobCap.brighter (0.35f), skin.knobCap.darker (0.6f));
            disc (g, c, r * 0.72f, skin.knobCap.brighter (0.55f), skin.knobCap.darker (0.15f));
            pointer (g, c, r * 0.2f, r * 0.93f, angle, thick, Colours::white);
            break;

        case KnobStyle::Cap:
            disc (g, c, r, Colour (0xff3a3a3a), Colour (0xff0d0d0d));
            knurl (g, c, r, 30, Colours::white.withAlpha (0.08f));
            disc (g, c, r * 0.6f, skin.knobCap.brighter (0.2f), skin.knobCap.darker (0.35f));
            pointer (g, c, r * 0.05f, r * 0.55f, angle, thick, Colour (0xff1a1a1a));
            pointer (g, c, r * 0.66f, r * 0.96f, angle, thick, Colours::white);
            break;

        case KnobStyle::Chicken:
            disc (g, c, r, Colour (0xff3a3a3c), Colour (0xff0b0b0c));
            knurl (g, c, r, 36, Colours::white.withAlpha (0.09f));
            disc (g, c, r * 0.48f, Colour (0xfff0f0f0), Colour (0xff8a8a8a));
            pointer (g, c, r * 0.55f, r * 0.96f, angle, thick, Colours::white);
            break;

        case KnobStyle::Moog:
            disc (g, c, r, Colour (0xffe4e4e4), Colour (0xff6e6e6e));
            disc (g, c, r * 0.8f, Colour (0xff3b3b3d), Colour (0xff0a0a0b));
            knurl (g, c, r * 0.8f, 40, Colours::white.withAlpha (0.1f));
            pointer (g, c, r * 0.12f, r * 0.78f, angle, thick, Colours::white);
            break;

        case KnobStyle::Modern:
            disc (g, c, r, skin.knobCap.brighter (0.45f), skin.knobCap.darker (0.5f));
            g.setColour (Colours::white.withAlpha (0.12f));
            g.drawEllipse (c.x - r * 0.78f, c.y - r * 0.78f, r * 1.56f, r * 1.56f, 1.0f);
            pointer (g, c, r * 0.35f, r * 0.92f, angle, thick, skin.accent.brighter (0.3f));
            break;
    }

    g.setColour (Colours::black.withAlpha (0.55f));
    g.drawEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f, 0.8f);
}

void fader (Graphics& g, Rect area, float thumbY, float minY, float maxY, const Skin& skin)
{
    const float cx = area.getCentreX();
    const auto slot = Rect (cx - 2.5f, maxY - 4.0f, 5.0f, minY - maxY + 8.0f);

    for (int i = 0; i <= 10; ++i)
    {
        const float y = minY + (maxY - minY) * (float) i / 10.0f;
        const float w = i % 5 == 0 ? 7.0f : 4.5f;
        g.setColour (skin.textDim.withAlpha (0.75f));
        g.drawHorizontalLine ((int) y, cx - 6.0f - w, cx - 6.0f);
        g.drawHorizontalLine ((int) y, cx + 6.0f, cx + 6.0f + w);
    }

    recess (g, slot, 2.5f);
    g.setColour (skin.accent.withAlpha (0.85f));
    g.fillRoundedRectangle (Rect (cx - 1.0f, thumbY, 2.0f, minY - thumbY), 1.0f);

    const float capW = juce::jmin (area.getWidth() - 6.0f, 24.0f), capH = 13.0f;
    const auto cap = Rect (capW, capH).withCentre ({ cx, thumbY });
    g.setColour (Colours::black.withAlpha (0.55f));
    g.fillRoundedRectangle (cap.translated (0.0f, 2.0f), 2.0f);
    g.setGradientFill (juce::ColourGradient (Colour (0xfff4f4f4), cap.getX(), cap.getY(), Colour (0xffa9a9a9), cap.getX(), cap.getBottom(), false));
    g.fillRoundedRectangle (cap, 2.0f);
    g.setColour (Colour (0xff202020));
    g.fillRect (cap.getX() + 2.0f, cap.getCentreY() - 1.0f, cap.getWidth() - 4.0f, 2.0f);
    g.setColour (Colours::black.withAlpha (0.6f));
    g.drawRoundedRectangle (cap, 2.0f, 0.8f);
}

void printedBox (Graphics& g, Rect r, const juce::String& title, const Skin& skin)
{
    g.setColour (skin.line.withAlpha (0.9f));
    g.drawRoundedRectangle (r, 5.0f, 1.0f);

    const auto f = condensed (11.0f);
    const float w = juce::GlyphArrangement::getStringWidth (f, title) + 14.0f;
    const auto tag = Rect (r.getX() + 10.0f, r.getY() - 7.0f, w, 14.0f);
    g.setColour (skin.line);
    g.fillRoundedRectangle (tag, 3.0f);
    g.setColour (skin.lightPlate ? Colours::white : skin.plateBottom);
    g.setFont (f);
    g.drawText (title, tag, juce::Justification::centred);
}

void sevenSegment (Graphics& g, const juce::String& text, Rect area, Colour on)
{
    static const int masks[10] { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };

    int count = 0;
    for (auto ch : text) count += ch == '.' ? 0 : 1;
    const float h = area.getHeight();
    const float w = juce::jmin (h * 0.55f, area.getWidth() / (float) juce::jmax (1, count) - h * 0.12f);
    const float t = w * 0.2f, gap = t * 0.18f;
    float x = area.getX();

    auto hSeg = [&] (juce::Path& p, float x0, float x1, float y)
    {
        p.startNewSubPath (x0, y);
        p.lineTo (x0 + t * 0.5f, y - t * 0.5f);
        p.lineTo (x1 - t * 0.5f, y - t * 0.5f);
        p.lineTo (x1, y);
        p.lineTo (x1 - t * 0.5f, y + t * 0.5f);
        p.lineTo (x0 + t * 0.5f, y + t * 0.5f);
        p.closeSubPath();
    };
    auto vSeg = [&] (juce::Path& p, float xc, float y0, float y1)
    {
        p.startNewSubPath (xc, y0);
        p.lineTo (xc + t * 0.5f, y0 + t * 0.5f);
        p.lineTo (xc + t * 0.5f, y1 - t * 0.5f);
        p.lineTo (xc, y1);
        p.lineTo (xc - t * 0.5f, y1 - t * 0.5f);
        p.lineTo (xc - t * 0.5f, y0 + t * 0.5f);
        p.closeSubPath();
    };

    for (auto ch : text)
    {
        if (ch == '.')
        {
            g.setColour (on);
            g.fillEllipse (x - w * 0.12f, area.getBottom() - t * 1.1f, t, t);
            continue;
        }

        const int mask = juce::CharacterFunctions::isDigit (ch) ? masks[ch - '0'] : ch == '-' ? 0x40 : 0;
        const float left = x + t * 0.5f, right = x + w - t * 0.5f;
        const float top = area.getY() + t * 0.5f, mid = area.getCentreY(), bottom = area.getBottom() - t * 0.5f;

        juce::Path segs[7];
        hSeg (segs[0], left + gap, right - gap, top);
        vSeg (segs[1], right, top + gap, mid - gap);
        vSeg (segs[2], right, mid + gap, bottom - gap);
        hSeg (segs[3], left + gap, right - gap, bottom);
        vSeg (segs[4], left, mid + gap, bottom - gap);
        vSeg (segs[5], left, top + gap, mid - gap);
        hSeg (segs[6], left + gap, right - gap, mid);

        const auto shear = juce::AffineTransform::shear (-0.08f, 0.0f).translated (area.getCentreY() * 0.08f, 0.0f);
        for (int s = 0; s < 7; ++s)
        {
            segs[s].applyTransform (shear);
            const bool lit = (mask >> s) & 1;
            if (lit)
            {
                g.setColour (on.withAlpha (0.25f));
                g.strokePath (segs[s], juce::PathStrokeType (t * 0.6f));
            }
            g.setColour (lit ? on : on.withAlpha (0.07f));
            g.fillPath (segs[s]);
        }
        x += w + h * 0.12f;
    }
}

void vuMeter (Graphics& g, Rect r, float needleDb)
{
    plate (g, r, Colour (0xff1a1a1c), Colour (0xff0c0c0d), 6.0f, false);
    const auto face = r.reduced (5.0f);
    g.setGradientFill (juce::ColourGradient (Colour (0xfff6edd2), face.getX(), face.getY(), Colour (0xffd9c99e), face.getX(), face.getBottom(), false));
    g.fillRoundedRectangle (face, 3.0f);

    juce::Graphics::ScopedSaveState state (g);
    g.reduceClipRegion (face.toNearestInt());

    const Point pivot { face.getCentreX(), face.getBottom() + face.getHeight() * 0.35f };
    const float radius = face.getHeight() * 1.12f;
    const float range = juce::degreesToRadians (46.0f);
    auto angleFor = [range] (float db)
    {
        const float lo = std::pow (10.0f, -20.0f / 20.0f), hi = std::pow (10.0f, 3.0f / 20.0f);
        const float frac = (std::pow (10.0f, juce::jlimit (-22.0f, 4.0f, db) / 20.0f) - lo) / (hi - lo);
        return -range + frac * 2.0f * range;
    };

    juce::Path scale, red;
    scale.addCentredArc (pivot.x, pivot.y, radius * 0.9f, radius * 0.9f, 0.0f, angleFor (-20.0f), angleFor (0.0f), true);
    red.addCentredArc (pivot.x, pivot.y, radius * 0.9f, radius * 0.9f, 0.0f, angleFor (0.0f), angleFor (3.0f), true);
    g.setColour (Colour (0xff222222));
    g.strokePath (scale, juce::PathStrokeType (1.2f));
    g.setColour (Colour (0xffc8281e));
    g.strokePath (red, juce::PathStrokeType (3.0f));

    g.setFont (condensed (9.0f));
    for (int db : { -20, -10, -7, -5, -3, -2, -1, 0, 1, 2, 3 })
    {
        const float a = angleFor ((float) db);
        g.setColour (db > 0 ? Colour (0xffc8281e) : Colour (0xff222222));
        g.drawLine ({ pivot.getPointOnCircumference (radius * 0.9f, a), pivot.getPointOnCircumference (radius * 0.97f, a) }, 1.0f);
        if (db == -20 || db == -10 || db == -5 || db == -3 || db == 0 || db == 3)
        {
            const auto p = pivot.getPointOnCircumference (radius * 1.04f, a);
            g.drawText (juce::String (std::abs (db)), Rect (16.0f, 10.0f).withCentre (p), juce::Justification::centred);
        }
    }
    g.setColour (Colour (0xff222222));
    g.setFont (display (11.0f));
    g.drawText ("VU", face.withTrimmedTop (face.getHeight() * 0.45f).withHeight (14.0f), juce::Justification::centred);

    const float a = angleFor (needleDb);
    g.setColour (Colours::black.withAlpha (0.25f));
    g.drawLine ({ pivot.translated (1.5f, 1.5f), pivot.getPointOnCircumference (radius * 0.98f, a).translated (1.5f, 1.5f) }, 1.4f);
    g.setColour (Colour (0xff1a1a1a));
    g.drawLine ({ pivot, pivot.getPointOnCircumference (radius * 0.98f, a) }, 1.3f);

    g.setGradientFill (juce::ColourGradient (Colours::white.withAlpha (0.35f), face.getX(), face.getY(), Colours::transparentWhite,
                                             face.getX(), face.getCentreY(), false));
    g.fillRect (face);
}

void icon (Graphics& g, const juce::String& name, Rect r, Colour colour)
{
    g.setColour (colour);
    const float s = juce::jmin (r.getWidth(), r.getHeight());
    const auto b = Rect (s, s).withCentre (r.getCentre());

    if (name == "play")
    {
        juce::Path p;
        p.addTriangle (b.getX() + s * 0.2f, b.getY() + s * 0.1f, b.getX() + s * 0.2f, b.getBottom() - s * 0.1f, b.getRight() - s * 0.08f, b.getCentreY());
        g.fillPath (p);
    }
    else if (name == "stop")
    {
        g.fillRoundedRectangle (b.reduced (s * 0.15f), s * 0.08f);
    }
    else if (name == "rec")
    {
        g.fillEllipse (b.reduced (s * 0.12f));
    }
    else if (name == "dice")
    {
        g.drawRoundedRectangle (b.reduced (s * 0.08f), s * 0.18f, juce::jmax (1.2f, s * 0.08f));
        for (auto p : { Point (0.32f, 0.32f), Point (0.68f, 0.68f), Point (0.5f, 0.5f), Point (0.68f, 0.32f), Point (0.32f, 0.68f) })
            g.fillEllipse (Rect (s * 0.14f, s * 0.14f).withCentre ({ b.getX() + p.x * s, b.getY() + p.y * s }));
    }
    else if (name == "plus")
    {
        const float t = juce::jmax (1.6f, s * 0.14f);
        g.fillRect (Rect (s * 0.7f, t).withCentre (b.getCentre()));
        g.fillRect (Rect (t, s * 0.7f).withCentre (b.getCentre()));
    }
    else if (name == "close")
    {
        const float t = juce::jmax (1.4f, s * 0.12f), m = s * 0.25f;
        g.drawLine (b.getX() + m, b.getY() + m, b.getRight() - m, b.getBottom() - m, t);
        g.drawLine (b.getRight() - m, b.getY() + m, b.getX() + m, b.getBottom() - m, t);
    }
    else if (name == "down" || name == "up" || name == "right")
    {
        juce::Path p;
        if (name == "up") p.addTriangle (b.getX() + s * 0.2f, b.getBottom() - s * 0.35f, b.getRight() - s * 0.2f, b.getBottom() - s * 0.35f, b.getCentreX(), b.getY() + s * 0.25f);
        else if (name == "down") p.addTriangle (b.getX() + s * 0.2f, b.getY() + s * 0.35f, b.getRight() - s * 0.2f, b.getY() + s * 0.35f, b.getCentreX(), b.getBottom() - s * 0.25f);
        else p.addTriangle (b.getX() + s * 0.35f, b.getY() + s * 0.2f, b.getX() + s * 0.35f, b.getBottom() - s * 0.2f, b.getRight() - s * 0.25f, b.getCentreY());
        g.fillPath (p);
    }
    else if (name == "more")
    {
        for (int i = -1; i <= 1; ++i)
            g.fillEllipse (Rect (s * 0.16f, s * 0.16f).withCentre ({ b.getCentreX() + (float) i * s * 0.28f, b.getCentreY() }));
    }
    else if (name == "undo" || name == "redo")
    {
        juce::Path arc;
        const bool undo = name == "undo";
        arc.addCentredArc (b.getCentreX(), b.getCentreY() + s * 0.08f, s * 0.3f, s * 0.26f, 0.0f,
                           undo ? -2.2f : 2.2f, undo ? 1.3f : -1.3f, true);
        g.strokePath (arc, juce::PathStrokeType (juce::jmax (1.4f, s * 0.1f)));
        juce::Path head;
        const float hx = undo ? b.getCentreX() - s * 0.26f : b.getCentreX() + s * 0.26f, hy = b.getCentreY() - s * 0.12f;
        head.addTriangle (hx - s * 0.14f, hy, hx + s * 0.14f, hy, hx, hy - s * 0.2f);
        head.applyTransform (juce::AffineTransform::rotation (undo ? -0.6f : 0.6f, hx, hy));
        g.fillPath (head);
    }
    else if (name == "fullscreen")
    {
        const float t = juce::jmax (1.4f, s * 0.1f), m = s * 0.18f, l = s * 0.26f;
        const auto box = b.reduced (m);
        for (auto corner : { box.getTopLeft(), box.getTopRight(), box.getBottomLeft(), box.getBottomRight() })
        {
            const float dx = corner.x < box.getCentreX() ? l : -l, dy = corner.y < box.getCentreY() ? l : -l;
            g.drawLine (corner.x, corner.y, corner.x + dx, corner.y, t);
            g.drawLine (corner.x, corner.y, corner.x, corner.y + dy, t);
        }
    }
    else if (name == "keys")
    {
        const float t = juce::jmax (1.2f, s * 0.08f);
        const auto box = b.reduced (s * 0.12f, s * 0.2f);
        g.drawRect (box, t);
        for (int i = 1; i < 4; ++i)
            g.drawVerticalLine ((int) (box.getX() + box.getWidth() * (float) i / 4.0f), box.getY(), box.getBottom());
        for (float f : { 0.25f, 0.5f, 0.75f })
            g.fillRect (Rect (box.getWidth() * 0.12f, box.getHeight() * 0.55f).withCentre ({ box.getX() + box.getWidth() * f, box.getY() + box.getHeight() * 0.275f }));
    }
    else if (name == "sound")
    {
        // A small waveform.
        const float t = juce::jmax (1.4f, s * 0.1f), cy = b.getCentreY();
        static constexpr float heights[] { 0.2f, 0.5f, 0.85f, 0.45f, 0.65f, 0.3f };
        for (int i = 0; i < 6; ++i)
        {
            const float x = b.getX() + s * (0.15f + 0.14f * (float) i), h = s * 0.7f * heights[i];
            g.drawLine (x, cy - h * 0.5f, x, cy + h * 0.5f, t * 1.3f);
        }
    }
    else if (name == "gear")
    {
        const auto c = b.getCentre();
        const float t = juce::jmax (1.4f, s * 0.1f);
        for (int i = 0; i < 8; ++i)
        {
            const float a = (float) i * juce::MathConstants<float>::pi / 4.0f;
            g.drawLine (c.x + std::cos (a) * s * 0.22f, c.y + std::sin (a) * s * 0.22f,
                        c.x + std::cos (a) * s * 0.4f, c.y + std::sin (a) * s * 0.4f, t * 1.4f);
        }
        g.drawEllipse (Rect (s * 0.5f, s * 0.5f).withCentre (c), t);
        g.fillEllipse (Rect (s * 0.16f, s * 0.16f).withCentre (c));
    }
    else if (name == "globe")
    {
        const auto c = b.getCentre();
        const float t = juce::jmax (1.2f, s * 0.08f), radius = s * 0.38f;
        g.drawEllipse (Rect (2.0f * radius, 2.0f * radius).withCentre (c), t);
        g.drawEllipse (Rect (radius, 2.0f * radius).withCentre (c), t);
        g.drawLine (c.x - radius, c.y, c.x + radius, c.y, t);
        g.drawLine (c.x - radius * 0.85f, c.y - radius * 0.5f, c.x + radius * 0.85f, c.y - radius * 0.5f, t);
        g.drawLine (c.x - radius * 0.85f, c.y + radius * 0.5f, c.x + radius * 0.85f, c.y + radius * 0.5f, t);
    }
    else if (name == "patterns")
    {
        const float cell = s * 0.22f, gap = s * 0.1f, x0 = b.getX() + s * 0.13f, y0 = b.getY() + s * 0.13f;
        for (int row = 0; row < 3; ++row)
            for (int col = 0; col < 3; ++col)
            {
                const Rect square (x0 + (float) col * (cell + gap), y0 + (float) row * (cell + gap), cell, cell);
                if ((row + col) % 2 == 0) g.fillRoundedRectangle (square, s * 0.04f);
                else g.drawRoundedRectangle (square.reduced (s * 0.02f), s * 0.04f, juce::jmax (1.0f, s * 0.05f));
            }
    }
    else if (name == "folder")
    {
        juce::Path p;
        p.addRoundedRectangle (b.getX() + s * 0.1f, b.getY() + s * 0.3f, s * 0.8f, s * 0.5f, s * 0.06f);
        p.addRoundedRectangle (b.getX() + s * 0.1f, b.getY() + s * 0.22f, s * 0.32f, s * 0.14f, s * 0.04f);
        g.fillPath (p);
    }
    else if (name == "trash")
    {
        const float t = juce::jmax (1.2f, s * 0.09f);
        g.drawRoundedRectangle (Rect (b.getX() + s * 0.25f, b.getY() + s * 0.3f, s * 0.5f, s * 0.58f), s * 0.06f, t);
        g.fillRect (Rect (b.getX() + s * 0.15f, b.getY() + s * 0.18f, s * 0.7f, t));
        g.fillRect (Rect (b.getX() + s * 0.4f, b.getY() + s * 0.1f, s * 0.2f, t));
    }
}
} // namespace techno::paint
