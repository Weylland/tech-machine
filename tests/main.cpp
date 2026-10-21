#include "TestHelpers.h"

#include <cstdlib>
#if JUCE_WINDOWS
 #include <crtdbg.h>
#endif

// Test program of Tech Machine.
//   TechMachineTests check        every automatic test, exit code = number of failures (used by the CI)
//   TechMachineTests <mode> [...] one test or tool (golden, sweep, chaos, choices, levels, ui, song...)
// Renders, snapshots and WAV files are written next to the executable.

static int chaosIterations = 40;

static int test_host (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    // Simulated host at 120 BPM starting at ppq 1.3: the first kick must land exactly on beat 2.
    juce::AudioProcessor::setTypeOfNextNewPlugin (juce::AudioProcessor::wrapperType_VST3);
    TechMachineProcessor p;
    juce::AudioProcessor::setTypeOfNextNewPlugin (juce::AudioProcessor::wrapperType_Undefined);
    p.setShown (TR909, true);
    for (int s = 0; s < maxSteps; s += 4) p.machine (TR909).track.hits[0][(size_t) s] = true;
    FakeHead head;
    head.ppq = 1.3;
    p.setPlayHead (&head);
    const double rate = 48000.0;
    const int bs = 480;
    p.setPlayConfigDetails (0, 2, rate, bs);
    p.prepareToPlay (rate, bs);
    juce::AudioBuffer<float> block (2, bs);
    juce::MidiBuffer midi;
    std::vector<float> mono;
    for (int b = 0; b < 400; ++b)
    {
        p.processBlock (block, midi);
        for (int i = 0; i < bs; ++i) mono.push_back (block.getSample (0, i));
        head.ppq += bs / rate * head.bpm / 60.0;
    }
    // Onset = a 2 ms window much louder than the previous one.
    std::vector<int> onsets;
    const int win = 96;
    float prevPeak = 0.0f;
    for (int start = 0; start + win < (int) mono.size(); start += win)
    {
        float peak = 0.0f;
        for (int i = start; i < start + win; ++i) peak = std::max (peak, std::abs (mono[(size_t) i]));
        if (peak > 0.2f && peak > prevPeak * 3.0f)
            for (int i = start; i < start + win; ++i)
                if (std::abs (mono[(size_t) i]) > 0.01f) { onsets.push_back (i); break; }
        prevPeak = peak;
    }
    printf ("host sync: tempo shown %.1f, first onsets (samples):", p.displayBpm.load());
    for (size_t i = 0; i < juce::jmin<size_t> (5, onsets.size()); ++i) printf (" %d", onsets[i]);
    printf ("\n  expected: %d then every %d\n", (int) ((2.0 - 1.3) * 0.5 * rate), (int) (0.5 * rate));
    return 0;
}

static int test_levels (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    for (int t = 0; t < numMachines; ++t)
    {
        if (machineDef (t).kind != MachineKind::Drum) continue;
        for (int r = 0; r < (int) machineDef (t).rows.size(); ++r)
        {
            TechMachineProcessor p;
            p.newProject();
            p.setShown (t, true);
            for (int s = 0; s < maxSteps; s += 4) p.machine (t).track.hits[(size_t) r][(size_t) s] = true;
            print (juce::String ("row ") + machineDef (t).id + " " + machineDef (t).rows[(size_t) r].id, run (p, 2.0));
        }
    }
    for (int t = 0; t < numMachines; ++t)
    {
        if (machineDef (t).kind == MachineKind::Drum) continue;
        TechMachineProcessor p;
        p.newProject();
        p.setShown (t, true);
        auto& tr = p.machine (t).track;
        for (int s = 0; s < maxSteps; s += 2) { tr.on[(size_t) s] = true; tr.note[(size_t) s] = (s / 2) % 3 == 0 ? 0 : 7; }
        print (juce::String ("synth ") + machineDef (t).id, run (p, 4.0, juce::String ("synth_") + machineDef (t).id));
    }
    return 0;
}

static int test_examples (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    for (int e = 0; e < (int) examples().size(); ++e)
    {
        TechMachineProcessor p;
        p.loadExample (e);
        print (juce::String ("example ") + examples()[(size_t) e].name, run (p, 16.0, juce::String ("example_") + juce::String (e)));
    }
    return 0;
}

static int test_voice (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    const juce::String machine = argc > 2 ? argv[2] : "tr909", row = argc > 3 ? argv[3] : "oh";
    juce::Random rnd;
    int blown = 0;
    float worst = 0;
    for (int it = 0; it < (argc > 4 ? juce::String (argv[4]).getIntValue() : 300); ++it)
    {
        auto v = createDrumVoice (machine, row);
        v->prepare (48000.0f);
        DrumSettings st;
        float peak = 0;
        int firstBad = -1;
        for (int i = 0; i < 72000; ++i)
        {
            if (i % 6000 == 0) v->trigger (0.85f + 0.15f * rnd.nextFloat(), st);
            const float x = v->process();
            if (std::abs (x) > peak) peak = std::abs (x);
            if (firstBad < 0 && ! (std::abs (x) < 4.0f)) firstBad = i;
        }
        worst = std::max (worst, peak);
        if (firstBad >= 0) { ++blown; if (blown < 4) printf ("  blown at sample %d\n", firstBad); }
    }
    printf ("voice %s/%s blown %d/300 worst %g\n", machine.toRawUTF8(), row.toRawUTF8(), blown, worst);
    return 0;
}

static int test_reverb (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    int blown = 0;
    for (int it = 0; it < 500; ++it)
    {
        auto r = std::make_unique<FdnReverb>();
        r->prepare (48000.0f);
        r->setSize (0.5f);
        float worst = 0, l, rr;
        for (int i = 0; i < 48000; ++i)
        {
            r->process (0.0f, l, rr);
            worst = std::max (worst, std::abs (l));
        }
        if (worst > 0.0f) { ++blown; if (blown < 4) printf ("  reverb %d worst %g\n", it, worst); }
        std::vector<float> junk ((size_t) (1000 + it * 37), 1e20f);
    }
    printf ("reverb instances with output from silence: %d/500\n", blown);
    return 0;
}

static int test_fx (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    for (int r = 0; r < (int) machineDef (SoundFx).rows.size(); ++r)
    {
        TechMachineProcessor p;
        p.newProject();
        p.setShown (SoundFx, true);
        p.setBars (4);
        p.machine (SoundFx).track.hits[(size_t) r][0] = true;
        print (juce::String ("fx single ") + machineDef (SoundFx).rows[(size_t) r].id, run (p, 8.0, juce::String ("fx_") + machineDef (SoundFx).rows[(size_t) r].id));
    }
    return 0;
}

static int test_audit (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    // Every drum pattern on every kit: hits asked for vs hits placed, and the parts that found no sound.
    int lost = 0, total = 0;
    for (int t = 0; t < numMachines; ++t)
    {
        const auto& def = machineDef (t);
        if (def.kind != MachineKind::Drum || patternKindsFor (def).empty()) continue;
        juce::StringArray report;
        for (auto& pat : patterns())
        {
            if (pat.kind != PatternKind::Drums) continue;
            if (patternCoverage (def, pat) < 0.75f) { report.add (juce::String ("(hidden) ") + pat.name); continue; }
            TechMachineProcessor p;
            p.newProject();
            p.setShown (t, true);
            p.setBars (2);
            p.applyPattern (t, pat);
            juce::StringArray missing;
            for (auto& line : pat.drums)
            {
                bool needsGhostOnly = true;
                for (const char* c = line.steps; *c != 0; ++c) if (*c != '.' && *c != 'g') needsGhostOnly = false;
                if (needsGhostOnly) continue;
                ++total;
                if (rowForRole (def, line.role) < 0) { ++lost; missing.add (juce::String ((int) line.role)); }
            }
            int placed = 0;
            for (auto& row : p.machine (t).track.hits)
                for (int s2 = 0; s2 < 32; ++s2) placed += row[(size_t) s2] ? 1 : 0;
            if (! missing.isEmpty() || placed == 0)
                report.add (juce::String (pat.name) + " (roles " + missing.joinIntoString (",") + ", hits " + juce::String (placed) + ")");
        }
        printf ("[%s] %d patterns incomplete\n", def.id, report.size());
        for (auto& r : report) printf ("    %s\n", r.toRawUTF8());
    }
    printf ("lines without a sound: %d / %d\n", lost, total);
    return 0;
}

static int test_surprise (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    static const char* names[] { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    for (int g = 0; g < (int) styles().size(); ++g)
    {
        const juce::String genre = juce::String::fromUTF8 (styles()[(size_t) g].name);
        if (argc > 2 && ! genre.containsIgnoreCase (argv[2])) continue;
        TechMachineProcessor p;
        p.newProject();
        p.setGenre (g);
        p.generateAll();
        printf ("== %s (%d bars, progression", genre.toRawUTF8(), p.numSteps() / 16);

        printf (")\n");
        for (int t = 0; t < numMachines; ++t)
        {
            if (! p.isShown (t)) continue;
            const auto& def = machineDef (t);
            if (def.kind == MachineKind::Drum) { printf ("  [%s] drums\n", def.id); continue; }
            auto& tr = p.machine (t).track;
            juce::String line;
            for (int st = 0; st < p.numSteps(); ++st)
            {
                if (st % 16 == 0) line << "| ";
                if (! tr.on[(size_t) st]) { line << ".  "; continue; }
                const bool tie = st > 0 && tr.on[(size_t) st - 1] && tr.slide[(size_t) st - 1] && tr.note[(size_t) st - 1] == tr.note[(size_t) st];
                if (tie) { line << "-  "; continue; }
                const int n = snapToScale (p.currentScale(), tr.note[(size_t) st]);
                line << juce::String (names[(n + p.currentKey()) % 12]).paddedRight (' ', 3);
            }
            printf ("  [%s] %s\n", def.id, line.toRawUTF8());
        }
        run (p, 12.0, juce::String ("surprise_") + juce::String (g));
    }
    return 0;
}

static int test_midi (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    TechMachineProcessor p;
    p.newProject();
    p.setShown (TR909, true);
    p.setShown (SH101, true);
    p.setBars (1);
    p.setParam (pid::bpm, 120.0f);
    p.setPlayConfigDetails (0, 2, 48000.0, 480);
    p.prepareToPlay (48000.0, 480);
    juce::AudioBuffer<float> block (2, 480);

    // Live: a note on the 101 while the sequencer is stopped must sound.
    p.setMidiTarget (SH101);
    juce::MidiBuffer on;
    on.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 100), 0);
    p.processBlock (block, on);
    float peak = 0;
    for (int b2 = 0; b2 < 20; ++b2)
    {
        juce::MidiBuffer none;
        p.processBlock (block, none);
        peak = std::max (peak, block.getMagnitude (0, 0, 480));
    }
    printf ("live note on 101 (stopped): peak %.3f %s\n", peak, peak > 0.01f ? "ok" : "SILENT");
    juce::MidiBuffer off;
    off.addEvent (juce::MidiMessage::noteOff (1, 60), 0);
    p.processBlock (block, off);

    // Recording: play C, E, G on beats while the sequencer runs, then a pad on the 909.
    p.playing = true;
    p.setRecordNotes (true);
    const int stepSamples = 6000;   // 120 BPM, 16th notes at 48 kHz
    int sample = 0;
    auto runTo = [&] (int target, juce::MidiBuffer m)
    {
        while (sample < target)
        {
            p.processBlock (block, m);
            m.clear();
            sample += 480;
        }
    };
    runTo (stepSamples * 4 - 960, {});
    juce::MidiBuffer m1; m1.addEvent (juce::MidiMessage::noteOn (1, 64, (juce::uint8) 90), 0);
    runTo (stepSamples * 4 + 480, m1);
    juce::MidiBuffer m2; m2.addEvent (juce::MidiMessage::noteOff (1, 64), 0);
    runTo (stepSamples * 7 - 480, {});
    runTo (stepSamples * 7, m2);
    p.setMidiTarget (TR909);
    juce::MidiBuffer m3; m3.addEvent (juce::MidiMessage::noteOn (10, 38, (juce::uint8) 120), 0);
    runTo (stepSamples * 8 + 1920, m3);
    p.setRecordNotes (false);
    auto& t101 = p.machine (SH101).track;
    juce::String notes;
    for (int st = 0; st < 16; ++st) notes << (t101.on[(size_t) st] ? juce::String (t101.note[(size_t) st].load()) + (t101.slide[(size_t) st] ? "~" : " ") : juce::String (". "));
    printf ("recorded 101: %s\n", notes.toRawUTF8());
    juce::String hits;
    for (int st = 0; st < 16; ++st) hits << (p.machine (TR909).track.hits[1][(size_t) st] ? "x" : ".");
    printf ("recorded 909 snare: %s\n", hits.toRawUTF8());

    // MIDI learn: the next controller moved drives the parameter.
    p.midiLearn.start ("tr909_bd_level");
    juce::MidiBuffer cc; cc.addEvent (juce::MidiMessage::controllerEvent (1, 21, 0), 0);
    p.processBlock (block, cc);
    printf ("midi learn: CC %d -> level %.2f\n", p.midiLearn.controllerFor ("tr909_bd_level"), p.apvts.getRawParameterValue ("tr909_bd_level")->load());
    p.midiLearn.forget ("tr909_bd_level");
    return 0;
}

static int test_roll (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    Theme theme;
    juce::LookAndFeel::setDefaultLookAndFeel (&theme);
    TechMachineProcessor p;
    p.loadExample (5);
    // Juno: a long note (4 steps) and a short one (2 steps) starting together, then a new chord.
    auto& t = p.machine (Juno106).track;
    t.clear();
    const std::uint64_t a = 1ull << 9, c = 1ull << 12, e = 1ull << 16;
    t.chord[0] = a | c; t.tie[0] = a | c; t.on[0] = true;
    t.chord[1] = a | c; t.tie[1] = a; t.on[1] = true;
    t.chord[2] = a; t.tie[2] = a; t.on[2] = true;
    t.chord[3] = a; t.on[3] = true;
    t.chord[4] = e; t.on[4] = true;
    t.tile (p.numSteps());
    p.setShown (Juno106, true);
    print ("independent note lengths render", run (p, 3.0, "roll_render"));

    PianoRollEditor editor (p, Juno106);
    editor.setBounds (0, 0, 1580, 640);
    editor.getGrid().reload();
    editor.getGrid().fitAll();
    savePng (editor.createComponentSnapshot (editor.getLocalBounds(), true, 1.0f), "roll_editor_juno");

    PianoRollEditor mono (p, SH101);
    mono.setBounds (0, 0, 1580, 640);
    mono.getGrid().fitAll();
    savePng (mono.createComponentSnapshot (mono.getLocalBounds(), true, 1.0f), "roll_editor_101");

    auto file = outDir.getChildFile ("roll.techmachine");
    p.saveProject (file);
    TechMachineProcessor q;
    q.loadProject (file);
    printf ("ties saved: step 2 tie %llx (expected %llx)\n", (unsigned long long) q.machine (Juno106).track.tie[1].load(), (unsigned long long) a);
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    return 0;
}

