#include "Factories.h"
#include "SynthMachines.h"

namespace techno
{
namespace
{
class JunoMachine : public PolyMachine
{
public:
    JunoMachine() : PolyMachine (Juno106)
    {
        gateFraction = 0.75f;
        for (int i = 0; i < 8; ++i)
        {
            auto v = std::make_unique<JunoVoice> (shared);
            junoVoices.push_back (v.get());
            voices.push_back (std::move (v));
        }
    }

    void prepare (float sampleRate) override
    {
        PolyMachine::prepare (sampleRate);
        chorus.prepare (sampleRate);
    }

private:
    void applyParams() override
    {
        shared.saw = values[p106::Saw];
        shared.pulse = values[p106::Pulse];
        shared.sub = values[p106::Sub];
        shared.noise = values[p106::Noise];
        shared.pwm = values[p106::Pwm];
        shared.hpf = (int) values[p106::Hpf];
        shared.cutoff = values[p106::Cutoff];
        shared.reso = values[p106::Reso];
        shared.env = values[p106::Env];
        chorusMode = (int) values[p106::Chorus];
        for (auto* v : junoVoices)
            v->setEnvelope (knobTime (values[p106::Attack], 0.001f, 3.0f), knobTime (values[p106::Decay], 0.005f, 4.0f),
                            values[p106::Sustain], knobTime (values[p106::Release], 0.005f, 5.0f));
    }

    void postProcess (float& left, float& right) override
    {
        advance (lfoPhase, 0.6f / fs);
        shared.lfo = 1.0f - 4.0f * std::abs (lfoPhase - 0.5f);
        chorus.process (left * 2.2f, chorusMode, left, right);
    }

    JunoParams shared;
    std::vector<JunoVoice*> junoVoices;
    BbdChorus chorus;
    int chorusMode = 1;
    float lfoPhase = 0;
};

class SuperMachine : public PolyMachine
{
public:
    SuperMachine() : PolyMachine (JP8000)
    {
        gateFraction = 0.75f;
        for (int i = 0; i < 8; ++i)
        {
            auto v = std::make_unique<SuperVoice> (shared);
            superVoices.push_back (v.get());
            voices.push_back (std::move (v));
        }
    }

private:
    void applyParams() override
    {
        shared.detune = values[p8000::Detune];
        shared.mix = values[p8000::Mix];
        shared.cutoff = values[p8000::Cutoff];
        shared.reso = values[p8000::Reso];
        shared.env = values[p8000::Env];
        for (auto* v : superVoices)
            v->setEnvelope (knobTime (values[p8000::Attack], 0.001f, 3.0f), knobTime (values[p8000::Decay], 0.005f, 4.0f),
                            values[p8000::Sustain], knobTime (values[p8000::Release], 0.005f, 5.0f));
    }

    void postProcess (float& left, float& right) override
    {
        left *= 1.8f;
        right *= 1.8f;
    }

    SuperParams shared;
    std::vector<SuperVoice*> superVoices;
};
class FmMachine : public PolyMachine
{
public:
    explicit FmMachine (int machineType = DX7, int firstPreset = 0) : PolyMachine (machineType), presetOffset (firstPreset)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i) voices.push_back (std::make_unique<FmVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.preset = presetOffset + (int) values[pDX7::Preset];
        shared.brightness = values[pDX7::Brightness];
        shared.decay = values[pDX7::Decay];
        shared.release = values[pDX7::Release];
        shared.detune = values[pDX7::Detune];
        shared.tremolo = values[pDX7::Tremolo];
    }

    void postProcess (float& left, float& right) override
    {
        advance (lfoPhase, 5.2f / fs);
        shared.lfo = std::sin (2.0f * pi * lfoPhase);
        left *= 0.85f;
        right *= 0.85f;
    }

    FmParams shared;
    int presetOffset = 0;
    float lfoPhase = 0;
};

class StringMachine : public PolyMachine
{
public:
    StringMachine() : PolyMachine (Strings)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i)
        {
            auto v = std::make_unique<StringVoice> (shared);
            stringVoices.push_back (v.get());
            voices.push_back (std::move (v));
        }
    }

    void prepare (float sampleRate) override
    {
        PolyMachine::prepare (sampleRate);
        ensemble.prepare (sampleRate);
    }

private:
    void applyParams() override
    {
        shared.violin = values[pStrings::Violin];
        shared.cello = values[pStrings::Cello];
        shared.brightness = values[pStrings::Brightness];
        ensembleAmount = values[pStrings::Ensemble];
        for (auto* v : stringVoices)
            v->setEnvelope (knobTime (values[pStrings::Attack], 0.005f, 3.0f), knobTime (values[pStrings::Release], 0.02f, 5.0f));
    }

    void postProcess (float& left, float& right) override
    {
        ensemble.process (left * 1.8f, ensembleAmount, left, right);
    }

    StringParams shared;
    std::vector<StringVoice*> stringVoices;
    EnsembleChorus ensemble;
    float ensembleAmount = 0.7f;
};

