#include "song_factory.h"

namespace harm
{

// Possible offsets for verse
static std::vector<Step> first_chord_select_steps  = {"IV"_s, "VIb"_s, "III"_s, "I"_s};
static std::vector<Step> second_chord_select_steps = {"//"_s, "IV"_s,  "VII"_s, "IIIb"_s};
static std::vector<Step> cadence_1_select_steps    = {"I"_s,  "V"_s,   "iv"_s,  "IIb"_s};
static std::vector<Step> cadence_2_select_steps    = {"I"_s,  "i"_s,   "I"_s,   "I"_s};
// Offsets for rythm changes type of song
static std::vector<Step> rythm_changes_steps       = {"I"_s,  "vi"_s,  "ii"_s,  "V"_s};
// Offsets for forced verses
static std::vector<Step> forced_verse_1_steps      = {"I"_s,  "iv"_s,  "VII"_s, "iii"_s, "VIb"_s, "V"_s,  "I"_s, "V"_s,};
static std::vector<Step> forced_verse_2_steps      = {"V"_s,  "i"_s,   "V"_s,   "i"_s,   "V"_s,   "i"_s,  "V"_s, "i"_s,};
// Possible patterns for chorus
static std::vector<Step> chorus_1_steps            = {"V"_s,  "i"_s,   "V"_s,   "i"_s};
static std::vector<Step> chorus_2_steps            = {"I"_s,  "ii"_s,  "iii"_s, "IV"_s};
// Offsets for forced outro (harmonic march)
static std::vector<Step> forced_outro_steps        = {"i"_s,  "iv"_s,  "VII"_s, "III"_s, "VI"_s,  "II"_s, "V"_s, "I"_s,};
// Offsets for bridge (if any)
static std::vector<Step> bridge_steps              = {"V"_s,  "V"_s,   "V"_s,   "V"_s};
// Possible structures for the whole song
static std::vector<std::string> structures = {"VVVVVVVV", "CVCBVCVO", "IVCVCBCO", "IVCVCBVCO" };

SongFactory::SongFactory(int seed):
entropy_(seed)
{

}

void SongFactory::generate_verse(std::vector<Step>& steps, SongDescriptor& descriptor)
{
    if(descriptor.rythm_changes)
    {
        descriptor.minor = false;
        descriptor.root = 0;
        std::copy(rythm_changes_steps.begin(), rythm_changes_steps.end(), std::back_inserter(steps));
    }
    else if(descriptor.force_verse)
    {
        if(descriptor.force_verse == 1)
            std::copy(forced_verse_1_steps.begin(), forced_verse_1_steps.end(), std::back_inserter(steps));
        if(descriptor.force_verse == 2)
            std::copy(forced_verse_2_steps.begin(), forced_verse_2_steps.end(), std::back_inserter(steps));
    }
    else
    {
        steps.push_back(first_chord_select_steps[descriptor.first_chord]);
        auto&& second_chord = second_chord_select_steps[descriptor.second_chord];
        if(second_chord.offset == -1)
            steps.push_back(steps[0]);
        else
            steps.push_back(second_chord);
        steps.push_back(cadence_1_select_steps[descriptor.cadence]);
        steps.push_back(cadence_2_select_steps[descriptor.cadence]);

        // Repeat 4 bars
        for(int ii=0; ii<4; ++ii)
            steps.push_back(steps[ii]);
    }
}

int SongFactory::generate_intro(std::vector<Step>& steps,
                                const std::vector<Step>& verse,
                                SongDescriptor& descriptor)
{
    int intro_length = 1;

    if(descriptor.intro_is_root_x8)
    {
        for(int ii=0; ii<4; ++ii)
            steps.push_back("I"_s);

        intro_length = 2;
    }
    else
    {
        std::uniform_int_distribution<int> intro_type_dis(0,1);
        std::uniform_int_distribution<int> intro_len_dis(0,2);
        bool intro_type = bool(intro_type_dis(entropy_));
        intro_length = intro_len_dis(entropy_)+1;

        steps.push_back(verse[0]);
        steps.push_back(intro_type ? verse[1] : verse[0]);
        steps.push_back(intro_type ? verse[0] : verse[1]);
        steps.push_back(verse[1]);
    }

    return intro_length;
}

void SongFactory::generate_chorus(std::vector<Step>& steps,
                                  const std::vector<Step>& verse,
                                  SongDescriptor& descriptor)
{
    std::uniform_int_distribution<int> chorus_type_dis(0,2);
    int chorus_type = chorus_type_dis(entropy_);
    switch(chorus_type)
    {
        case 0:
            std::copy(chorus_1_steps.begin(), chorus_1_steps.end(), std::back_inserter(steps));
            break;

        case 1:
            steps.push_back(verse[2]);
            steps.push_back(verse[3]);
            steps.push_back(verse[2]);
            steps.push_back(verse[3]);
            break;

        case 2:
            std::copy(chorus_2_steps.begin(), chorus_2_steps.end(), std::back_inserter(steps));
            break;
    }
}

bool SongFactory::generate_outro(std::vector<Step>& steps,
                                 const std::vector<Step>& intro,
                                 const std::vector<Step>& verse,
                                 const std::vector<Step>& chorus,
                                 SongDescriptor& descriptor)
{
    bool chorus_ad_lib = false;

    if(descriptor.harmonic_march)
        std::copy(forced_outro_steps.begin(), forced_outro_steps.end(), std::back_inserter(steps));
    else
    {
        std::uniform_int_distribution<int> outro_type_dis(0,2);
        int outro_type = outro_type_dis(entropy_);
        switch(outro_type)
        {
            case 0:
                for(int ii=0; ii<8; ++ii)
                    steps.push_back(intro[ii%4]);
                break;
            case 1:
                for(int ii=0; ii<8; ++ii)
                    steps.push_back(verse[ii]);
                break;
            case 2:
                for(int ii=0; ii<8; ++ii)
                    steps.push_back(chorus[ii%4]);
                chorus_ad_lib = true;
                break;
        }
    }

    return chorus_ad_lib;
}

Song* SongFactory::generate_song(SongDescriptor& descriptor)
{
    // * Abstract representation of a song
    std::vector<std::vector<Step>> a_song;
    a_song.resize(5);

    // * Generate verse
    generate_verse(a_song[Song::VERSE], descriptor);

    // * Generate intro
    int intro_length = generate_intro(a_song[Song::INTRO],
                                      a_song[Song::VERSE],
                                      descriptor);

    // * Generate chorus
    generate_chorus(a_song[Song::CHORUS],
                    a_song[Song::VERSE],
                    descriptor);

    // * Generate bridge (just copy a simple pattern ftm)
    std::copy(bridge_steps.begin(), bridge_steps.end(), std::back_inserter(a_song[Song::BRIDGE]));

    // * Generate outro
    bool ad_lib = generate_outro(a_song[Song::OUTRO],
                                 a_song[Song::INTRO],
                                 a_song[Song::VERSE],
                                 a_song[Song::CHORUS],
                                 descriptor);

    // * Select structure
    std::string& structure = structures[0];
    if(!descriptor.force_4_chords)
    {
        std::uniform_int_distribution<int> structure_type_dis(1,3);
        structure = structures[structure_type_dis(entropy_)];
    }

    // * Generate song
    return new Song(a_song,
                    structure,
                    descriptor.root,
                    intro_length,
                    descriptor.minor,
                    descriptor.has_bridge,
                    ad_lib);
}

} // namespace harm