static int test_rollmouse (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    Theme theme;
    juce::LookAndFeel::setDefaultLookAndFeel (&theme);
    TechMachineProcessor p;
    p.newProject();
    p.setShown (SH101, true);
    p.setShown (Juno106, true);
    p.setBars (2);
    PianoRollEditor editor (p, SH101);
    editor.setBounds (0, 0, 1580, 640);
    auto& grid = editor.getGrid();
    grid.fitAll();

    auto event = [&grid] (juce::Point<float> at, juce::ModifierKeys mods, int clicks = 1)
    {
        return juce::MouseEvent (juce::Desktop::getInstance().getMainMouseSource(), at, mods, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                 &grid, &grid, juce::Time::getCurrentTime(), at, juce::Time::getCurrentTime(), clicks, false);
    };
    const juce::ModifierKeys left (juce::ModifierKeys::leftButtonModifier), right (juce::ModifierKeys::rightButtonModifier);
    auto drag = [&] (int s0, int n0, int s1, int n1, juce::ModifierKeys mods)
    {
        grid.mouseDown (event (grid.cellCentre (s0, n0), mods));
        grid.mouseDrag (event (grid.cellCentre (s1, n1), mods));
        grid.mouseUp (event (grid.cellCentre (s1, n1), mods));
    };
    auto dump = [&p] (int type)
    {
        auto& t = p.machine (type).track;
        juce::String line;
        for (int s = 0; s < p.numSteps(); ++s)
            line << (t.on[(size_t) s] ? juce::String (t.note[(size_t) s].load()) + (t.slide[(size_t) s] ? "~" : " ") : juce::String (". "));
        return line;
    };

    drag (4, 7, 7, 7, left);
    printf ("draw + stretch:   %s\n", dump (SH101).toRawUTF8());
    drag (5, 7, 9, 10, left);
    printf ("move (+4, +3):    %s\n", dump (SH101).toRawUTF8());
    grid.mouseDown (event (grid.cellCentre (11, 10), left));
    grid.mouseDrag (event (grid.cellCentre (11, 10).translated (2.0f * 23.0f, 0.0f), left));
    grid.mouseUp (event (grid.cellCentre (13, 10), left));
    printf ("click on note:    %s\n", dump (SH101).toRawUTF8());
    drag (0, 12, 1, 12, left);
    grid.keyPressed (juce::KeyPress ('A', juce::ModifierKeys::commandModifier, 0));
    grid.keyPressed (juce::KeyPress ('D', juce::ModifierKeys::commandModifier, 0));
    printf ("select all + dup: %s\n", dump (SH101).toRawUTF8());
    grid.keyPressed (juce::KeyPress (juce::KeyPress::upKey, juce::ModifierKeys::shiftModifier, 0));
    printf ("octave up:        %s\n", dump (SH101).toRawUTF8());
    grid.mouseDown (event (grid.cellCentre (0, 12), right));
    grid.mouseUp (event (grid.cellCentre (0, 12), right));
    printf ("right-click erase:%s\n", dump (SH101).toRawUTF8());
    p.undoManager.undo();
    grid.reload();
    printf ("undo:             %s\n", dump (SH101).toRawUTF8());

    // Poly: two notes of different lengths on the same step, then a chord.
    grid.setMachine (Juno106);
    grid.fitAll();
    drag (0, 12, 7, 12, left);
    drag (0, 15, 1, 15, left);
    drag (0, 19, 3, 19, left);
    auto& t = p.machine (Juno106).track;
    juce::String masks;
    for (int s = 0; s < 9; ++s) masks << juce::String::toHexString ((juce::int64) t.chord[(size_t) s].load()) << "/" << juce::String::toHexString ((juce::int64) t.tie[(size_t) s].load()) << " ";
    printf ("poly chord/tie:   %s\n", masks.toRawUTF8());
    grid.clearAll();
    grid.stackSize = 3;
    drag (0, 12, 3, 12, left);
    juce::String chord;
    for (int n = 0; n < noteRange; ++n) if ((t.chord[0].load() >> n) & 1u) chord << n << " ";
    int length = 0;
    while (length < 16 && t.chord[(size_t) length].load() != 0) ++length;
    printf ("chord tool from 12: notes %s length %d\n", chord.toRawUTF8(), length);
    print ("poly render", run (p, 2.0));
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    return 0;
}

static int test_golden (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    // One line per case: examples, then every machine playing a short phrase with each of its sounds.
    juce::StringArray lines;
    for (int e = 0; e < (int) examples().size(); ++e)
    {
        TechMachineProcessor p;
        p.loadExample (e);
        lines.add ("example " + juce::String (e) + " " + juce::String::toHexString ((juce::int64) fingerprint (p, 6.0)));
    }
    for (int t = 0; t < numMachines; ++t)
    {
        const auto& def = machineDef (t);
        const auto& presets = presetsFor (t);
        for (int k = -1; k < (int) presets.size(); ++k)
        {
            TechMachineProcessor p;
            p.newProject();
            p.setShown (t, true);
            p.setBars (1);
            if (k >= 0) p.applyPreset (t, presets[(size_t) k]);
            auto& tr = p.machine (t).track;
            for (int st = 0; st < pageSteps; ++st)
            {
                if (def.kind == MachineKind::Drum)
                    for (size_t r = 0; r < def.rows.size(); ++r) tr.hits[r][(size_t) st] = ((st + (int) r) % 3) == 0;
                else if (st % 2 == 0) { tr.on[(size_t) st] = true; tr.note[(size_t) st] = (st * 5) % 24; tr.slide[(size_t) st] = st % 6 == 4; tr.accent[(size_t) st] = st % 8 == 0; }
            }
            tr.tile (pageSteps);
            lines.add (juce::String (def.id) + " " + juce::String (k) + " " + juce::String::toHexString ((juce::int64) fingerprint (p, 2.0)));
        }
    }
    const auto file = outDir.getChildFile (argc > 2 ? juce::String (argv[2]) : juce::String ("golden.txt"));
    file.replaceWithText (lines.joinIntoString ("\n") + "\n");
    printf ("%d fingerprints written to %s\n", lines.size(), file.getFullPathName().toRawUTF8());
    return 0;
}

static int test_sweep (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    // Every parameter of every machine at its extremes (and every value of every selector):
    // flags silence, NaN, a saturated output or a heavy CPU load.
    int problems = 0, cases = 0;
    auto check = [&] (int t, const juce::String& what, const std::function<void (TechMachineProcessor&)>& set, bool mayBeSilent)
    {
        const auto& def = machineDef (t);
        TechMachineProcessor p;
        p.newProject();
        p.setShown (t, true);
        p.setBars (1);
        set (p);
        auto& tr = p.machine (t).track;
        for (int st = 0; st < pageSteps; ++st)
        {
            if (def.kind == MachineKind::Drum)
                for (size_t r = 0; r < def.rows.size(); ++r) tr.hits[r][(size_t) st] = st % 4 == 0;
            else if (st % 4 == 0) { tr.on[(size_t) st] = true; tr.note[(size_t) st] = (st * 3) % 14; }
        }
        tr.tile (pageSteps);
        const auto st = run (p, 1.5);
        ++cases;
        juce::String issue;
        if (st.nans > 0) issue << "NaN ";
        if (! mayBeSilent && st.rmsDb < -60.0) issue << "silent ";
        if (st.rmsDb > -4.0) issue << "saturated ";
        if (st.cpu > 15.0) issue << "cpu " << juce::String (st.cpu, 1) << "% ";
        if (issue.isNotEmpty())
        {
            ++problems;
            printf ("  %-10s %-34s rms %6.1f dB  %s\n", def.id, what.toRawUTF8(), st.rmsDb, issue.toRawUTF8());
        }
    };

    for (int t = 0; t < numMachines; ++t)
    {
        if (argc > 2 && juce::String (machineDef (t).id) != argv[2]) continue;
        const auto& def = machineDef (t);
        if (t == Sampler) continue;   // no sound until a file is loaded
        for (const char* fx : { "drive", "crush", "filter", "echo", "reverb", "pan" })
            for (float v : { 0.0f, 1.0f })
                check (t, juce::String ("@") + fx + "=" + juce::String (v), [t, fx, v] (TechMachineProcessor& p) { p.setParam (pid::machine (t, fx), v); }, false);

        if (def.kind == MachineKind::Drum)
        {
            for (int r = 0; r < (int) def.rows.size(); ++r)
                for (const char* param : { "level", "tune", "decay", "tone", "drive", "filter", "echo", "reverb" })
                    for (float v : { 0.0f, 1.0f })
                        check (t, juce::String (def.rows[(size_t) r].id) + "." + param + "=" + juce::String (v),
                               [t, r, param, v, &def] (TechMachineProcessor& p)
                               {
                                   // Only this row plays, so a silent sound cannot hide behind the others.
                                   for (int other = 0; other < (int) def.rows.size(); ++other)
                                       if (other != r) p.setParam (pid::row (t, other, "mute"), 1.0f);
                                   p.setParam (pid::row (t, r, param), v);
                               },
                               juce::String (param) == "level" && v == 0.0f);
        }
        else
        {
            for (int i = 0; i < (int) def.params.size(); ++i)
            {
                const auto& param = def.params[(size_t) i];
                if (param.choices.empty())
                {
                    for (float v : { 0.0f, 1.0f })
                        check (t, juce::String (param.id) + "=" + juce::String (v), [t, i, v] (TechMachineProcessor& p) { p.setParam (pid::synth (t, i), v); }, false);
                }
                else
                {
                    for (int c = 0; c < (int) param.choices.size(); ++c)
                        check (t, juce::String (param.id) + "=" + juce::String (c), [t, i, c] (TechMachineProcessor& p) { p.setParam (pid::synth (t, i), (float) c); }, false);
                }
            }
        }
    }
    printf ("sweep: %d cases, %d with problems\n", cases, problems);
    return problems;
}

static int test_project (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    int failures = 0;
    auto expect = [&] (bool ok, const juce::String& what)
    {
        if (! ok) ++failures;
        printf ("  %-60s %s\n", what.toRawUTF8(), ok ? "ok" : "FAIL");
    };

    // 1. Every example sounds exactly the same after a save and a load.
    int same = 0;
    for (int e = 0; e < (int) examples().size(); ++e)
    {
        TechMachineProcessor a;
        a.loadExample (e);
        const auto file = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("tm_roundtrip.techmachine");
        a.saveProject (file);
        TechMachineProcessor b;
        b.loadProject (file);
        file.deleteFile();
        if (fingerprint (a, 4.0) == fingerprint (b, 4.0)) ++same;
        else printf ("    example %d differs after save + load\n", e);
    }
    expect (same == (int) examples().size(), "examples identical after save + load (" + juce::String (same) + ")");

    // 2. Sampler pads: loaded, saved with the project, read back.
    const auto wav = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("tm_test_loop.wav");
    {
        juce::AudioBuffer<float> sine (1, 48000);
        for (int i = 0; i < 48000; ++i) sine.setSample (0, i, 0.5f * std::sin ((float) i * 0.05f));
        wav.deleteFile();
        std::unique_ptr<juce::OutputStream> out = wav.createOutputStream();
        juce::WavAudioFormat format;
        auto writer = format.createWriterFor (out, juce::AudioFormatWriterOptions {}.withSampleRate (48000).withNumChannels (1).withBitsPerSample (16));
        writer->writeFromAudioSampleBuffer (sine, 0, 48000);
    }
    const auto projectFile = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("tm_pads.techmachine");
    {
        TechMachineProcessor a;
        a.newProject();
        expect (a.chopLoop (wav), "chop a loop");
        expect (a.samplePadName (7) == "tm_test_loop 8", "pad 8 named after its slice");
        a.saveProject (projectFile);
        TechMachineProcessor b;
        b.loadProject (projectFile);
        expect (b.samplePadName (3) == "tm_test_loop 4" && b.isShown (Sampler), "pads restored by loading the project");
        expect (fingerprint (a, 2.0) == fingerprint (b, 2.0), "sampler sounds the same after loading");
    }

    // 3. A project that has no sampler and fewer machines (older file) leaves nothing behind.
    {
        TechMachineProcessor p;
        p.newProject();
        p.loadSamplePad (0, wav);
        p.machine (Brass).track.on[0] = true;
        auto xml = p.createProjectXml();
        auto* patterns = xml->getChildByName ("Patterns");
        for (auto* e : patterns->getChildIterator())
            if (e->getStringAttribute ("id") == "sampler" || e->getStringAttribute ("id") == "brass") { patterns->removeChildElement (e, true); break; }
        for (auto* e : patterns->getChildIterator())
            if (e->getStringAttribute ("id") == "sampler" || e->getStringAttribute ("id") == "brass") { patterns->removeChildElement (e, true); break; }
        const auto old = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("tm_old.techmachine");
        xml->writeTo (old);
        TechMachineProcessor q;
        q.loadSamplePad (0, wav);
        q.machine (Brass).track.on[0] = true;
        q.loadProject (old);
        expect (q.samplePadName (0).isEmpty(), "older file: previous pads cleared");
        expect (! q.machine (Brass).track.on[0], "older file: machines it does not know start empty");
        old.deleteFile();
    }
    wav.deleteFile();
    projectFile.deleteFile();

    // 4. A chord played on the keyboard into a poly synth is written whole, held keys tied.
    {
        TechMachineProcessor p;
        p.newProject();
        p.setShown (Juno106, true);
        p.setBars (1);
        p.setParam (pid::bpm, 120.0f);
        p.setPlayConfigDetails (0, 2, 48000.0, 480);
        p.prepareToPlay (48000.0, 480);
        p.setMidiTarget (Juno106);
        juce::AudioBuffer<float> block (2, 480);
        p.playing = true;
        p.setRecordNotes (true);
        int sample = 0;
        auto runTo = [&] (int target, juce::MidiBuffer m)
        {
            while (sample < target) { p.processBlock (block, m); m.clear(); sample += 480; }
        };
        runTo (6000 * 4 - 960, {});
        const int base = machineBaseNote (machineDef (Juno106), p.currentKey(), 2);
        juce::MidiBuffer chord;
        for (int n : { 0, 4, 7 }) chord.addEvent (juce::MidiMessage::noteOn (1, base + n, (juce::uint8) 90), 0);
        runTo (6000 * 4, chord);
        juce::MidiBuffer release;
        for (int n : { 0, 4, 7 }) release.addEvent (juce::MidiMessage::noteOff (1, base + n), 0);
        runTo (6000 * 7, {});
        runTo (6000 * 7 + 480, release);
        p.setRecordNotes (false);
        auto& t = p.machine (Juno106).track;
        const std::uint64_t mask = (1ull << 0) | (1ull << 4) | (1ull << 7);
        printf ("    steps 4..7 chord/tie: %llx/%llx %llx/%llx %llx/%llx %llx/%llx\n",
                (unsigned long long) t.chord[4].load(), (unsigned long long) t.tie[4].load(), (unsigned long long) t.chord[5].load(), (unsigned long long) t.tie[5].load(),
                (unsigned long long) t.chord[6].load(), (unsigned long long) t.tie[6].load(), (unsigned long long) t.chord[7].load(), (unsigned long long) t.tie[7].load());
        expect (t.chord[4].load() == mask && t.tie[4].load() == mask && t.chord[6].load() == mask && t.tie[6].load() == 0 && ! t.on[7],
                "recorded chord kept whole, held for 3 steps");
    }
    printf ("project: %d failures\n", failures);
    return failures;
}

static int test_ui (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    Theme theme;
    juce::LookAndFeel::setDefaultLookAndFeel (&theme);
    juce::StringArray issues;
    for (auto lang : { Language::French, Language::English })
    {
        setLanguage (lang);
        const juce::String code = lang == Language::French ? "fr" : "en";
        for (int width : { 1440, 1600 })
        {
            const juce::String tag = code + "_" + juce::String (width);
            {
                TechMachineProcessor p;
                std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
                ed->setSize (width, 800);
                savePng (ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f), "ui_start_" + tag);
                auditLayout (*ed, theme, "start " + tag, issues);
            }
            {
                TechMachineProcessor p;
                p.loadExample (5);
                for (int t = 0; t < numMachines; ++t) p.setShown (t, true);
                std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
                ed->setSize (width, 800);
                hideStartScreen (*ed);
                ed->resized();
                savePng (ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f), "ui_rack_" + tag);
                auditLayout (*ed, theme, "rack " + tag, issues);

                juce::Array<MachinePanel*> panels;
                findPanels (*ed, panels);
                if (width == 1440)
                    for (auto* panel : panels)
                        if (panel->isVisible())
                            savePng (panel->createComponentSnapshot (panel->getLocalBounds(), true, 1.0f), "ui_panel_" + code + "_" + machineDef (panel->type).id);
                for (auto* panel : panels)
                    if (panel->type == Juno106 && panel->onOpenRoll) panel->onOpenRoll (Juno106);
                savePng (ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f), "ui_roll_" + tag);
                auditLayout (*ed, theme, "roll " + tag, issues);
                for (auto* panel : panels)
                    if (panel->type == TR909 && panel->onOpenAutomation) panel->onOpenAutomation (TR909);
                savePng (ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f), "ui_automation_" + tag);
                auditLayout (*ed, theme, "automation " + tag, issues);
                p.setSong ({ { 0, 1 }, { 1, 4 }, { 2, 2 }, { 1, 4 } });
                if (auto* view = findMainView (*ed))
                    for (auto* c : view->getChildren())
                        if (auto* b = dynamic_cast<techno::IconButton*> (c); b != nullptr && b->getButtonText() == utf8 ("MORCEAU") && b->onClick) b->onClick();
                pumpTimers();
                savePng (ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f), "ui_song_" + tag);
                auditLayout (*ed, theme, "song " + tag, issues);
            }
        }
    }
    setLanguage (Language::French);
    for (auto& i : issues) printf ("%s\n", i.toRawUTF8());
    printf ("ui: %d issues\n", issues.size());
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    return issues.size();
}