class OrganMachine : public PolyMachine
{
public:
    OrganMachine() : PolyMachine (Organ)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i) voices.push_back (std::make_unique<OrganVoice> (shared));
        buildOrganTable (shared, 0.8f, 0.5f, 0.3f, false);
    }

    void prepare (float sampleRate) override
    {
        PolyMachine::prepare (sampleRate);
        leslie.prepare (sampleRate);
    }

private:
    void applyParams() override
    {
        const float bass = values[pOrgan::Bass], middle = values[pOrgan::Middle], high = values[pOrgan::High];
        const bool house = values[pOrgan::Type] > 0.5f;
        if (bass != lastBass || middle != lastMiddle || high != lastHigh || house != lastHouse)
        {
            buildOrganTable (shared, bass, middle, high, house);
            lastBass = bass;
            lastMiddle = middle;
            lastHigh = high;
            lastHouse = house;
        }
        shared.percussion = values[pOrgan::Percussion];
        shared.click = values[pOrgan::Click];
        leslieAmount = values[pOrgan::Leslie];
        drive = 1.0f + values[pOrgan::Drive] * 8.0f;
    }

    void postProcess (float& left, float& right) override
    {
        const float x = fastTanh (left * 1.6f * drive) / fastTanh (drive);
        leslie.process (x * 0.55f, leslieAmount, left, right);
    }

    OrganParams shared;
    Leslie leslie;
    float lastBass = -1, lastMiddle = -1, lastHigh = -1, leslieAmount = 0.4f, drive = 1.0f;
    bool lastHouse = false;
};

class PianoMachine : public PolyMachine
{
public:
    PianoMachine() : PolyMachine (Piano)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i) voices.push_back (std::make_unique<PianoVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pPiano::Type];
        shared.brightness = values[pPiano::Brightness];
        shared.decay = values[pPiano::Decay];
        shared.release = values[pPiano::Release];
        shared.hammer = values[pPiano::Hammer];
        shared.width = values[pPiano::Width];
    }

    PianoParams shared;
};

class MalletMachine : public PolyMachine
{
public:
    MalletMachine() : PolyMachine (Mallets)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i) voices.push_back (std::make_unique<MalletVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pMallets::Type];
        shared.brightness = values[pMallets::Brightness];
        shared.decay = values[pMallets::Decay];
        shared.release = values[pMallets::Release];
        shared.mallet = values[pMallets::Mallet];
        shared.vibrato = values[pMallets::Vibrato];
    }

    void postProcess (float&, float&) override
    {
        advance (lfoPhase, 5.5f / fs);
        shared.lfo = std::sin (2.0f * pi * lfoPhase);
    }

    MalletParams shared;
    float lfoPhase = 0;
};

class PluckMachine : public PolyMachine
{
public:
    PluckMachine() : PolyMachine (Pluck)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 8; ++i) voices.push_back (std::make_unique<PluckVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pPluck::Type];
        shared.brightness = values[pPluck::Brightness];
        shared.decay = values[pPluck::Decay];
        shared.release = values[pPluck::Release];
        shared.strum = values[pPluck::Strum];
        shared.body = values[pPluck::Body];
    }

    // All the notes of a chord start during the same step, before the next sample: the strum restarts here.
    void postProcess (float&, float&) override { shared.strumIndex = 0; }

    PluckParams shared;
};

class ChoirMachine : public PolyMachine
{
public:
    ChoirMachine() : PolyMachine (Choir)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 8; ++i)
        {
            auto v = std::make_unique<ChoirVoice> (shared);
            choirVoices.push_back (v.get());
            voices.push_back (std::move (v));
        }
    }

    void prepare (float sampleRate) override
    {
        PolyMachine::prepare (sampleRate);
        ensemble.prepare (sampleRate);
    }

private:
    void applyParams() override
    {
        shared.vowel = (int) values[pChoir::Vowel];
        shared.ensemble = values[pChoir::Ensemble];
        shared.brightness = values[pChoir::Brightness];
        shared.breath = values[pChoir::Breath];
        for (auto* v : choirVoices)
            v->setEnvelope (knobTime (values[pChoir::Attack], 0.01f, 3.0f), knobTime (values[pChoir::Release], 0.05f, 5.0f));
    }

    void postProcess (float& left, float& right) override { ensemble.process (left, 0.3f + shared.ensemble * 0.6f, left, right); }

    ChoirParams shared;
    std::vector<ChoirVoice*> choirVoices;
    EnsembleChorus ensemble;
};

