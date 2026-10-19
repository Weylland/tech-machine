#include "../PluginProcessor.h"

using namespace techno;

// Sounds: the settings of a machine that make its sound (not its volume, pan or notes).
// Factory sounds come from core/Presets; the user's own sounds are small XML files, one per sound:
//   Music/Tech Machine/Sons/<machine id>/<name>.tmsound

std::vector<juce::String> TechMachineProcessor::soundParameters (int type)
{
    const auto& def = machineDef (type);
    std::vector<juce::String> ids;
    for (const char* fx : { "drive", "crush", "filter", "echo", "reverb" }) ids.push_back (pid::machine (type, fx));
    if (def.kind == MachineKind::Drum)
        for (int r = 0; r < (int) def.rows.size(); ++r)
            for (const char* p : { "level", "tune", "decay", "tone", "drive", "filter", "pan", "echo", "reverb" })
                ids.push_back (pid::row (type, r, p));
    for (int i = 0; i < (int) def.params.size(); ++i) ids.push_back (pid::synth (type, i));
    return ids;
}

void TechMachineProcessor::resetSound (int type)
{
    for (auto& id : soundParameters (type))
        if (auto* p = apvts.getParameter (id)) p->setValueNotifyingHost (p->getDefaultValue());
}

juce::File TechMachineProcessor::userSoundsFolder (int type)
{
    // The tests keep their sounds apart from the user's.
    const auto root = juce::SystemStats::getEnvironmentVariable ("TECHMACHINE_TEST_SETTINGS", {}).isNotEmpty()
                          ? juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("Tech Machine Tests")
                          : juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("Tech Machine");
    return root.getChildFile ("Sons").getChildFile (machineDef (type).id);
}

juce::StringArray TechMachineProcessor::userSounds (int type)
{
    juce::StringArray names;
    for (auto& f : userSoundsFolder (type).findChildFiles (juce::File::findFiles, false, "*.tmsound"))
        names.add (f.getFileNameWithoutExtension());
    names.sortNatural();
    return names;
}

juce::String TechMachineProcessor::saveUserSound (int type, const juce::String& wanted)
{
    auto folder = userSoundsFolder (type);
    if (! folder.createDirectory()) return {};

    // Never overwrite a sound: "Basse" then "Basse (2)"...
    const auto base = juce::File::createLegalFileName (wanted.trim().isEmpty() ? utf8 ("Mon son") : wanted.trim());
    auto fileName = base;
    for (int n = 2; folder.getChildFile (fileName + ".tmsound").exists(); ++n) fileName = base + " (" + juce::String (n) + ")";

    juce::XmlElement xml ("TechMachineSound");
    xml.setAttribute ("machine", machineDef (type).id);
    xml.setAttribute ("version", JucePlugin_VersionString);
    for (auto& id : soundParameters (type))
    {
        auto* e = xml.createNewChildElement ("Param");
        e->setAttribute ("id", id);
        e->setAttribute ("value", juce::String ((double) apvts.getRawParameterValue (id)->load(), 9));
    }
    return xml.writeTo (folder.getChildFile (fileName + ".tmsound")) ? fileName : juce::String();
}

bool TechMachineProcessor::loadUserSound (int type, const juce::String& soundName)
{
    const auto xml = juce::XmlDocument::parse (userSoundsFolder (type).getChildFile (soundName + ".tmsound"));
    if (xml == nullptr || ! xml->hasTagName ("TechMachineSound") || xml->getStringAttribute ("machine") != machineDef (type).id)
        return false;

    resetSound (type);   // settings missing from the file (older version) go back to their default
    const auto allowed = soundParameters (type);
    for (auto* e : xml->getChildWithTagNameIterator ("Param"))
    {
        const auto id = e->getStringAttribute ("id");
        if (std::find (allowed.begin(), allowed.end(), id) == allowed.end()) continue;
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 ((float) e->getDoubleAttribute ("value")));
    }
    dirty = true;
    return true;
}

bool TechMachineProcessor::deleteUserSound (int type, const juce::String& soundName)
{
    // To the recycle bin rather than gone for good (the tests just remove their own files).
    const auto file = userSoundsFolder (type).getChildFile (soundName + ".tmsound");
    if (juce::SystemStats::getEnvironmentVariable ("TECHMACHINE_TEST_SETTINGS", {}).isNotEmpty()) return file.deleteFile();
    return file.moveToTrash();
}