static int test_propagate (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    Theme theme;
    juce::LookAndFeel::setDefaultLookAndFeel (&theme);
    int failures = 0;
    auto expect = [&] (bool ok, const char* what)
    {
        if (! ok) ++failures;
        printf ("  %-66s %s\n", what, ok ? "ok" : "FAIL");
    };

    TechMachineProcessor p;
    p.newProject();
    p.setBars (1);
    for (int t : { TR909, TB303, Juno106 }) p.setShown (t, true);
    for (int st = 0; st < pageSteps; st += 4)
    {
        p.machine (Juno106).track.on[(size_t) st] = true;
        p.machine (Juno106).track.note[(size_t) st] = st / 2;
        p.machine (TB303).track.on[(size_t) st + 2] = true;
        p.machine (TB303).track.note[(size_t) st + 2] = st;
    }
    p.machine (Juno106).track.tile (pageSteps);
    p.machine (TB303).track.tile (pageSteps);
    p.touchPattern();

    std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
    ed->setSize (1600, 900);
    hideStartScreen (*ed);
    pumpTimers();
    juce::Array<MachinePanel*> panels;
    findPanels (*ed, panels);
    auto panelOf = [&] (int t) -> MachinePanel* { for (auto* x : panels) if (x->type == t) return x; return nullptr; };
    auto* juno = panelOf (Juno106);
    auto* acid = panelOf (TB303);

    auto changes = [&] (juce::Component& c, const std::function<void()>& action)
    {
        const auto before = imageHash (c);
        p.undoManager.beginNewTransaction();   // as every click in the interface does
        action();
        pumpTimers();
        return imageHash (c) != before;
    };

    expect (changes (*juno, [&] { p.setParam (pid::key, 5.0f); }), "key -> note names of the poly panel");
    expect (changes (*acid, [&] { p.setParam (pid::scale, 2.0f); }), "scale (minor -> major) -> note names of the mono panel");
    expect (changes (*acid, [&] { p.setParam (pid::synth (TB303, 0), 3.0f); }), "octave of the 303 -> its note names");
    expect (changes (*juno, [&] { p.setParam (pid::synth (Juno106, 1), 0.0f); }), "chord type 'single note' -> row label NOTES, names");
    expect (changes (*juno, [&] { p.setParam (pid::synth (Juno106, 1), 1.0f); }), "chord type back to chords -> row label ACCORDS");
    expect (changes (*juno, [&] { p.beginPatternEdit(); p.machine (Juno106).track.on[1] = true; p.touchPattern(); }), "steps changed by code -> panel");
    expect (changes (*juno, [&] { p.undoManager.undo(); }), "undo -> panel");
    expect (changes (*acid, [&] { p.setMidiTarget (TB303); }), "MIDI target -> badge on the 303");

    // Piano roll
    juno->onOpenRoll (Juno106);
    pumpTimers();
    auto* roll = findChild<PianoRollEditor> (*ed);
    expect (roll != nullptr && roll->isVisible(), "piano roll opens");
    if (roll != nullptr)
    {
        auto& grid = roll->getGrid();
        auto* machineList = findChild<juce::ComboBox> (*roll);
        auto listed = [&] (int t)
        {
            for (int i = 0; i < machineList->getNumItems(); ++i)
                if (machineList->getItemId (i) == t + 1) return true;
            return false;
        };
        expect (listed (Juno106) && listed (TB303) && ! listed (Brass) && ! listed (DX7), "roll lists only the synths of the rack");
        p.setShown (Brass, true);
        pumpTimers();
        expect (listed (Brass), "synth added to the rack -> listed in the roll");
        p.setShown (Brass, false);
        pumpTimers();
        expect (! listed (Brass), "synth removed from the rack -> gone from the roll");

        expect (changes (grid, [&] { p.setParam (pid::key, 2.0f); }), "key -> roll keyboard and rows");
        expect (changes (grid, [&] { p.setParam (pid::scale, 0.0f); }), "scale -> roll rows in / out of the scale");
        expect (changes (grid, [&] { p.setParam (pid::synth (Juno106, 0), 3.0f); }), "octave -> roll keyboard");
        expect (changes (grid, [&] { p.beginPatternEdit(); p.machine (Juno106).track.on[3] = true; p.touchPattern(); }), "notes changed elsewhere -> roll");
        expect (changes (grid, [&] { p.undoManager.undo(); }), "undo -> roll");
        expect (changes (grid, [&] { p.setBars (2); }), "2 bars -> roll shows 32 steps");
        expect (changes (grid, [&] { p.setBars (1); }), "back to 1 bar -> roll");
        expect (changes (grid, [&] { p.applyPreset (Juno106, presetsFor (Juno106)[1]); p.setParam (pid::synth (Juno106, 0), 1.0f); }), "preset + octave -> roll");

        p.setShown (Juno106, false);
        pumpTimers();
        expect (findChild<PianoRollEditor> (*ed) == nullptr, "machine removed from the rack -> roll closes");
    }

    p.setShown (Juno106, true);
    pumpTimers();
    panels.clear();
    findPanels (*ed, panels);
    panelOf (Juno106)->onOpenRoll (Juno106);
    pumpTimers();
    p.newProject();
    pumpTimers();
    expect (findChild<PianoRollEditor> (*ed) == nullptr, "new project -> roll closes");

    p.loadExample (5);
    pumpTimers();
    int visiblePanels = 0;
    panels.clear();
    findPanels (*ed, panels);
    for (auto* x : panels) visiblePanels += x->isVisible() && p.isShown (x->type) ? 1 : 0;
    int shown = 0;
    for (int t = 0; t < numMachines; ++t) shown += p.isShown (t) ? 1 : 0;
    expect (visiblePanels == shown, "loading an example -> the rack shows exactly its machines");

    // Sampler pads appear on the panel
    const auto wav = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("tm_prop.wav");
    {
        juce::AudioBuffer<float> sine (1, 4800);
        for (int i = 0; i < 4800; ++i) sine.setSample (0, i, 0.5f * std::sin ((float) i * 0.05f));
        std::unique_ptr<juce::OutputStream> out = wav.createOutputStream();
        juce::WavAudioFormat format;
        auto writer = format.createWriterFor (out, juce::AudioFormatWriterOptions {}.withSampleRate (48000).withNumChannels (1).withBitsPerSample (16));
        writer->writeFromAudioSampleBuffer (sine, 0, 4800);
    }
    p.setShown (Sampler, true);
    pumpTimers();
    panels.clear();
    findPanels (*ed, panels);
    expect (changes (*panelOf (Sampler), [&] { p.loadSamplePad (2, wav); }), "sample loaded -> pad name on the sampler");
    wav.deleteFile();

    ed.reset();
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    printf ("propagate: %d failures\n", failures);
    return failures;
}

static int test_chaos (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    // Random settings on every parameter of each synth at once: no NaN, no runaway level.
    juce::Random rnd (1234);
    int problems = 0, cases = 0;
    for (int t = 0; t < numMachines; ++t)
    {
        const auto& def = machineDef (t);
        if (def.kind == MachineKind::Drum) continue;
        if (argc > 2 && juce::String (def.id) != argv[2]) continue;
        for (int it = 0; it < chaosIterations; ++it)
        {
            TechMachineProcessor p;
            p.newProject();
            p.setShown (t, true);
            p.setBars (1);
            juce::String settings;
            for (int i = 0; i < (int) def.params.size(); ++i)
            {
                const auto& param = def.params[(size_t) i];
                const float v = param.choices.empty() ? rnd.nextFloat() : (float) rnd.nextInt ((int) param.choices.size());
                p.setParam (pid::synth (t, i), v);
                settings << param.id << "=" << juce::String (v, 2) << " ";
            }
            auto& tr = p.machine (t).track;
            for (int st = 0; st < pageSteps; ++st)
            {
                tr.on[(size_t) st] = st % 2 == 0;
                tr.note[(size_t) st] = (st * 5) % 24;
                tr.slide[(size_t) st] = st % 6 == 4;
                tr.accent[(size_t) st] = st % 4 == 0;
            }
            tr.tile (pageSteps);
            auto st = run (p, 1.5);
            // A busy computer (a shared CI machine) can slow down one run: too slow only if it is twice in a row.
            if (st.cpu > 20.0 && st.nans == 0) st.cpu = std::min (st.cpu, run (p, 1.5).cpu);
            ++cases;
            if (st.nans > 0 || st.rmsDb > -5.0 || st.cpu > 20.0)
            {
                ++problems;
                printf ("  %-10s rms %6.1f nan %d cpu %.1f  %s\n", def.id, st.rmsDb, st.nans, st.cpu, settings.toRawUTF8());
            }
        }
    }
    printf ("chaos: %d cases, %d with problems\n", cases, problems);
    return problems;
}

static int test_choices (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    // Level of every value of the sound selectors (model, type, table, wave...): they should stay close.
    for (int t = 0; t < numMachines; ++t)
    {
        const auto& def = machineDef (t);
        if (def.kind == MachineKind::Drum) continue;
        if (argc > 2 && juce::String (def.id) != argv[2]) continue;
        for (int i = 0; i < (int) def.params.size(); ++i)
        {
            const auto& param = def.params[(size_t) i];
            const juce::String id (param.id);
            if (param.choices.empty() || id == "octave" || id == "chord" || id == "arp" || id == "hpf" || id == "chorus") continue;
            juce::String line;
            double low = 0, high = -200;
            for (int c = 0; c < (int) param.choices.size(); ++c)
            {
                TechMachineProcessor p;
                p.newProject();
                p.setShown (t, true);
                p.setBars (1);
                p.setParam (pid::synth (t, i), (float) c);
                // Two notes held for half a bar each: long enough for slow attacks, still a hit for percussive sounds.
                auto& tr = p.machine (t).track;
                for (int st = 0; st < pageSteps; ++st)
                {
                    tr.on[(size_t) st] = true;
                    tr.note[(size_t) st] = st < 8 ? 0 : 7;
                    tr.slide[(size_t) st] = st % 8 != 7;
                }
                tr.tile (pageSteps);
                const double db = momentaryLoudness (p, 4.0);
                line << juce::String (db, 1) << " ";
                low = c == 0 ? db : std::min (low, db);
                high = std::max (high, db);
            }
            printf ("%-10s %-11s spread %5.1f dB : %s\n", def.id, param.id, high - low, line.toRawUTF8());
        }
    }
    return 0;
}