class HooverMachine : public PolyMachine
{
public:
    HooverMachine() : PolyMachine (Hoover)
    {
        gateFraction = 0.85f;
        for (int i = 0; i < 8; ++i) voices.push_back (std::make_unique<HooverVoice> (shared));
    }

    void prepare (float sampleRate) override
    {
        PolyMachine::prepare (sampleRate);
        chorus.prepare (sampleRate);
    }

private:
    void applyParams() override
    {
        shared.detune = values[pHoov::Detune];
        shared.dip = values[pHoov::Dip];
        shared.cutoff = values[pHoov::Cutoff];
        shared.attack = knobTime (values[pHoov::Attack], 0.002f, 1.5f);
        shared.release = knobTime (values[pHoov::Release], 0.02f, 3.0f);
        chorusMode = (int) values[pHoov::Chorus];
    }

    void postProcess (float& left, float& right) override
    {
        const float mono = (left + right) * 0.5f;
        if (chorusMode > 0) chorus.process (mono * 2.0f, chorusMode, left, right);
        else left = right = mono * 2.0f;
    }

    HooverParams shared;
    BbdChorus chorus;
    int chorusMode = 3;
};

class Poly80Machine : public PolyMachine
{
public:
    Poly80Machine() : PolyMachine (Poly80)
    {
        gateFraction = 0.75f;
        for (int i = 0; i < 8; ++i)
        {
            auto v = std::make_unique<Poly80Voice> (shared);
            polyVoices.push_back (v.get());
            voices.push_back (std::move (v));
        }
    }

private:
    void applyParams() override
    {
        shared.saw = values[pP80::Saw];
        shared.pulse = values[pP80::Pulse];
        shared.detune = values[pP80::Detune];
        shared.cutoff = values[pP80::Cutoff];
        shared.reso = values[pP80::Reso];
        shared.env = values[pP80::Env];
        for (auto* v : polyVoices)
            v->setEnvelope (knobTime (values[pP80::Attack], 0.001f, 3.0f), knobTime (values[pP80::Decay], 0.005f, 4.0f),
                            values[pP80::Sustain], knobTime (values[pP80::Release], 0.005f, 5.0f));
    }

    Poly80Params shared;
    std::vector<Poly80Voice*> polyVoices;
};

// Supersaw voices with a sidechain-like volume pump locked to the beats.
class EdmMachine : public PolyMachine
{
public:
    EdmMachine() : PolyMachine (Edm)
    {
        gateFraction = 0.8f;
        for (int i = 0; i < 8; ++i)
        {
            auto v = std::make_unique<SuperVoice> (shared);
            superVoices.push_back (v.get());
            voices.push_back (std::move (v));
        }
    }

    void onStep (const StepInfo& s) override
    {
        PolyMachine::onStep (s);
        beatSamples = (float) s.nominalStepSamples * 4.0f;
        if (s.step % 4 == 0) sinceBeat = 0.0f;
    }

private:
    void applyParams() override
    {
        shared.detune = values[pEdm::Detune];
        shared.mix = values[pEdm::Mix];
        shared.cutoff = values[pEdm::Cutoff];
        shared.reso = values[pEdm::Reso];
        shared.env = values[pEdm::Env];
        pump = values[pEdm::Pump];
        for (auto* v : superVoices)
            v->setEnvelope (knobTime (values[pEdm::Attack], 0.001f, 3.0f), knobTime (values[pEdm::Decay], 0.005f, 4.0f),
                            values[pEdm::Sustain], knobTime (values[pEdm::Release], 0.005f, 5.0f));
    }

    void postProcess (float& left, float& right) override
    {
        sinceBeat += 1.0f;
        const float x = juce::jmin (1.0f, sinceBeat / (beatSamples * 0.6f));
        const float duck = 1.0f - pump * (1.0f - x) * (1.0f - x);
        left *= 2.5f * duck;
        right *= 2.5f * duck;
    }

    SuperParams shared;
    std::vector<SuperVoice*> superVoices;
    float pump = 0.6f, beatSamples = 24000.0f, sinceBeat = 0.0f;
};