static int test_song (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    // "Blood Rave": an original dark acid techno track (132 BPM, F minor), 4-bar loop + an arranged 32-bar WAV.
    TechMachineProcessor p;
    p.newProject();
    auto set = [&] (const juce::String& id, float v) { if (p.apvts.getParameter (id) == nullptr) printf ("unknown parameter %s\n", id.toRawUTF8()); else p.setParam (id, v); };
    set ("bpm", 132); set ("swing", 0.0f); set ("key", 5); set ("scale", 1); p.setBars (4);
    set ("echoTime", 2); set ("echoFeedback", 0.4f); set ("echoLevel", 0.55f); set ("reverbSize", 0.55f); set ("reverbLevel", 0.35f);
    set ("comp", 0.35f); set ("pump", 0.15f);

    auto rowOf = [] (int type, const char* id) { const auto& d = machineDef (type); for (size_t i = 0; i < d.rows.size(); ++i) if (juce::String (d.rows[i].id) == id) return (int) i; return -1; };
    auto drums = [&] (int type, const char* rowId, juce::String pattern)
    {
        pattern = pattern.removeCharacters (" ");
        const int r = rowOf (type, rowId);
        auto& t = p.machine (type).track;
        for (int st = 0; st < maxSteps; ++st)
        {
            const auto c = pattern[st % pattern.length()];
            if (c == 'x' || c == 'X') t.hits[(size_t) r][(size_t) st] = true;
            if (c == 'X') t.accent[(size_t) st] = true;
        }
    };
    auto noteOf = [] (juce::String token)
    {
        static const char* names[] { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        const bool sharp = token.length() > 1 && token[1] == '#';
        const juce::String name = token.substring (0, sharp ? 2 : 1);
        int pc = 0;
        for (int i = 0; i < 12; ++i) if (name == names[i]) pc = i;
        return 12 * (token.substring (sharp ? 2 : 1).getIntValue() + 1) + pc;
    };
    auto line = [&] (int type, const juce::String& text)
    {
        auto tokens = juce::StringArray::fromTokens (text, " ", "");
        tokens.removeEmptyStrings();
        auto& t = p.machine (type).track;
        const auto& d = machineDef (type);
        const int octave = d.params.empty() ? 2 : (int) p.apvts.getRawParameterValue (pid::synth (type, 0))->load();
        const int base = machineBaseNote (d, p.currentKey(), octave);
        for (int st = 0; st < maxSteps; ++st)
        {
            const auto token = tokens[st % tokens.size()];
            if (token == ".") continue;
            const bool accent = token.containsChar ('!'), glide = token.containsChar ('~');
            int offset = noteOf (token.removeCharacters ("!~")) - base;
            while (offset < 0) offset += 12;
            while (offset >= noteRange) offset -= 12;
            t.on[(size_t) st] = true;
            t.note[(size_t) st] = offset;
            t.accent[(size_t) st] = accent;
            t.slide[(size_t) st] = glide;
        }
    };

    // 909: four on the floor with a little grit, rolling hats, ride in the second half, a fill into the loop.
    p.setShown (TR909, true);
    set ("tr909_bd_decay", 0.55f); set ("tr909_bd_tone", 0.65f); set ("tr909_bd_drive", 0.3f); set ("tr909_bd_level", 0.95f);
    set ("tr909_ch_level", 0.32f); set ("tr909_ch_decay", 0.22f); set ("tr909_oh_level", 0.42f); set ("tr909_oh_decay", 0.4f);
    set ("tr909_cp_level", 0.55f); set ("tr909_cp_reverb", 0.25f); set ("tr909_sd_level", 0.5f); set ("tr909_rd_level", 0.3f);
    set ("tr909_cr_level", 0.35f); set ("tr909_cr_reverb", 0.2f);
    drums (TR909, "bd", "X...x...x...x...");
    drums (TR909, "cp", "....x.......x...");
    drums (TR909, "ch", "xxx.xxx.xxx.xxx.");
    drums (TR909, "oh", "..x...x...x...x.");
    drums (TR909, "rd", "................................" "x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.");
    drums (TR909, "sd", "................................................" "..........x.xxxx");
    drums (TR909, "cr", "x...............................................................");

    // 303: a rolling sixteenth line on F, octave jumps, the minor sixth and seventh, accents and glides.
    p.setShown (TB303, true);
    set ("tb303_octave", 2); set ("tb303_wave", 0); set ("tb303_cutoff", 0.4f); set ("tb303_reso", 0.86f); set ("tb303_env", 0.66f);
    set ("tb303_decay", 0.4f); set ("tb303_accent", 0.85f); set ("tb303_drive", 0.5f); set ("tb303_echo", 0.15f); set ("tb303_volume", 0.72f);
    const juce::String a1 = "F1! F1 F2~ F1 F1 G#1 F1 F2! F1 F1 D#2~ F1 C2 F1 F2! C#2 ";
    const juce::String a2 = "F1! F1 F2~ F1 F1 G#1 F1 F2! F1 C2 F1 D#2~ F2 F1 G#1! C2 ";
    const juce::String a3 = "F1! F1 F2~ F1 C#2 F1 D#2 F1 F2! F1 G#2~ F2 D#2! C2 A#1 G#1~ ";
    line (TB303, a1 + a2 + a1 + a3);

    // Dark driven stabs on the downbeats, thrown into the echo.
    p.setShown (Poly80, true);
    set ("poly80_model", 5); set ("poly80_chord", 1); set ("poly80_octave", 2); set ("poly80_cutoff", 0.45f); set ("poly80_reso", 0.25f);
    set ("poly80_env", 0.5f); set ("poly80_attack", 0.0f); set ("poly80_decay", 0.22f); set ("poly80_sustain", 0.0f); set ("poly80_release", 0.25f);
    set ("poly80_echo", 0.5f); set ("poly80_reverb", 0.3f); set ("poly80_volume", 0.6f);
    line (Poly80, juce::String ("F3 . . . . . . . . . . . . . . . ") + ". . . . . . . . . . F3 . . . . . "
                              + "C#3 . . . . . . . . . . . . . . . " + ". . . . . . . . . . D#3 . . . . . ");

    // A riser into the next loop and an impact on its first beat.
    p.setShown (SoundFx, true);
    set ("fx_rise_level", 0.45f); set ("fx_imp_level", 0.5f); set ("fx_rise_decay", 0.45f);
    drums (SoundFx, "imp", "x...............................................................");
    drums (SoundFx, "rise", "................................................x...............");

    p.touchPattern();
    const auto folder = juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("Tech Machine");
    const auto projectFile = TechMachineProcessor::projectsFolder().getChildFile ("Blood Rave (acid).techmachine");
    p.saveProject (projectFile);
    printf ("project: %s\n", projectFile.getFullPathName().toRawUTF8());

    // Arranged render: 8 passes of the 4-bar loop with the 303 filter opening, a breakdown and the drop.
    const double rate = 48000.0;
    const int bs = 480;
    p.setPlayConfigDetails (0, 2, rate, bs);
    p.prepareToPlay (rate, bs);
    p.playing = true;
    const double loopSamples = 4.0 * 16.0 * (60.0 / 132.0 / 4.0) * rate;
    const int total = (int) (loopSamples * 8.0 + 3.0 * rate);
    const auto wavFile = folder.getChildFile ("Blood Rave (acid).wav");
    wavFile.deleteFile();
    std::unique_ptr<juce::OutputStream> out = wavFile.createOutputStream();
    juce::WavAudioFormat wav;
    auto writer = wav.createWriterFor (out, juce::AudioFormatWriterOptions {}.withSampleRate (rate).withNumChannels (2).withBitsPerSample (24));
    juce::AudioBuffer<float> block (2, bs);
    juce::MidiBuffer midi;
    double sectionSum[9] {}; int sectionCount[9] {}; float peak = 0;
    for (int pos = 0; pos < total; pos += bs)
    {
        const double loops = (double) pos / loopSamples;
        const int section = juce::jmin (8, (int) loops);
        const double inLoop = loops - std::floor (loops);
        float cutoff = 0.4f;
        bool acidOn = true, kickOn = true, stabOn = false, fxOn = true;
        switch (section)
        {
            case 0: acidOn = false; fxOn = false; break;
            case 1: cutoff = 0.12f; break;
            case 2: case 3: case 4: cutoff = (float) (0.12 + 0.43 * ((loops - 2.0) / 3.0)); break;
            case 5: cutoff = 0.6f; kickOn = false; stabOn = true; break;
            case 6: case 7: cutoff = (float) (0.45 + 0.08 * std::sin (inLoop * juce::MathConstants<double>::twoPi)); stabOn = true; break;
            default: acidOn = kickOn = stabOn = fxOn = false; p.playing = false; break;
        }
        set ("tb303_cutoff", cutoff);
        set ("tb303_mute", acidOn ? 0.0f : 1.0f);
        set ("tr909_bd_mute", kickOn ? 0.0f : 1.0f);
        set ("poly80_mute", stabOn ? 0.0f : 1.0f);
        set ("fx_mute", fxOn ? 0.0f : 1.0f);
        p.processBlock (block, midi);
        writer->writeFromAudioSampleBuffer (block, 0, bs);
        for (int i = 0; i < bs; ++i)
        {
            const float l = block.getSample (0, i), r = block.getSample (1, i);
            sectionSum[section] += 0.5 * (l * l + r * r);
            peak = std::max (peak, std::max (std::abs (l), std::abs (r)));
        }
        sectionCount[section] += bs;
    }
    writer.reset();
    printf ("wav: %s (%.1f s)\n", wavFile.getFullPathName().toRawUTF8(), total / rate);
    for (int k = 0; k < 9; ++k)
        if (sectionCount[k] > 0) printf ("  section %d: rms %6.1f dB\n", k + 1, 10.0 * std::log10 (std::max (1.0e-12, sectionSum[k] / sectionCount[k])));
    printf ("  peak %.3f\n", peak);
    return 0;
}

static int test_blow (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    const int machine = argc > 2 ? juce::String (argv[2]).getIntValue() : TR909;
    const auto& def = machineDef (machine);
    for (int r = -1; r < (int) def.rows.size(); ++r)
    {
        int blown = 0;
        float worst = 0;
        for (int it = 0; it < 150; ++it)
        {
            TechMachineProcessor p;
            p.newProject();
            p.setShown (machine, true);
            p.setBars (1);
            auto& t = p.machine (machine).track;
            for (int s = 0; s < maxSteps; ++s)
                for (int row = 0; row < (int) def.rows.size(); ++row)
                    if (r < 0 ? (s % 2 == 0) : row == r) t.hits[(size_t) row][(size_t) s] = r < 0 ? (s + row) % 3 == 0 : s % 2 == 0;
            const auto st = run (p, 1.5);
            worst = std::max (worst, st.peak);
            if (st.peak > 1.5f || st.nans > 0) ++blown;
        }
        printf ("row %-4s blown %d/150 worst %.3f\n", r < 0 ? "all" : def.rows[(size_t) r].id, blown, worst);
    }
    return 0;
}

static int test_params (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    struct Checker : ExampleWriter
    {
        TechMachineProcessor& p;
        int missing = 0;
        explicit Checker (TechMachineProcessor& proc) : p (proc) {}
        void setParam (const juce::String& id, float value) override
        {
            if (p.apvts.getParameter (id) == nullptr) { printf ("  missing param: %s\n", id.toRawUTF8()); ++missing; }
            else p.setParam (id, value);
        }
        void show (int type) override { p.setShown (type, true); }
        Track& track (int type) override { return p.machine (type).track; }
        int baseNote (int) override { return 36; }
    };
    TechMachineProcessor p;
    Checker c (p);
    for (int e = 0; e < (int) examples().size(); ++e) buildExample (e, c);
    printf ("example params missing: %d\n", c.missing);

    int badKeys = 0, presetCount = 0;
    for (int t = 0; t < numMachines; ++t)
        for (auto& preset : presetsFor (t))
        {
            ++presetCount;
            for (auto& kv : preset.values)
                if (p.presetParameter (t, kv.key) == nullptr) { ++badKeys; printf ("  preset %s / %s: unknown key %s\n", machineDef (t).id, preset.name, kv.key); }
        }
    printf ("presets: %d, unknown keys: %d\n", presetCount, badKeys);
    int badBand = 0;
    for (auto& st : styles())
        for (auto& m : defaultBand (st))
        {
            if (m.preset == nullptr) continue;
            bool found = false;
            for (auto& pr : presetsFor (m.machine)) found = found || std::strcmp (pr.name, m.preset) == 0;
            if (! found) { ++badBand; printf ("  band %s: no preset %s on %s\n", st.name, m.preset, machineDef (m.machine).id); }
        }
    printf ("band presets missing: %d\n", badBand);

    juce::StringArray ids;
    int duplicates = 0;
    for (auto* param : p.getParameters())
        if (auto* withId = dynamic_cast<juce::AudioProcessorParameterWithID*> (param))
        {
            if (ids.contains (withId->paramID)) { ++duplicates; printf ("  duplicate id %s\n", withId->paramID.toRawUTF8()); }
            ids.add (withId->paramID);
        }
    printf ("parameters: %d, duplicate ids: %d\n", ids.size(), duplicates);
    return c.missing + badKeys + badBand + duplicates;
}

static int test_patterns (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    int count = 0, bad = 0;
    for (auto& pat : patterns())
    {
        for (int t = 0; t < numMachines; ++t)
        {
            const auto kinds = patternKindsFor (machineDef (t));
            if (std::find (kinds.begin(), kinds.end(), pat.kind) == kinds.end()) continue;
            if (pat.kind != PatternKind::Drums && t != Sub808 && t != DX7 && t != TB303) continue;
            if (pat.kind == PatternKind::Drums && t != TR909 && t != Percussion && t != BreakKit) continue;
            if (patternCoverage (machineDef (t), pat) < 0.75f) continue;   // hidden from the menu on this kit
            TechMachineProcessor p;
            p.newProject();
            p.setShown (t, true);
            p.setBars (1);
            p.applyPattern (t, pat);
            const bool empty = p.machine (t).track.isEmpty();
            const auto st = run (p, 2.0);
            ++count;
            if (st.nans > 0 || st.peak > 1.0f || (empty && t != Percussion))
            {
                ++bad;
                print (juce::String ("BAD ") + utf8 (pat.name) + " on " + machineDef (t).id, st);
            }
        }
    }
    printf ("patterns applied: %d, problems: %d\n", count, bad);
    TechMachineProcessor p;
    p.newProject();
    p.setShown (TR909, true);
    for (auto& pat : patterns())
        if (juce::String (pat.name) == "Funky Drummer") p.applyPattern (TR909, pat);
    dump (p, TR909);
    return 0;
}

static int test_lang (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    const auto previous = language();
    setLanguage (Language::English);
    for (int t = 0; t < numMachines; ++t)
    {
        const auto& d = machineDef (t);
        utf8 (d.name); utf8 (d.subtitle); utf8 (d.help);
        for (auto& r : d.rows) { utf8 (r.label); utf8 (r.help); utf8 (r.toneHelp); }
        for (auto& prm : d.params) { utf8 (prm.label); utf8 (prm.help); for (auto* c : prm.choices) utf8 (c); }
    }
    for (auto& st : styles()) { utf8 (st.name); utf8 (st.help); }
    for (auto& e : examples()) { utf8 (e.name); utf8 (e.genre); utf8 (e.description); }
    for (auto& pat : patterns()) { utf8 (pat.category); utf8 (pat.name); }
    {
        Theme theme;
        juce::LookAndFeel::setDefaultLookAndFeel (&theme);
        TechMachineProcessor p;
        for (int t = 0; t < numMachines; ++t) p.setShown (t, true);
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        ed->setSize (1600, 900);
        savePng (ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f), "start_screen_en");
        p.loadExample (11);
        hideStartScreen (*ed);
        savePng (ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f), "editor_en");
        juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    }
    const auto missing = missingTranslations();
    auto f = outDir.getChildFile ("missing.txt");
    f.replaceWithText (missing.joinIntoString ("\n=====\n"));
    printf ("missing translations: %d (see missing.txt)\n", missing.size());
    setLanguage (previous);
    return 0;
}

static int test_genres (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    for (int g = 0; g < (int) styles().size(); ++g)
    {
        TechMachineProcessor p;
        p.newProject();
        for (int t = 0; t < numMachines; ++t) p.setShown (t, true);
        p.setBars (4);
        p.setGenre (g);
        p.generateAll();
        print (juce::String ("genre ") + styles()[(size_t) g].name, run (p, 10.0, juce::String ("genre_") + juce::String (g)));
        if (g == 0 || g == 16 || g == 17 || g == 21 || g == 25 || g == 26)
            for (int t : { TR909, MPC, Percussion, Sub808, ModelD, DX7, Organ }) dump (p, t);
    }
    return 0;
}

static int test_tools (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    TechMachineProcessor p;
    p.loadExample (0);
    const auto before = p.createProjectXml()->toString();
    p.generate (TR909, GenerateMode::NewIdea);
    p.generate (ModelD, GenerateMode::Vary);
    p.shiftMachine (Juno106, 1);
    p.undoManager.undo();
    p.undoManager.undo();
    p.undoManager.undo();
    printf ("undo x3 restores project: %s\n", p.createProjectXml()->toString() == before ? "yes" : "NO");
    p.undoManager.redo();
    printf ("redo changes it again: %s\n", p.createProjectXml()->toString() != before ? "yes" : "NO");

    auto file = outDir.getChildFile ("roundtrip.techmachine");
    p.saveProject (file);
    TechMachineProcessor q;
    q.loadProject (file);
    auto a = p.createProjectXml(), b = q.createProjectXml();
    const auto sa = a->toString(), sb = b->toString();
    printf ("save/load roundtrip identical: %s\n", sa == sb ? "yes" : "NO");
    if (sa != sb)
    {
        int i = 0;
        while (i < sa.length() && sa[i] == sb[i]) ++i;
        printf ("  first difference: ...%s\n  vs ...%s\n", sa.substring (i - 80, i + 60).toRawUTF8(), sb.substring (i - 80, i + 60).toRawUTF8());
    }

    auto wav = outDir.getChildFile ("export.wav");
    const bool ok = TechMachineProcessor::exportWav (*p.createProjectXml(), wav, 2, {});
    printf ("export wav: %s (%lld bytes)\n", ok ? "ok" : "FAILED", (long long) wav.getSize());

    TechMachineProcessor empty;
    empty.newProject();
    empty.setShown (TR909, true);
    printf ("new machine empty: %s\n", empty.machine (TR909).track.isEmpty() ? "yes" : "NO");
    return 0;
}

static int test_snap (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    Theme theme;
    juce::LookAndFeel::setDefaultLookAndFeel (&theme);
    {
        TechMachineProcessor p;
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        ed->setSize (1600, 900);
        savePng (ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f), "start_screen");
    }
    {
        TechMachineProcessor p;
        for (int t = 0; t < numMachines; ++t) p.setShown (t, true);
        p.loadExample (5);
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        ed->setSize (1600, 900);
        hideStartScreen (*ed);
        savePng (ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f), "editor_example");
    }
    {
        TechMachineProcessor p;
        p.loadExample (0);
        int page = 0;
        for (int t = 0; t < numMachines; ++t)
        {
            MachinePanel panel (p, t, page);
            panel.setBounds (0, 0, 1490, panel.preferredHeight (1490));
            savePng (panel.createComponentSnapshot (panel.getLocalBounds(), true, 1.0f), juce::String ("panel_") + machineDef (t).id);
        }
    }
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    return 0;
}


// Automation: values per step replace the knobs, in the strip, the synths and the drum sounds; they are saved,
// kept by HASARD, recorded from the knobs, drawn with the mouse, shown on the panel and undone.
static int test_automation (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    int failures = 0;
    auto expect = [&] (bool ok, const char* what)
    {
        if (! ok) ++failures;
        printf ("  %-66s %s\n", what, ok ? "ok" : "FAIL");
    };
    auto acid = [] (TechMachineProcessor& p)
    {
        p.newProject();
        p.setShown (TB303, true);
        p.setBars (1);
        auto& t = p.machine (TB303).track;
        for (int st = 0; st < pageSteps; ++st) { t.on[(size_t) st] = true; t.note[(size_t) st] = st % 5; }
        t.tile (pageSteps);
        p.setParam ("tb303_cutoff", 0.1f);
    };
    const int cutoff = automation::synthTarget (p303::Cutoff);

    TechMachineProcessor plain, locked;
    acid (plain);
    acid (locked);
    for (int st = 0; st < maxSteps; ++st) locked.setAutomation (TB303, cutoff, st, 0.9f);
    // Brightness: level of the sample-to-sample differences against the level of the signal.
    auto brightness = [] (TechMachineProcessor& p)
    {
        p.setPlayConfigDetails (0, 2, 48000.0, 512);
        p.prepareToPlay (48000.0, 512);
        p.playing = true;
        juce::AudioBuffer<float> buffer (2, 512);
        juce::MidiBuffer none;
        double level = 0, edges = 0;
        float previous = 0;
        for (int i = 0; i < 140; ++i)
        {
            p.processBlock (buffer, none);
            for (int n = 0; n < 512; ++n)
            {
                const float x = buffer.getSample (0, n);
                level += x * x;
                edges += (x - previous) * (x - previous);
                previous = x;
            }
        }
        return std::sqrt (edges / std::max (level, 1.0e-12));
    };
    const double closed = brightness (plain), open = brightness (locked);
    printf ("    brightness closed %.3f open %.3f\n", closed, open);
    expect (open > closed * 1.5, "303 filter automated open: much brighter than the closed knob");

    TechMachineProcessor muted;
    acid (muted);
    for (int st = 0; st < maxSteps; ++st) muted.setAutomation (TB303, automation::Volume, st, 0.0f);
    expect (run (muted, 1.5).rmsDb < -60.0, "strip volume automated to 0: silence");

    TechMachineProcessor kick, longKick;
    for (auto* p : { &kick, &longKick })
    {
        p->newProject();
        p->setShown (TR909, true);
        p->setBars (1);
        for (int st = 0; st < maxSteps; st += 4) p->machine (TR909).track.hits[0][(size_t) st] = true;
        p->setParam ("tr909_bd_decay", 0.1f);
    }
    for (int st = 0; st < maxSteps; ++st) longKick.setAutomation (TR909, automation::rowTarget (machineDef (TR909), 0, automation::Decay), st, 1.0f);
    expect (run (longKick, 1.5).rmsDb > run (kick, 1.5).rmsDb + 2.0, "909 kick length automated: longer kick");

    // Half of the steps automated: the knob comes back on the others.
    TechMachineProcessor half;
    acid (half);
    for (int st = 0; st < maxSteps; st += 2) half.setAutomation (TB303, cutoff, st, 0.9f);
    const double halfBright = brightness (half);
    expect (halfBright > closed * 1.15 && halfBright < open * 0.95, "steps without a value follow the knob");

    // After the sequencer stops, the knobs rule again.
    locked.playing = false;
    juce::AudioBuffer<float> block (2, 512);
    juce::MidiBuffer midi;
    locked.processBlock (block, midi);
    expect (juce::exactlyEqual (locked.machine (TB303).setting (cutoff, 0.1f), 0.1f), "stopped: back to the knob");

    // Saved with the project, read back
    const auto file = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("tm_automation.techmachine");
    {
        TechMachineProcessor p;
        acid (p);
        for (int st = 0; st < 16; ++st) p.setAutomation (TB303, cutoff, st, (float) st / 15.0f);
        p.saveProject (file);
        TechMachineProcessor q;
        q.loadProject (file);
        const int lane = q.machine (TB303).track.laneFor (cutoff, false);
        expect (lane >= 0 && std::abs (q.machine (TB303).track.lockValue[(size_t) lane][15].load() - 1.0f) < 0.001f
                && q.machine (TB303).track.lockValue[(size_t) lane][20].load() < 0.0f,
                "automation saved and read back");
        float worst = 0;
        const int pl = p.machine (TB303).track.laneFor (cutoff, false), ql = q.machine (TB303).track.laneFor (cutoff, false);
        for (int st = 0; st < maxSteps && pl >= 0 && ql >= 0; ++st)
            worst = std::max (worst, std::abs (p.machine (TB303).track.lockValue[(size_t) pl][(size_t) st].load()
                                               - q.machine (TB303).track.lockValue[(size_t) ql][(size_t) st].load()));
        printf ("    largest difference after save + load: %g\n", (double) worst);
        expect (fingerprint (p, 2.0) == fingerprint (q, 2.0), "same sound after save + load");
        file.deleteFile();
    }

    // HASARD writes new notes but keeps the automation; undo removes a drawing.
    {
        TechMachineProcessor p;
        acid (p);
        p.setAutomation (TB303, cutoff, 3, 0.7f);
        p.generate (TB303, GenerateMode::NewIdea);
        expect (p.machine (TB303).track.laneFor (cutoff, false) >= 0, "HASARD keeps the automation");
        p.undoManager.beginNewTransaction();
        p.beginPatternEdit();
        p.setAutomation (TB303, automation::Echo, 5, 0.5f);
        p.undoManager.undo();
        expect (p.machine (TB303).track.laneFor (automation::Echo, false) < 0, "undo removes the automation drawn");
    }

    // Recording: knob moves while playing land in the step being played.
    {
        TechMachineProcessor p;
        acid (p);
        p.setPlayConfigDetails (0, 2, 48000.0, 480);
        p.prepareToPlay (48000.0, 480);
        p.playing = true;
        p.setRecordAutomation (true);
        juce::AudioBuffer<float> b480 (2, 480);
        for (int i = 0; i < 20; ++i) p.processBlock (b480, midi);
        const int step = p.currentStep.load();
        p.recordKnob ("tb303_reso", 0.95f);
        p.setRecordAutomation (false);
        p.recordKnob ("tb303_env", 0.2f);
        auto& t = p.machine (TB303).track;
        const int lane = t.laneFor (automation::synthTarget (p303::Reso), false);
        expect (step >= 0 && lane >= 0 && std::abs (t.lockValue[(size_t) lane][(size_t) step].load() - 0.95f) < 0.001f,
                "recording: the knob move is written into the current step");
        expect (t.laneFor (automation::synthTarget (p303::EnvMod), false) < 0, "not recording: knob moves write nothing");
    }

    // Drawing with the mouse, erasing with the right button; the panel button lights up.
    {
        Theme theme;
        juce::LookAndFeel::setDefaultLookAndFeel (&theme);
        TechMachineProcessor p;
        acid (p);
        AutomationEditor editor (p, TB303);
        editor.setBounds (0, 0, 1300, 600);
        editor.selectTarget (cutoff);
        auto event = [&editor] (juce::Point<int> at, juce::ModifierKeys mods)
        {
            const auto f = at.toFloat();
            return juce::MouseEvent (juce::Desktop::getInstance().getMainMouseSource(), f, mods, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                     &editor, &editor, juce::Time::getCurrentTime(), f, juce::Time::getCurrentTime(), 1, false);
        };
        const juce::ModifierKeys left (juce::ModifierKeys::leftButtonModifier), right (juce::ModifierKeys::rightButtonModifier);
        editor.mouseDown (event (editor.pointFor (0, 0.2f), left));
        editor.mouseDrag (event (editor.pointFor (15, 0.8f), left));
        editor.mouseUp (event (editor.pointFor (15, 0.8f), left));
        auto& t = p.machine (TB303).track;
        const int lane = t.laneFor (cutoff, false);
        bool rising = lane >= 0;
        for (int st = 1; st < 16 && rising; ++st)
            rising = t.lockValue[(size_t) lane][(size_t) st].load() >= t.lockValue[(size_t) lane][(size_t) st - 1].load();
        expect (rising && t.lockValue[(size_t) lane][0].load() < 0.3f && t.lockValue[(size_t) lane][15].load() > 0.7f,
                "a mouse drag draws a rising ramp over every step it crosses");
        editor.mouseDown (event (editor.pointFor (3, 0.5f), right));
        editor.mouseUp (event (editor.pointFor (3, 0.5f), right));
        expect (t.lockValue[(size_t) lane][3].load() < 0.0f, "right click erases a step");

        int page = 0;
        MachinePanel panel (p, TB303, page);
        panel.refreshCells();
        juce::TextButton* automationButton = nullptr;
        for (auto* c : panel.getChildren())
            if (auto* b = dynamic_cast<juce::TextButton*> (c); b != nullptr && b->getButtonText() == "AUTOMATION") automationButton = b;
        expect (automationButton != nullptr && automationButton->getToggleState(), "AUTOMATION button lit on a machine with automation");
        juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    }

    printf ("automation: %d failures\n", failures);
    return failures;
}


// Song: parts hold their own patterns, switch on time, follow the song list, are saved, copied, cleared and undone.
static int test_parts (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    int failures = 0;
    auto expect = [&] (bool ok, const char* what)
    {
        if (! ok) ++failures;
        printf ("  %-66s %s\n", what, ok ? "ok" : "FAIL");
    };
    auto setUp = [] (TechMachineProcessor& p)
    {
        p.newProject();
        p.setShown (TR909, true);
        p.setShown (TB303, true);
        p.setBars (1);
        p.setParam (pid::bpm, 120.0f);
        // Part A: kick only. Part B: the 303 only.
        for (int st = 0; st < maxSteps; st += 4) p.machine (TR909).track.hits[0][(size_t) st] = true;
        p.selectPart (1);
        for (int st = 0; st < maxSteps; ++st) { p.machine (TB303).track.on[(size_t) st] = true; p.machine (TB303).track.note[(size_t) st] = st % 3; }
        p.selectPart (0);
    };

    TechMachineProcessor p;
    setUp (p);
    expect (p.currentPart() == 0 && p.machine (TR909).track.hits[0][0] && ! p.machine (TB303).track.on[0], "part A shows its own notes");
    p.selectPart (1);
    expect (p.machine (TB303).track.on[0] && ! p.machine (TR909).track.hits[0][0], "part B shows its own notes");
    p.selectPart (0);
    expect (! p.isPartEmpty (1) && p.isPartEmpty (2) && ! p.isProjectEmpty(), "empty and filled parts are told apart");

    // Switching while playing waits for the end of the loop.
    p.setPlayConfigDetails (0, 2, 48000.0, 480);
    p.prepareToPlay (48000.0, 480);
    p.playing = true;
    juce::AudioBuffer<float> block (2, 480);
    juce::MidiBuffer midi;
    for (int i = 0; i < 10; ++i) p.processBlock (block, midi);   // 4800 samples: step 0 of 16 (6000 samples per step)
    p.selectPart (1);
    expect (p.currentPart() == 0 && p.cuedPartIndex() == 1, "while playing, the new part waits");
    int switchedAt = -1;
    for (int i = 10; i < 220 && switchedAt < 0; ++i)
    {
        p.processBlock (block, midi);
        if (p.currentPart() == 1) switchedAt = (i + 1) * 480;
    }
    expect (switchedAt > 16 * 6000 && switchedAt <= 16 * 6000 + 480, "it starts exactly at the end of the loop");

    // Song: A x1, B x2, then back to A.
    {
        TechMachineProcessor q;
        setUp (q);
        q.setSong ({ { 0, 1 }, { 1, 2 } });
        q.setSongMode (true);
        q.setPlayConfigDetails (0, 2, 48000.0, 480);
        q.prepareToPlay (48000.0, 480);
        q.playing = true;
        juce::String order;
        int lastLoop = -1;
        for (int i = 0; i < 4 * 200 + 20; ++i)
        {
            q.processBlock (block, midi);
            const int loop = (i * 480) / (16 * 6000);
            if (loop != lastLoop && (i * 480) % (16 * 6000) > 960) { order << partName (q.currentPart()); lastLoop = loop; }
        }
        printf ("    loops played: %s\n", order.toRawUTF8());
        expect (order.startsWith ("ABBA"), "the song plays A, B, B, then starts again");
        expect (std::abs (q.songSeconds() - 3.0 * 16 * 15.0 / 120.0) < 0.001, "song length = 3 loops");
    }

    // Saved and read back with the song
    const auto file = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("tm_song.techmachine");
    {
        TechMachineProcessor a;
        setUp (a);
        a.setSong ({ { 1, 3 }, { 0, 1 } });
        a.setSongMode (true);
        a.saveProject (file);
        TechMachineProcessor b;
        b.loadProject (file);
        expect (b.song().size() == 2 && b.song()[0].part == 1 && b.song()[0].repeats == 3 && b.isSongMode(), "song saved and read back");
        b.selectPart (1);
        expect (b.machine (TB303).track.on[0] && ! b.machine (TR909).track.hits[0][0], "part B saved and read back");
        a.selectPart (0);
        b.selectPart (0);
        expect (fingerprint (a, 8.0) == fingerprint (b, 8.0), "same song after save + load");
        file.deleteFile();
    }

    // Copy, clear, undo in another part
    {
        TechMachineProcessor q;
        setUp (q);
        q.copyPart (1, 2);
        expect (! q.isPartEmpty (2), "copy B into C");
        q.clearPart (2);
        expect (q.isPartEmpty (2), "clear C");
        q.undoManager.beginNewTransaction();
        q.beginPatternEdit();
        q.machine (TR909).track.hits[1][0] = true;   // a snare in A
        q.selectPart (1);
        q.undoManager.undo();
        q.selectPart (0);
        expect (! q.machine (TR909).track.hits[1][0] && q.machine (TR909).track.hits[0][0], "undo after switching part goes back in the right part");
    }

    // Export in song mode: the whole song
    {
        TechMachineProcessor q;
        setUp (q);
        q.setSong ({ { 0, 2 }, { 1, 2 } });
        q.setSongMode (true);
        const auto wav = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("tm_song.wav");
        TechMachineProcessor::exportWav (*q.createProjectXml(), wav, 1, {});
        juce::AudioFormatManager formats;
        formats.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (wav));
        const double seconds = reader != nullptr ? (double) reader->lengthInSamples / reader->sampleRate : 0.0;
        reader.reset();
        wav.deleteFile();
        printf ("    exported %.2f s\n", seconds);
        expect (std::abs (seconds - (4.0 * 2.0 + 2.5)) < 0.05, "song mode export: 4 loops + the tail");
    }

    printf ("parts: %d failures\n", failures);
    return failures;
}


// The user's own sounds: saved, listed, loaded exactly, never mixed up between machines, deleted to the bin.
static int test_sounds (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    int failures = 0;
    auto expect = [&] (bool ok, const char* what)
    {
        if (! ok) ++failures;
        printf ("  %-66s %s\n", what, ok ? "ok" : "FAIL");
    };
    const auto folder = TechMachineProcessor::userSoundsFolder (TB303);
    expect (folder.getFullPathName().contains ("Tech Machine Tests"), "the tests use their own sounds folder");
    folder.deleteRecursively();

    TechMachineProcessor p;
    p.newProject();
    auto raw = [&p] (const char* id) { return p.apvts.getRawParameterValue (id)->load(); };
    p.setParam ("tb303_cutoff", 0.123f);
    p.setParam ("tb303_reso", 0.9f);
    p.setParam ("tb303_wave", 1.0f);
    p.setParam ("tb303_volume", 0.4f);
    const auto saved = p.saveUserSound (TB303, "Ma basse");
    expect (saved == "Ma basse" && TechMachineProcessor::userSounds (TB303).contains ("Ma basse"), "a sound is saved and listed");

    p.setParam ("tb303_cutoff", 0.8f);
    p.setParam ("tb303_reso", 0.1f);
    p.setParam ("tb303_wave", 0.0f);
    p.setParam ("tb303_volume", 0.9f);
    expect (p.loadUserSound (TB303, "Ma basse"), "the sound loads");
    expect (std::abs (raw ("tb303_cutoff") - 0.123f) < 1.0e-6f && std::abs (raw ("tb303_reso") - 0.9f) < 1.0e-6f
            && juce::exactlyEqual (raw ("tb303_wave"), 1.0f), "its settings come back exactly");
    expect (std::abs (raw ("tb303_volume") - 0.9f) < 1.0e-6f, "the volume is not part of a sound");

    expect (p.saveUserSound (TB303, "Ma basse") == "Ma basse (2)", "the same name again: Ma basse (2), nothing overwritten");
    expect (p.saveUserSound (TB303, "A/B: test?").isNotEmpty(), "a name with forbidden characters is cleaned");
    expect (TechMachineProcessor::userSounds (TB303).size() == 3, "three sounds listed");

    // A 303 sound is not a 101 sound.
    folder.getChildFile ("Ma basse.tmsound").copyFileTo (TechMachineProcessor::userSoundsFolder (SH101).getChildFile ("Intrus.tmsound"));
    TechMachineProcessor::userSoundsFolder (SH101).createDirectory();
    folder.getChildFile ("Ma basse.tmsound").copyFileTo (TechMachineProcessor::userSoundsFolder (SH101).getChildFile ("Intrus.tmsound"));
    expect (! p.loadUserSound (SH101, "Intrus"), "a sound of another machine is refused");

    // Drum machines too
    p.setParam ("tr909_bd_decay", 0.77f);
    p.saveUserSound (TR909, "Kick long");
    p.setParam ("tr909_bd_decay", 0.2f);
    p.loadUserSound (TR909, "Kick long");
    expect (std::abs (raw ("tr909_bd_decay") - 0.77f) < 1.0e-6f, "drum machine sounds too");

    expect (TechMachineProcessor::deleteUserSound (TB303, "Ma basse (2)") && ! TechMachineProcessor::userSounds (TB303).contains ("Ma basse (2)"),
            "a deleted sound goes to the bin");
    TechMachineProcessor::userSoundsFolder (TB303).getParentDirectory().deleteRecursively();
    printf ("sounds: %d failures\n", failures);
    return failures;
}