class TalkboxMachine : public PolyMachine
{
public:
    TalkboxMachine() : PolyMachine (Talkbox)
    {
        gateFraction = 0.85f;
        for (int i = 0; i < 6; ++i) voices.push_back (std::make_unique<TalkboxVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.from = (int) values[pTalk::From];
        shared.to = (int) values[pTalk::To];
        shared.speed = values[pTalk::Speed];
        shared.brightness = values[pTalk::Brightness];
        shared.release = values[pTalk::Release];
    }

    TalkboxParams shared;
};

class MellotronMachine : public PolyMachine
{
public:
    MellotronMachine() : PolyMachine (Mellotron)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i)
        {
            auto v = std::make_unique<MellotronVoice> (shared);
            melloVoices.push_back (v.get());
            voices.push_back (std::move (v));
        }
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pMello::Type];
        shared.tone = values[pMello::Tone];
        shared.wow = values[pMello::Wow];
        hiss = values[pMello::Noise];
        for (auto* v : melloVoices)
            v->setEnvelope (knobTime (values[pMello::Attack], 0.01f, 1.5f), knobTime (values[pMello::Release], 0.03f, 2.0f));
    }

    void postProcess (float& left, float& right) override
    {
        advance (slow, 0.55f / fs);
        advance (fast, 6.8f / fs);
        shared.wobble = std::sin (2.0f * pi * slow) + 0.3f * std::sin (2.0f * pi * fast);
        hissFilter += 0.2f * (noise.next() - hissFilter);
        const float h = hissFilter * hiss * 0.01f;
        left = left * 0.65f + h;
        right = right * 0.65f + h;
    }

    MellotronParams shared;
    std::vector<MellotronVoice*> melloVoices;
    Noise noise;
    float slow = 0, fast = 0, hiss = 0.3f, hissFilter = 0;
};

class EPianoMachine : public PolyMachine
{
public:
    EPianoMachine() : PolyMachine (EPiano)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i) voices.push_back (std::make_unique<EPianoVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pEP::Type];
        shared.tone = values[pEP::Tone];
        shared.decay = values[pEP::Decay];
        shared.release = values[pEP::Release];
        shared.bark = values[pEP::Bark];
        tremolo = values[pEP::Tremolo];
    }

    // Stereo tremolo: the sound moves from one side to the other.
    void postProcess (float& left, float& right) override
    {
        advance (lfo, 4.6f / fs);
        const float pan = std::sin (2.0f * pi * lfo) * tremolo * 0.7f;
        const float mono = (left + right) * 0.5f * 0.65f;
        left = mono * (1.0f - pan);
        right = mono * (1.0f + pan);
    }

    EPianoParams shared;
    float tremolo = 0.3f, lfo = 0;
};

class BrassMachine : public PolyMachine
{
public:
    BrassMachine() : PolyMachine (Brass)
    {
        gateFraction = 0.75f;
        for (int i = 0; i < 8; ++i) voices.push_back (std::make_unique<BrassVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pBrass::Type];
        shared.brightness = values[pBrass::Brightness];
        shared.attack = values[pBrass::Attack];
        shared.release = values[pBrass::Release];
        shared.vibrato = values[pBrass::Vibrato];
        shared.growl = values[pBrass::Growl];
    }

    BrassParams shared;
};

class WindMachine : public PolyMachine
{
public:
    WindMachine() : PolyMachine (Winds)
    {
        gateFraction = 0.85f;
        for (int i = 0; i < 8; ++i) voices.push_back (std::make_unique<WindVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pWind::Type];
        shared.breath = values[pWind::Breath];
        shared.vibrato = values[pWind::Vibrato];
        shared.attack = values[pWind::Attack];
        shared.release = values[pWind::Release];
        shared.brightness = values[pWind::Brightness];
    }

    WindParams shared;
};

class OrchestraMachine : public PolyMachine
{
public:
    OrchestraMachine() : PolyMachine (Orchestra)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i) voices.push_back (std::make_unique<OrchestraVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pOrch::Type];
        shared.attack = values[pOrch::Attack];
        shared.release = values[pOrch::Release];
        shared.brightness = values[pOrch::Brightness];
        shared.ensemble = values[pOrch::Ensemble];
    }

    OrchestraParams shared;
};
} // namespace

std::unique_ptr<Machine> createInstrumentMachine (int type)
{
    switch (type)
    {
        case Juno106:   return std::make_unique<JunoMachine>();
        case JP8000:    return std::make_unique<SuperMachine>();
        case DX7:       return std::make_unique<FmMachine>();
        case M1:        return std::make_unique<FmMachine> (M1, 30);
        case Strings:   return std::make_unique<StringMachine>();
        case Organ:     return std::make_unique<OrganMachine>();
        case Piano:     return std::make_unique<PianoMachine>();
        case Mallets:   return std::make_unique<MalletMachine>();
        case Pluck:     return std::make_unique<PluckMachine>();
        case Choir:     return std::make_unique<ChoirMachine>();
        case Hoover:    return std::make_unique<HooverMachine>();
        case Poly80:    return std::make_unique<Poly80Machine>();
        case Edm:       return std::make_unique<EdmMachine>();
        case Talkbox:   return std::make_unique<TalkboxMachine>();
        case Mellotron: return std::make_unique<MellotronMachine>();
        case EPiano:    return std::make_unique<EPianoMachine>();
        case Brass:     return std::make_unique<BrassMachine>();
        case Winds:     return std::make_unique<WindMachine>();
        case Orchestra: return std::make_unique<OrchestraMachine>();
        default:        break;
    }
    jassertfalse;
    return nullptr;
}
} // namespace techno