// Exports: the MIDI file holds the right notes (drums on channel 10 with General MIDI notes, synth pitches,
// chords, held notes, the song), and the stems give one sounding WAV per machine.
static int test_exports (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    int failures = 0;
    auto expect = [&] (bool ok, const char* what)
    {
        if (! ok) ++failures;
        printf ("  %-66s %s\n", what, ok ? "ok" : "FAIL");
    };
    const auto folder = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("tm_exports");
    folder.deleteRecursively();
    folder.createDirectory();

    TechMachineProcessor p;
    p.newProject();
    p.setBars (1);
    p.setParam (pid::bpm, 120.0f);
    p.setParam (pid::key, 0.0f);     // C
    p.setParam (pid::scale, 2.0f);   // major
    p.setShown (TR909, true);
    p.setShown (TB303, true);
    p.setShown (Juno106, true);
    auto& kick = p.machine (TR909).track;
    for (int st = 0; st < 16; st += 4) kick.hits[0][(size_t) st] = true;
    auto& acid = p.machine (TB303).track;
    acid.on[0] = true; acid.note[0] = 0;                                   // C, one step
    acid.on[4] = true; acid.note[4] = 4; acid.slide[4] = true;             // E held over two steps
    acid.on[5] = true; acid.note[5] = 4;
    auto& juno = p.machine (Juno106).track;
    juno.on[8] = true; juno.note[8] = 0;                                   // a chord on C
    p.setParam ("juno106_chord", 1.0f);
    for (auto* t : { &kick, &acid, &juno }) t->tile (16);

    auto readMidi = [] (const juce::File& f)
    {
        juce::MidiFile midi;
        juce::FileInputStream in (f);
        midi.readFrom (in);
        return midi;
    };
    auto notesOf = [] (const juce::MidiMessageSequence& seq, int& channel)
    {
        std::vector<std::pair<double, int>> notes;
        for (auto* e : seq)
            if (e->message.isNoteOn())
            {
                notes.push_back ({ e->message.getTimeStamp(), e->message.getNoteNumber() });
                channel = e->message.getChannel();
            }
        return notes;
    };
    auto lengthOf = [] (const juce::MidiMessageSequence& seq, double start)
    {
        for (auto* e : seq)
            if (e->message.isNoteOn() && std::abs (e->message.getTimeStamp() - start) < 1.0 && e->noteOffObject != nullptr)
                return e->noteOffObject->message.getTimeStamp() - start;
        return 0.0;
    };

    const auto midiFile = folder.getChildFile ("test.mid");
    expect (p.exportMidi (midiFile), "MIDI file written");
    const auto midi = readMidi (midiFile);
    expect (midi.getNumTracks() == 4, "one track per machine plus the tempo");
    int drumChannel = 0, acidChannel = 0, junoChannel = 0;
    const auto drums = notesOf (*midi.getTrack (1), drumChannel);
    expect (drums.size() == 4 && drums[0].second == 36 && std::abs (drums[1].first - 960.0) < 1.0 && drumChannel == 10,
            "909 kick: 4 notes C1 (36) on the beats, channel 10");
    const auto bass = notesOf (*midi.getTrack (2), acidChannel);
    const int base = machineBaseNote (machineDef (TB303), 0, 2);
    expect (bass.size() == 2 && bass[0].second == base && bass[1].second == base + 4, "303: C then E, at the right pitch");
    expect (std::abs (lengthOf (*midi.getTrack (2), 4 * 240.0) - 1.5 * 240.0) < 1.0, "303: the held E lasts 1.5 steps (like the gate of the synth)");
    const auto chord = notesOf (*midi.getTrack (3), junoChannel);
    int intervals[5];
    const int expectedNotes = chordIntervals (2, 0, 1, intervals);   // the chord the synth plays: C major scale, chord type Auto
    expect ((int) chord.size() == expectedNotes && std::abs (chord[0].first - 8 * 240.0) < 1.0, "106: the chord the synth plays, on step 9");

    // Song: A x2 then B (empty) x1 -> the notes of A twice.
    p.setSong ({ { 0, 2 }, { 1, 1 } });
    p.setSongMode (true);
    p.selectPart (0);
    const auto songFile = folder.getChildFile ("song.mid");
    p.exportMidi (songFile);
    int ch = 0;
    expect (notesOf (*readMidi (songFile).getTrack (1), ch).size() == 8, "song: the kicks of part A twice");
    p.setSongMode (false);

    // Stems
    const auto stems = folder.getChildFile ("stems");
    expect (TechMachineProcessor::exportStems (*p.createProjectXml(), stems, 1, {}), "stems exported");
    const auto files = stems.findChildFiles (juce::File::findFiles, false, "*.wav");
    expect (files.size() == 3, "one WAV per machine of the rack");
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    int sounding = 0;
    for (auto& f : files)
    {
        std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (f));
        if (reader == nullptr) continue;
        juce::AudioBuffer<float> buffer (2, (int) reader->lengthInSamples);
        reader->read (&buffer, 0, (int) reader->lengthInSamples, 0, true, true);
        sounding += buffer.getMagnitude (0, buffer.getNumSamples()) > 0.01f ? 1 : 0;
    }
    expect (sounding == 3, "every stem has sound");
    folder.deleteRecursively();

    // Separate outputs in a host: the kick sent to output 2 leaves the master.
    {
        juce::AudioProcessor::setTypeOfNextNewPlugin (juce::AudioProcessor::wrapperType_VST3);
        TechMachineProcessor host;
        juce::AudioProcessor::setTypeOfNextNewPlugin (juce::AudioProcessor::wrapperType_Undefined);
        auto layout = host.getBusesLayout();
        layout.outputBuses.getReference (1) = juce::AudioChannelSet::stereo();
        expect (host.setBusesLayout (layout), "the host can turn on output 2");
        host.newProject();
        host.setShown (TR909, true);
        for (int st = 0; st < maxSteps; st += 4) host.machine (TR909).track.hits[0][(size_t) st] = true;
        host.setParam (pid::machine (TR909, "output"), 1.0f);
        host.prepareToPlay (48000.0, 512);
        FakeHead head;
        host.setPlayHead (&head);
        juce::AudioBuffer<float> buffer (host.getTotalNumOutputChannels(), 512);
        juce::MidiBuffer none;
        float master = 0.0f, aux = 0.0f;
        for (int i = 0; i < 100; ++i)
        {
            head.ppq = (double) i * 512.0 / 48000.0 * 2.0;
            host.processBlock (buffer, none);
            master = std::max (master, buffer.getMagnitude (0, 0, 512));
            aux = std::max (aux, buffer.getMagnitude (2, 0, 512));
        }
        expect (aux > 0.1f && master < 0.001f, "the kick plays on output 2, not on the master");
        host.setParam (pid::machine (TR909, "output"), 0.0f);
        master = 0.0f;
        for (int i = 100; i < 200; ++i)
        {
            head.ppq = (double) i * 512.0 / 48000.0 * 2.0;
            host.processBlock (buffer, none);
            master = std::max (master, buffer.getMagnitude (0, 0, 512));
        }
        expect (master > 0.1f, "back on the master");
        host.setPlayHead (nullptr);
    }
    printf ("exports: %d failures\n", failures);
    return failures;
}

// MIDI expression: velocity of the steps, pitch wheel, modulation wheel, MIDI clock and the vocoder microphone.
static int test_expression (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    int failures = 0;
    auto expect = [&] (bool ok, const char* what)
    {
        if (! ok) ++failures;
        printf ("  %-66s %s\n", what, ok ? "ok" : "FAIL");
    };
    constexpr double rate = 48000.0;
    constexpr int bs = 480;

    // Renders blocks; the MIDI events go in the first block. Returns the left channel.
    auto render = [&] (TechMachineProcessor& p, int blocks, juce::MidiBuffer midi = {}, const std::vector<float>* input = nullptr)
    {
        std::vector<float> out;
        juce::AudioBuffer<float> block (2, bs);
        for (int b = 0; b < blocks; ++b)
        {
            block.clear();
            if (input != nullptr)
                for (int ch = 0; ch < 2; ++ch)
                    for (int i = 0; i < bs; ++i) block.setSample (ch, i, (*input)[(size_t) ((b * bs + i) % (int) input->size())]);
            p.processBlock (block, midi);
            midi.clear();
            out.insert (out.end(), block.getReadPointer (0), block.getReadPointer (0) + bs);
        }
        return out;
    };
    auto peakOf = [] (const std::vector<float>& v, size_t from = 0)
    {
        float peak = 0.0f;
        for (size_t i = from; i < v.size(); ++i) peak = std::max (peak, std::abs (v[i]));
        return peak;
    };
    auto rmsOf = [] (const std::vector<float>& v)
    {
        double sum = 0.0;
        for (float x : v) sum += (double) x * x;
        return std::sqrt (sum / (double) juce::jmax ((size_t) 1, v.size()));
    };
    auto pitchOf = [&] (const std::vector<float>& v) { return estimatePitch (v, rate); };
    auto setup = [&] (TechMachineProcessor& p, int type, int inputs = 0)
    {
        p.newProject();
        p.setBars (1);
        p.setParam (pid::bpm, 120.0f);
        p.setShown (type, true);
        p.setMidiTarget (type);
        p.setPlayConfigDetails (inputs, 2, rate, bs);
        p.prepareToPlay (rate, bs);
    };

    // Velocity: a quiet step plays quieter, a step without a value plays as before.
    {
        float peaks[2] {};
        for (int quiet = 0; quiet < 2; ++quiet)
        {
            TechMachineProcessor p;
            setup (p, TR909);
            p.machine (TR909).track.hits[0][0] = true;
            if (quiet == 1) p.setAutomation (TR909, automation::velocityTarget (machineDef (TR909)), 0, automation::normalVelocity * 0.5f);
            p.playing = true;
            peaks[quiet] = peakOf (render (p, 20));
        }
        printf ("  velocity: normal %.3f, half %.3f\n", peaks[0], peaks[1]);
        expect (peaks[0] > 0.05f && std::abs (peaks[1] / peaks[0] - 0.5f) < 0.1f, "a step at half velocity hits about half as hard");
    }

    // Velocity recorded from the keyboard, and written to the MIDI export.
    {
        TechMachineProcessor p;
        setup (p, SH101);
        p.playing = true;
        p.setRecordNotes (true);
        render (p, 2);
        juce::MidiBuffer on;
        on.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 40), 0);
        render (p, 2, on);
        juce::MidiBuffer off;
        off.addEvent (juce::MidiMessage::noteOff (1, 60), 0);
        render (p, 2, off);
        p.setRecordNotes (false);
        auto& track = p.machine (SH101).track;
        const int lane = track.findLane (automation::velocityTarget (machineDef (SH101)));
        float recorded = -1.0f;
        for (int st = 0; st < 16 && lane >= 0; ++st)
            if (track.on[(size_t) st]) recorded = track.lockValue[(size_t) lane][(size_t) st].load();
        printf ("  recorded velocity %.3f\n", recorded);
        expect (std::abs (recorded - 40.0f / 127.0f * 0.9375f) < 0.01f, "a soft key records a soft step");

        const auto file = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("tm_velocity.mid");
        p.exportMidi (file);
        juce::MidiFile midi;
        juce::FileInputStream in (file);
        midi.readFrom (in);
        int exported = -1;
        for (int t = 0; t < midi.getNumTracks(); ++t)
            for (auto* e : *midi.getTrack (t))
                if (e->message.isNoteOn()) exported = e->message.getVelocity();
        file.deleteFile();
        printf ("  exported velocity %d\n", exported);
        expect (exported > 20 && exported < 50, "the MIDI export keeps the soft step");
    }

    // Pitch wheel: up to two semitones on a held note, back when released.
    {
        TechMachineProcessor p;
        setup (p, SH101);
        juce::MidiBuffer on;
        on.addEvent (juce::MidiMessage::noteOn (1, 57, (juce::uint8) 100), 0);
        render (p, 10, on);
        const double normal = pitchOf (render (p, 50));
        juce::MidiBuffer up;
        up.addEvent (juce::MidiMessage::pitchWheel (1, 16383), 0);
        render (p, 10, up);
        const double bent = pitchOf (render (p, 50));
        juce::MidiBuffer centre;
        centre.addEvent (juce::MidiMessage::pitchWheel (1, 8192), 0);
        render (p, 10, centre);
        const double back = pitchOf (render (p, 50));
        printf ("  pitch: %.1f Hz, bent %.1f Hz (ratio %.3f), back %.1f Hz\n", normal, bent, bent / normal, back);
        expect (std::abs (bent / normal - 1.1225) < 0.03, "the pitch wheel raises the note two semitones");
        expect (std::abs (back / normal - 1.0) < 0.01, "the note comes back when the wheel is released");
    }

    // Pitch wheel on a polyphonic machine.
    {
        TechMachineProcessor p;
        setup (p, Juno106);
        p.setParam ("juno106_chord", 0.0f);
        juce::MidiBuffer on;
        on.addEvent (juce::MidiMessage::noteOn (1, 57, (juce::uint8) 100), 0);
        render (p, 10, on);
        const double normal = pitchOf (render (p, 50));
        juce::MidiBuffer down;
        down.addEvent (juce::MidiMessage::pitchWheel (1, 0), 0);
        render (p, 10, down);
        const double bent = pitchOf (render (p, 50));
        printf ("  poly pitch: %.1f Hz, bent %.1f Hz (ratio %.3f)\n", normal, bent, bent / normal);
        expect (std::abs (bent / normal - 0.8909) < 0.03, "the pitch wheel lowers a chord machine two semitones");
    }

    // Modulation wheel: vibrato on the held note.
    {
        std::vector<float> outs[2];
        for (int mod = 0; mod < 2; ++mod)
        {
            TechMachineProcessor p;
            setup (p, SH101);
            juce::MidiBuffer on;
            on.addEvent (juce::MidiMessage::noteOn (1, 57, (juce::uint8) 100), 0);
            if (mod == 1) on.addEvent (juce::MidiMessage::controllerEvent (1, 1, 127), 0);
            outs[mod] = render (p, 100, on);
        }
        // The pitch over windows of 50 ms: steady without the wheel, moving with it.
        auto spread = [&] (const std::vector<float>& v)
        {
            double lo = 1e9, hi = 0;
            const size_t window = 1440;
            for (size_t from = 4800; from + window <= v.size(); from += window)
            {
                const double f = pitchOf (std::vector<float> (v.begin() + (long) from, v.begin() + (long) (from + window)));
                lo = std::min (lo, f);
                hi = std::max (hi, f);
            }
            return hi - lo;
        };
        printf ("  pitch spread: still %.1f Hz, with the wheel %.1f Hz\n", spread (outs[0]), spread (outs[1]));
        expect (spread (outs[1]) > spread (outs[0]) + 4.0, "the modulation wheel adds vibrato");
    }

    // MIDI clock: tempo, start and stop from another machine.
    {
        TechMachineProcessor p;
        setup (p, TR909);
        const double interval = rate * 60.0 / (140.0 * 24.0);
        double nextTick = 0.0;
        int sample = 0;
        auto clockBlocks = [&] (int blocks, juce::MidiBuffer first = {})
        {
            for (int b = 0; b < blocks; ++b)
            {
                juce::MidiBuffer m (first);
                first.clear();
                for (; nextTick < sample + bs; nextTick += interval)
                    m.addEvent (juce::MidiMessage::midiClock(), (int) nextTick - sample);
                render (p, 1, m);
                sample += bs;
            }
        };
        juce::MidiBuffer start;
        start.addEvent (juce::MidiMessage::midiStart(), 0);
        clockBlocks (200, start);
        printf ("  clock: following %d, tempo %.2f, playing %d, step %d\n", (int) p.isFollowingMidiClock(), p.displayBpm.load(),
                (int) p.playing.load(), p.currentStep.load());
        expect (p.isFollowingMidiClock() && std::abs (p.displayBpm.load() - 140.0f) < 0.5f, "the tempo follows the clock (140)");
        expect (p.playing.load(), "MIDI start plays");
        // 2 s at 140 BPM = 18.7 steps: on a bar of 16, step 2
        expect (p.currentStep.load() == 2, "the steps run at the clock's tempo");
        juce::MidiBuffer stop;
        stop.addEvent (juce::MidiMessage::midiStop(), 0);
        clockBlocks (2, stop);
        expect (! p.playing.load(), "MIDI stop stops");
        render (p, 120);
        expect (! p.isFollowingMidiClock(), "without a clock, back to the own tempo");
        render (p, 1);
        expect (std::abs (p.displayBpm.load() - 120.0f) < 0.01f, "the own tempo is back (120)");
    }

    // Vocoder microphone: the chords speak with the voice of the audio input.
    {
        std::vector<float> voice ((size_t) bs * 10);
        juce::Random random (7);
        for (size_t i = 0; i < voice.size(); ++i)   // a buzzing voice at 150 Hz with a little breath
            voice[i] = 0.3f * (2.0f * std::fmod ((float) i * 150.0f / (float) rate, 1.0f) - 1.0f) + 0.03f * (random.nextFloat() * 2.0f - 1.0f);
        const std::vector<float> silence (voice.size(), 0.0f);
        double levels[3] {};
        for (int c = 0; c < 3; ++c)
        {
            TechMachineProcessor p;
            setup (p, Vocoder, 2);
            if (c > 0) p.setParam ("vocoder_source", 1.0f);
            juce::MidiBuffer on;
            on.addEvent (juce::MidiMessage::noteOn (1, 57, (juce::uint8) 100), 0);
            levels[c] = rmsOf (render (p, 100, on, c == 2 ? &silence : &voice));
        }
        printf ("  vocoder rms: internal voice %.4f, microphone %.4f, silent microphone %.5f\n", levels[0], levels[1], levels[2]);
        expect (levels[1] > 0.01, "the microphone makes the chords speak");
        expect (levels[2] < levels[1] * 0.05, "a silent microphone keeps the vocoder quiet");
    }

    // The input never reaches the output by itself.
    {
        TechMachineProcessor p;
        setup (p, TR909, 2);
        std::vector<float> loud ((size_t) bs, 0.5f);
        expect (peakOf (render (p, 4, {}, &loud)) < 1.0e-6f, "the audio input is not passed to the output");
    }

    printf ("expression: %d failures\n", failures);
    return failures;
}

// Sequencer: length and speed of each track, chance, conditions, delay, FILL, scenes, arpeggiator.
static int test_sequencer (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    int failures = 0;
    auto expect = [&] (bool ok, const char* what)
    {
        if (! ok) ++failures;
        printf ("  %-66s %s\n", what, ok ? "ok" : "FAIL");
    };
    constexpr double rate = 48000.0;
    constexpr int bs = 60;            // 100 blocks per step at 120 BPM: the step of each track is read block by block
    constexpr int stepSamples = 6000;

    auto setup = [&] (TechMachineProcessor& p, std::initializer_list<int> types)
    {
        p.newProject();
        p.setBars (1);
        p.setParam (pid::bpm, 120.0f);
        p.setParam (pid::swing, 0.0f);
        for (int t : types) p.setShown (t, true);
        p.setPlayConfigDetails (0, 2, rate, bs);
        p.prepareToPlay (rate, bs);
    };
    // Plays a number of samples; returns the left channel and, per block, the step each machine shows.
    struct Run { std::vector<float> out; std::vector<std::vector<int>> steps; };
    auto play = [&] (TechMachineProcessor& p, int samples, std::initializer_list<int> watched = {})
    {
        Run run;
        run.steps.resize (watched.size());
        juce::AudioBuffer<float> block (2, bs);
        juce::MidiBuffer midi;
        for (int done = 0; done < samples; done += bs)
        {
            block.clear();
            p.processBlock (block, midi);
            run.out.insert (run.out.end(), block.getReadPointer (0), block.getReadPointer (0) + bs);
            size_t k = 0;
            for (int t : watched) run.steps[k++].push_back (p.machine (t).playStep.load());
        }
        return run;
    };
    // The steps a track went through, in order (each new step once).
    auto stepsSeen = [] (const std::vector<int>& perBlock)
    {
        std::vector<int> seen;
        int last = -2;
        for (int s : perBlock)
        {
            if (s != last) { seen.push_back (s); last = s; }
        }
        return seen;
    };
    auto peakIn = [] (const std::vector<float>& v, size_t from, size_t to)
    {
        float peak = 0.0f;
        for (size_t i = from; i < to && i < v.size(); ++i) peak = std::max (peak, std::abs (v[i]));
        return peak;
    };
    auto firstSound = [] (const std::vector<float>& v, size_t from)
    {
        for (size_t i = from; i < v.size(); ++i)
            if (std::abs (v[i]) > 0.01f) return (long) (i - from);
        return -1L;
    };

    // Length: a track of 3 steps against a sequence of 16.
    {
        TechMachineProcessor p;
        setup (p, { TR909, TB303 });
        p.setTrackTiming (TR909, 3, 0);
        p.playing = true;
        auto run = play (p, stepSamples * 16 + bs, { TR909, TB303 });
        const auto drums = stepsSeen (run.steps[0]), bass = stepsSeen (run.steps[1]);
        bool cycles = drums.size() == 17;
        for (size_t i = 0; i < drums.size() && cycles; ++i) cycles = drums[i] == (int) (i % 3);
        expect (cycles, "a track of 3 steps plays 0 1 2 0 1 2...");
        expect (bass.size() == 17 && bass.back() == 0, "the other tracks keep the 16 steps of the sequence");
    }

    // Speeds: double, half, triplets.
    {
        const int expected[numTrackSpeeds] { 16, 32, 8, 24, 12 };
        bool all = true;
        for (int speed = 1; speed < numTrackSpeeds; ++speed)
        {
            TechMachineProcessor p;
            setup (p, { Juno106 });
            p.setTrackTiming (Juno106, 0, speed);
            p.playing = true;
            auto run = play (p, stepSamples * 16 - bs, { Juno106 });
            const auto seen = stepsSeen (run.steps[0]);
            printf ("  speed %d: %d steps in one bar (expected %d)\n", speed, (int) seen.size(), expected[speed]);
            all = all && (int) seen.size() == expected[speed];
        }
        expect (all, "double, half and triplet speeds play the right number of steps");
    }

    // Chance and condition: a kick on the first step, over 32 loops of one bar.
    {
        auto hitsPerLoop = [&] (int setting, float value)
        {
            TechMachineProcessor p;
            setup (p, { TR909 });
            p.setParam (pid::bpm, 200.0f);   // a step of 3600 samples, a loop of 57600
            p.machine (TR909).track.hits[0][0] = true;
            p.setAutomation (TR909, automation::stepTarget (machineDef (TR909), setting), 0, value);
            p.playing = true;
            auto run = play (p, 57600 * 32);
            std::vector<bool> hits;
            for (size_t loop = 0; loop < 32; ++loop)
                hits.push_back (peakIn (run.out, loop * 57600, loop * 57600 + 600) > 0.05f);
            return hits;
        };
        auto countOf = [] (const std::vector<bool>& v) { return (int) std::count (v.begin(), v.end(), true); };
        const auto always = hitsPerLoop (automation::Chance, 1.0f);
        const auto never = hitsPerLoop (automation::Chance, 0.0f);
        const auto half = hitsPerLoop (automation::Chance, 0.5f);
        printf ("  chance: 100%% %d/32, 0%% %d/32, 50%% %d/32\n", countOf (always), countOf (never), countOf (half));
        expect (countOf (always) == 32 && countOf (never) == 0, "chance 100 % always plays, 0 % never");
        expect (countOf (half) >= 8 && countOf (half) <= 24, "chance 50 % plays about one loop in two");
        expect (half == hitsPerLoop (automation::Chance, 0.5f), "the chance gives the same music each time it starts");

        const auto second = hitsPerLoop (automation::Condition, 6.0f);   // Tour 2 sur 2
        bool odd = true;
        for (size_t loop = 0; loop < second.size(); ++loop) odd = odd && second[loop] == (loop % 2 == 1);
        expect (odd, "condition « Tour 2 sur 2 » plays on loops 2, 4, 6...");
        const auto firstOnly = hitsPerLoop (automation::Condition, 3.0f);
        expect (countOf (firstOnly) == 1 && firstOnly[0], "condition « Premier tour seulement » plays once");
        const auto fourth = hitsPerLoop (automation::Condition, 13.0f);  // Tour 4 sur 4
        expect (countOf (fourth) == 8 && fourth[3] && ! fourth[0], "condition « Tour 4 sur 4 » plays on every 4th loop");
    }

    // Delay: half a step late at the highest value.
    {
        long onsets[2] {};
        for (int late = 0; late < 2; ++late)
        {
            TechMachineProcessor p;
            setup (p, { TR909 });
            p.machine (TR909).track.hits[0][4] = true;
            if (late == 1) p.setAutomation (TR909, automation::stepTarget (machineDef (TR909), automation::Delay), 4, 1.0f);
            p.playing = true;
            onsets[late] = firstSound (play (p, stepSamples * 6).out, 0);
        }
        printf ("  delay: step heard at %ld samples, delayed at %ld\n", onsets[0], onsets[1]);
        expect (onsets[0] >= 0 && std::abs (onsets[1] - onsets[0] - stepSamples / 2) <= 2, "the delay moves the step half a step later");
    }

    // FILL: snare and toms on the last beat of the loop, then a crash; it plays once.
    {
        std::vector<float> outs[2];
        for (int fill = 0; fill < 2; ++fill)
        {
            TechMachineProcessor p;
            setup (p, { TR909 });
            for (int st : { 0, 4, 8, 12 }) p.machine (TR909).track.hits[0][(size_t) st] = true;
            p.armFill (fill == 1);
            p.playing = true;
            outs[fill] = play (p, stepSamples * 16 * 3).out;
            if (fill == 1) expect (! p.isFillArmed(), "FILL turns itself off once played");
        }
        auto differs = [&] (size_t from, size_t to)
        {
            double diff = 0.0;
            for (size_t i = from; i < to; ++i) diff = std::max (diff, (double) std::abs (outs[0][i] - outs[1][i]));
            return diff;
        };
        const size_t bar = stepSamples * 16;
        printf ("  fill: difference beats 1-3 %.4f, last beat %.4f, next downbeat %.4f, second loop %.4f\n",
                differs (0, stepSamples * 12), differs (stepSamples * 12, bar), differs (bar, bar + stepSamples), differs (bar + stepSamples * 4, 2 * bar));
        expect (differs (0, stepSamples * 12) < 1.0e-6, "nothing changes before the last beat");
        expect (differs (stepSamples * 12 + 2000, bar) > 0.05, "the last beat plays the fill");
        expect (differs (bar, bar + stepSamples) > 0.05, "a crash on the next downbeat");
        expect (differs (2 * bar + stepSamples * 12, 3 * bar) < differs (stepSamples * 12 + 2000, bar) * 0.2,
                "the next loops play without the fill");
    }

    // Scenes: they come in at the start of the next bar.
    {
        TechMachineProcessor p;
        setup (p, { TR909, TB303 });
        auto muted = [&p] (int t) { return p.apvts.getRawParameterValue (pid::machine (t, "mute"))->load() > 0.5f; };
        p.setParam (pid::machine (TB303, "mute"), 1.0f);
        p.storeScene (0);
        p.setParam (pid::machine (TB303, "mute"), 0.0f);
        p.setParam (pid::machine (TR909, "mute"), 1.0f);
        p.storeScene (1);
        expect (p.hasScene (0) && p.hasScene (1) && ! p.hasScene (2), "scenes are stored");
        p.recallScene (0);
        play (p, bs);
        expect (muted (TB303) && ! muted (TR909), "stopped: a scene comes in at once");
        p.playing = true;
        play (p, stepSamples * 2);
        p.recallScene (1);
        play (p, stepSamples * 4);
        const bool waits = muted (TB303) && ! muted (TR909) && p.cuedSceneIndex() == 1;
        play (p, stepSamples * 11);
        expect (waits && muted (TR909) && ! muted (TB303) && p.lastScene() == 1, "playing: the scene waits for the next bar");

        // Saved with the project, with the length and speed of the tracks.
        p.setTrackTiming (TB303, 5, 3);
        const auto file = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("tm_sequencer.tmproj");
        p.playing = false;
        play (p, bs);
        p.saveProject (file);
        TechMachineProcessor q;
        q.loadProject (file);
        file.deleteFile();
        expect (q.hasScene (0) && q.hasScene (1) && ! q.hasScene (2), "scenes are saved with the project");
        expect (q.machine (TB303).track.length.load() == 5 && q.machine (TB303).track.speed.load() == 3, "track length and speed are saved");
        q.newProject();
        expect (! q.hasScene (0) && q.machine (TB303).track.length.load() == 0, "a new project has no scenes and normal tracks");
    }

    // In a music software: the tracks follow the position of the song, even when it starts in the middle.
    {
        juce::AudioProcessor::setTypeOfNextNewPlugin (juce::AudioProcessor::wrapperType_VST3);
        TechMachineProcessor host;
        juce::AudioProcessor::setTypeOfNextNewPlugin (juce::AudioProcessor::wrapperType_Undefined);
        host.newProject();
        host.setShown (TR909, true);
        host.setShown (Juno106, true);
        host.setTrackTiming (TR909, 3, 0);
        host.setTrackTiming (Juno106, 0, 1);
        host.prepareToPlay (rate, bs);
        FakeHead head;
        host.setPlayHead (&head);
        juce::AudioBuffer<float> buffer (host.getTotalNumOutputChannels(), bs);
        juce::MidiBuffer none;
        std::vector<int> drums, juno;
        for (int i = 0; i < 300; ++i)   // from bar 2 (beat 4.0), three steps
        {
            head.ppq = 4.0 + (double) i * bs / rate * 2.0;
            host.processBlock (buffer, none);
            if (i % 100 == 1)
            {
                drums.push_back (host.machine (TR909).playStep.load());
                juno.push_back (host.machine (Juno106).playStep.load());
            }
        }
        host.setPlayHead (nullptr);
        printf ("  host from bar 2: 3-step track %d %d %d, double-speed track %d %d %d\n", drums[0], drums[1], drums[2], juno[0], juno[1], juno[2]);
        expect (drums == std::vector<int> { 1, 2, 0 }, "a track of 3 steps follows the song position (step 16 = its step 1)");
        expect (juno == std::vector<int> { 0, 2, 4 }, "a double-speed track follows the song position");
    }

    // MIDI export: a track of 3 steps and a track at double speed.
    {
        TechMachineProcessor p;
        setup (p, { TR909, TB303 });
        p.machine (TR909).track.hits[0][0] = true;
        p.setTrackTiming (TR909, 3, 0);
        auto& bass = p.machine (TB303).track;
        for (size_t st = 0; st < 16; ++st) bass.on[st] = true;
        p.setTrackTiming (TB303, 0, 1);
        const auto file = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("tm_sequencer.mid");
        p.exportMidi (file);
        juce::MidiFile midi;
        juce::FileInputStream in (file);
        midi.readFrom (in);
        int kicks = 0, notes = 0;
        for (int t = 1; t < midi.getNumTracks(); ++t)
            for (auto* e : *midi.getTrack (t))
                if (e->message.isNoteOn()) ++(e->message.getChannel() == 10 ? kicks : notes);
        file.deleteFile();
        printf ("  MIDI export: %d kicks, %d bass notes\n", kicks, notes);
        expect (kicks == 6, "a track of 3 steps: 6 kicks in a bar of 16");
        expect (notes == 32, "a track at double speed: 32 notes in a bar");
    }

    // Arpeggiator: a held chord is played note by note, up, one note per 16th.
    {
        TechMachineProcessor p;
        setup (p, { Juno106 });
        p.setMidiTarget (Juno106);
        p.setParam (pid::machine (Juno106, "arp"), 1.0f);
        play (p, bs);
        juce::MidiBuffer chord;
        for (int note : { 48, 52, 55 }) chord.addEvent (juce::MidiMessage::noteOn (1, note, (juce::uint8) 100), 0);
        juce::AudioBuffer<float> block (2, bs);
        p.processBlock (block, chord);
        const auto out = play (p, stepSamples * 6).out;
        std::vector<double> pitches;
        for (int st = 0; st < 6; ++st)
        {
            const std::vector<float> window (out.begin() + st * stepSamples + 600, out.begin() + st * stepSamples + 3000);
            pitches.push_back (estimatePitch (window, rate));
        }
        printf ("  arpeggio:");
        for (double f : pitches) printf (" %.0f", f);
        printf (" Hz\n");
        // The estimate may land an octave low (the sub oscillator).
        auto near = [] (double f, int midi)
        {
            const double hz = juce::MidiMessage::getMidiNoteInHertz (midi);
            return std::abs (f / hz - 1.0) < 0.03 || std::abs (f / (hz * 0.5) - 1.0) < 0.03;
        };
        expect (near (pitches[0], 48) && near (pitches[1], 52) && near (pitches[2], 55) && near (pitches[3], 48),
                "the held chord C E G is played C, E, G, C...");

        p.setParam (pid::machine (Juno106, "arpoctaves"), 1.0f);
        p.setParam (pid::machine (Juno106, "arp"), 2.0f);   // down
        play (p, bs);
        for (int note : { 48, 52, 55 }) chord.addEvent (juce::MidiMessage::noteOn (1, note, (juce::uint8) 100), 0);
        p.processBlock (block, chord);
        const auto down = play (p, stepSamples * 3).out;
        const std::vector<float> firstNote (down.begin() + 600, down.begin() + 3000);
        printf ("  arpeggio down over 2 octaves starts at %.0f Hz\n", estimatePitch (firstNote, rate));
        expect (near (estimatePitch (firstNote, rate), 67), "down over two octaves starts on the top G");
    }

    printf ("sequencer: %d failures\n", failures);
    return failures;
}

// Sound: talkbox vowels at the same level, the new effects, less aliasing.
static int test_sound (int argc, char** argv)
{
    juce::ignoreUnused (argc, argv);
    int failures = 0;
    auto expect = [&] (bool ok, const char* what)
    {
        if (! ok) ++failures;
        printf ("  %-66s %s\n", what, ok ? "ok" : "FAIL");
    };
    constexpr double rate = 48000.0;
    constexpr int bs = 480;

    auto setup = [&] (TechMachineProcessor& p, std::initializer_list<int> types)
    {
        p.newProject();
        p.setBars (1);
        p.setParam (pid::bpm, 120.0f);
        p.setParam (pid::swing, 0.0f);
        for (int t : types) p.setShown (t, true);
        p.setPlayConfigDetails (0, 2, rate, bs);
        p.prepareToPlay (rate, bs);
    };
    struct Stereo { std::vector<float> left, right; };
    auto render = [&] (TechMachineProcessor& p, int blocks, juce::MidiBuffer midi = {})
    {
        Stereo out;
        juce::AudioBuffer<float> block (2, bs);
        for (int b = 0; b < blocks; ++b)
        {
            block.clear();
            p.processBlock (block, midi);
            midi.clear();
            out.left.insert (out.left.end(), block.getReadPointer (0), block.getReadPointer (0) + bs);
            out.right.insert (out.right.end(), block.getReadPointer (1), block.getReadPointer (1) + bs);
        }
        return out;
    };
    auto rmsDb = [] (const std::vector<float>& v, size_t from, size_t to)
    {
        double sum = 0.0;
        to = std::min (to, v.size());
        for (size_t i = from; i < to; ++i) sum += (double) v[i] * v[i];
        return 10.0 * std::log10 (sum / (double) juce::jmax ((size_t) 1, to - from) + 1.0e-12);
    };
    auto noteOn = [] (int note)
    {
        juce::MidiBuffer m;
        m.addEvent (juce::MidiMessage::noteOn (1, note, (juce::uint8) 100), 0);
        return m;
    };

    // Talkbox: every vowel held at the same level (within 2 dB).
    {
        double lo = 1e9, hi = -1e9;
        printf ("  talkbox vowels:");
        for (int v = 0; v < 6; ++v)
        {
            double level = 0.0;
            for (int note : { 48, 60, 67 })
            {
                TechMachineProcessor p;
                setup (p, { Talkbox });
                p.setMidiTarget (Talkbox);
                p.setParam (pid::synth (Talkbox, pTalk::From), (float) v);
                p.setParam (pid::synth (Talkbox, pTalk::To), (float) v);
                render (p, 1);
                const auto out = render (p, 60, noteOn (note));
                level += rmsDb (out.left, 9600, out.left.size()) / 3.0;
            }
            printf (" %s %.1f", machineDef (Talkbox).params[pTalk::From].choices[(size_t) v], level);
            lo = std::min (lo, level);
            hi = std::max (hi, level);
        }
        printf (" dB\n");
        expect (hi - lo < 2.0, "the six talkbox vowels play at the same level");
    }

    // Aliasing: on a held note, the energy that is not on a harmonic of the note (folded back from above the
    // highest frequency the sample rate can hold), relative to the whole sound.
    auto inharmonicDb = [&] (const std::vector<float>& v, double f0)
    {
        constexpr int n = 8192;
        const size_t from = v.size() - (size_t) n;
        std::vector<double> x ((size_t) n);
        for (int i = 0; i < n; ++i)
        {
            const double w = 0.35875 - 0.48829 * std::cos (2.0 * juce::MathConstants<double>::pi * i / (n - 1))
                           + 0.14128 * std::cos (4.0 * juce::MathConstants<double>::pi * i / (n - 1))
                           - 0.01168 * std::cos (6.0 * juce::MathConstants<double>::pi * i / (n - 1));
            x[(size_t) i] = v[from + (size_t) i] * w;
        }
        double harmonic = 0.0, other = 0.0;
        for (int k = 4; k < n / 2; ++k)
        {
            const double step = 2.0 * juce::MathConstants<double>::pi * k / n;
            double re = 0.0, im = 0.0;
            for (int i = 0; i < n; ++i)
            {
                re += x[(size_t) i] * std::cos (step * i);
                im -= x[(size_t) i] * std::sin (step * i);
            }
            const double hz = (double) k * rate / n;
            const double nearest = std::round (hz / f0) * f0;
            ((nearest > 0.0 && std::abs (hz - nearest) < 5.0 * rate / n) ? harmonic : other) += re * re + im * im;
        }
        return 10.0 * std::log10 (other / (harmonic + other) + 1.0e-12);
    };
    auto heldNote = [&] (int type, int note, std::initializer_list<std::pair<int, float>> settings)
    {
        TechMachineProcessor p;
        setup (p, { type });
        p.setMidiTarget (type);
        for (auto& [param, value] : settings) p.setParam (pid::synth (type, param), value);
        p.setParam (pid::machine (type, "reverb"), 0.0f);
        p.setParam (pid::echoLevel, 0.0f);
        p.setParam (pid::reverbLevel, 0.0f);
        render (p, 1);
        return render (p, 150, noteOn (note)).left;
    };
    {
        const auto west = heldNote (WestCoast, 84, { { pWest::Gate, 1.0f }, { pWest::Fold, 1.0f }, { pWest::Fm, 0.0f }, { pWest::Decay, 1.0f } });
        const double westDb = inharmonicDb (west, juce::MidiMessage::getMidiNoteInHertz (84));
        const auto odyssey = heldNote (SemiMod, 79, { { pSemi::Model, 1.0f }, { pSemi::Wave, 0.0f }, { pSemi::Osc2, 1.0f }, { pSemi::Cutoff, 1.0f },
                                                     { pSemi::Env, 0.3f }, { pSemi::Sustain, 1.0f }, { pSemi::Drive, 0.0f } });
        const double odysseyDb = inharmonicDb (odyssey, juce::MidiMessage::getMidiNoteInHertz (79));
        printf ("  aliasing: west coast wavefolder %.1f dB, odyssey sync %.1f dB\n", westDb, odysseyDb);
        expect (westDb < -50.0, "the WEST COAST wavefolder keeps its aliasing low");
        expect (odysseyDb < -36.0, "the synced oscillator of the Odyssey keeps its aliasing low");
    }

    // Effects: one 909 hit, sent to the echo or the reverb of the master.
    auto hit = [&] (int row, std::initializer_list<std::pair<const char*, float>> params, int blocks)
    {
        TechMachineProcessor p;
        setup (p, { TR909 });
        p.machine (TR909).track.hits[(size_t) row][0] = true;
        p.setParam (pid::echoLevel, 0.0f);
        p.setParam (pid::reverbLevel, 0.0f);
        for (auto& [id, value] : params) p.setParam (id, value);
        p.playing = true;
        const auto out = render (p, blocks);
        p.playing = false;
        return out;
    };
    {
        auto sideDifference = [] (const Stereo& o, size_t from, size_t to)
        {
            double diff = 0.0;
            for (size_t i = from; i < to; ++i) diff = std::max (diff, (double) std::abs (o.left[i] - o.right[i]));
            return diff;
        };
        const auto send = juce::String (pid::machine (TR909, "echo"));
        const auto pingPong = hit (0, { { send.toRawUTF8(), 1.0f }, { pid::echoLevel, 1.0f } }, 400);
        const auto tape = hit (0, { { send.toRawUTF8(), 1.0f }, { pid::echoLevel, 1.0f }, { pid::echoType, 1.0f } }, 400);
        printf ("  echo: left-right difference ping-pong %.3f, tape %.6f; tape tail %.1f dB\n", sideDifference (pingPong, 48000, 192000),
                sideDifference (tape, 48000, 192000), rmsDb (tape.left, 48000, 192000));
        expect (sideDifference (pingPong, 48000, 192000) > 0.01 && sideDifference (tape, 48000, 192000) < 1.0e-6
                    && rmsDb (tape.left, 48000, 192000) > -50.0, "the tape echo repeats in the middle, the ping-pong on both sides");
    }
    {
        const auto send = juce::String (pid::machine (TR909, "reverb"));
        auto reverbOf = [&] (float type)
        {
            return hit (3, { { send.toRawUTF8(), 1.0f }, { pid::reverbLevel, 1.0f }, { pid::reverbType, type } }, 300).left;
        };
        // The reverb alone: the same hit without reverb taken away. The next hit comes at 2 s.
        const auto dry = hit (3, {}, 300).left;
        auto wet = [&] (const std::vector<float>& v)
        {
            std::vector<float> w (v.size());
            for (size_t i = 0; i < v.size(); ++i) w[i] = v[i] - dry[i];
            return w;
        };
        const auto room = wet (reverbOf (0.0f)), hall = wet (reverbOf (1.0f)), plate = wet (reverbOf (2.0f));
        const double roomTail = rmsDb (room, 60000, 90000), hallTail = rmsDb (hall, 60000, 90000);
        auto onset = [] (const std::vector<float>& w)
        {
            for (size_t i = 0; i < w.size(); ++i)
                if (std::abs (w[i]) > 1.0e-4f) return (double) i / 48.0;
            return 1.0e9;
        };
        const double roomStart = onset (room), plateStart = onset (plate);
        printf ("  reverb: tail after 1.3 s room %.1f / hall %.1f dB, starts after room %.1f / plate %.1f ms\n", roomTail, hallTail, roomStart, plateStart);
        expect (hallTail > roomTail + 3.0, "the hall rings longer than the room");
        expect (plateStart < roomStart - 10.0, "the plate starts sooner than the room (no gap)");
    }

    // Sidechain: the synths make room for the kicks, following their length.
    {
        auto synthUnderKicks = [&] (float mode, float kickDecay)
        {
            Stereo outs[2];
            for (int withSynth = 0; withSynth < 2; ++withSynth)
            {
                TechMachineProcessor p;
                setup (p, withSynth == 1 ? std::initializer_list<int> { TR909, Juno106 } : std::initializer_list<int> { TR909 });
                for (int st = 0; st < 16; st += 4) p.machine (TR909).track.hits[0][(size_t) st] = true;
                p.setParam (pid::row (TR909, 0, "decay"), kickDecay);
                p.setParam (pid::pump, 1.0f);
                p.setParam (pid::pumpMode, mode);
                p.setParam (pid::echoLevel, 0.0f);
                p.setParam (pid::reverbLevel, 0.0f);
                p.setMidiTarget (Juno106);
                render (p, 1);
                if (withSynth == 1) render (p, 1, noteOn (57));
                p.playing = true;
                outs[withSynth] = render (p, 300);
            }
            std::vector<float> synth (outs[1].left.size());
            for (size_t i = 0; i < synth.size(); ++i) synth[i] = outs[1].left[i] - outs[0].left[i];
            return synth;
        };
        const size_t beat = 24000, kick = beat * 4;   // the second bar: everything is settled
        const auto shortKick = synthUnderKicks (1.0f, 0.1f), longKick = synthUnderKicks (1.0f, 1.0f);
        const double during = rmsDb (shortKick, kick + 480, kick + 2400), after = rmsDb (shortKick, kick + 19000, kick + 23500);
        const double longLater = rmsDb (longKick, kick + 7200, kick + 9600), shortLater = rmsDb (shortKick, kick + 7200, kick + 9600);
        printf ("  sidechain: synth %.1f dB under the kick, %.1f dB before the next; 150-200 ms after a short kick %.1f, a long kick %.1f dB\n",
                during, after, shortLater, longLater);
        expect (during < after - 6.0, "the sidechain lowers the synths under each kick");
        expect (longLater < shortLater - 2.0, "a longer kick keeps the synths down longer");
    }

    // Phaser and flanger: the sound moves, its level stays.
    {
        auto juno = [&] (const char* effect, float amount)
        {
            TechMachineProcessor p;
            setup (p, { Juno106 });
            p.setMidiTarget (Juno106);
            p.setParam (pid::machine (Juno106, effect), amount);
            p.setParam (pid::echoLevel, 0.0f);
            p.setParam (pid::reverbLevel, 0.0f);
            render (p, 1);
            return render (p, 300, noteOn (57)).left;
        };
        const auto dry = juno ("phaser", 0.0f), phased = juno ("phaser", 1.0f), flanged = juno ("flanger", 1.0f);
        auto difference = [&] (const std::vector<float>& wet)
        {
            double d = 0.0;
            for (size_t i = 48000; i < wet.size(); ++i) d = std::max (d, (double) std::abs (wet[i] - dry[i]));
            return d;
        };
        const double dryDb = rmsDb (dry, 48000, dry.size());
        printf ("  modulation: dry %.1f dB, phaser %.1f dB (changes %.2f), flanger %.1f dB (changes %.2f)\n", dryDb,
                rmsDb (phased, 48000, phased.size()), difference (phased), rmsDb (flanged, 48000, flanged.size()), difference (flanged));
        expect (difference (phased) > 0.02 && std::abs (rmsDb (phased, 48000, phased.size()) - dryDb) < 3.0, "the phaser changes the sound, not its level");
        expect (difference (flanged) > 0.02 && std::abs (rmsDb (flanged, 48000, flanged.size()) - dryDb) < 3.0, "the flanger changes the sound, not its level");
    }

    printf ("sound: %d failures\n", failures);
    return failures;
}

struct Mode { const char* name; int (*run) (int, char**); };

static const Mode modes[] {
    { "automation", test_automation },
    { "parts", test_parts },
    { "sounds", test_sounds },
    { "exports", test_exports },
    { "expression", test_expression },
    { "sequencer", test_sequencer },
    { "sound", test_sound },
    { "host", test_host },
    { "levels", test_levels },
    { "examples", test_examples },
    { "voice", test_voice },
    { "reverb", test_reverb },
    { "fx", test_fx },
    { "audit", test_audit },
    { "surprise", test_surprise },
    { "midi", test_midi },
    { "roll", test_roll },
    { "rollmouse", test_rollmouse },
    { "golden", test_golden },
    { "sweep", test_sweep },
    { "project", test_project },
    { "ui", test_ui },
    { "propagate", test_propagate },
    { "chaos", test_chaos },
    { "choices", test_choices },
    { "song", test_song },
    { "blow", test_blow },
    { "params", test_params },
    { "patterns", test_patterns },
    { "lang", test_lang },
    { "genres", test_genres },
    { "tools", test_tools },
    { "snap", test_snap },
};

// French text left untranslated in the English interface (names of instruments are the same in both languages).
static int test_french (int argc, char** argv)
{
    test_lang (argc, argv);
    int french = 0;
    for (auto& text : juce::StringArray::fromLines (outDir.getChildFile ("missing.txt").loadFileAsString()))
        if (text.containsAnyOf (juce::CharPointer_UTF8 ("\xc3\xa9\xc3\xa8\xc3\xa0\xc3\xa7\xc3\xb9\xc3\xaa\xc3\xb4\xc3\xbb\xc3\xae")))
        {
            printf ("  untranslated: %s\n", text.substring (0, 80).toRawUTF8());
            ++french;
        }
    printf ("french: %d untranslated texts\n", french);
    return french;
}

static int check (char** argv)
{
    chaosIterations = 8;
    char* none[] { argv[0], nullptr };
    int failures = 0;
    for (auto run : { test_params, test_project, test_propagate, test_automation, test_parts, test_sounds, test_exports, test_expression, test_sequencer, test_sound, test_ui, test_chaos, test_french })
        failures += run (1, none);
    printf ("\ncheck: %d failures\n", failures);
    return failures;
}

int main (int argc, char** argv)
{
    setvbuf (stdout, nullptr, _IONBF, 0);
#if JUCE_WINDOWS && JUCE_DEBUG
    _CrtSetReportMode (_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile (_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportHook ([] (int, char* message, int*) -> int
    {
        fprintf (stderr, "%s\n%s\n", message, juce::SystemStats::getStackBacktrace().toRawUTF8());
        exit (3);
    });
#endif
    // The tests use their own settings (recent projects, MIDI map), never the user's.
#if JUCE_WINDOWS
    _putenv ("TECHMACHINE_TEST_SETTINGS=1");
#else
    setenv ("TECHMACHINE_TEST_SETTINGS", "1", 1);
#endif
    juce::ScopedJuceInitialiser_GUI init;
    const juce::String mode = argc > 1 ? juce::String (argv[1]) : "check";
    if (mode == "check") return check (argv) == 0 ? 0 : 1;
    if (mode == "french") return test_french (argc, argv) == 0 ? 0 : 1;
    for (auto& m : modes)
        if (mode == m.name) return m.run (argc, argv) == 0 ? 0 : 1;
    printf ("unknown mode %s\n", mode.toRawUTF8());
    return 2;
}
