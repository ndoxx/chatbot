#ifndef SONG_FACTORY_H
#define SONG_FACTORY_H

#include <random>

#include "song.h"

namespace harm
{

struct SongDescriptor
{
    // Harmony
    int first_chord = 0;           // codes for first chord in the verse
    int second_chord = 0;          // codes for second chord in the verse
    int cadence = 0;               // codes for cadence
    int root = 0;                  // root chord
    bool minor = false;            // true: song is minor, output is translated to relative minor
    // Structure
    bool force_4_chords = false;   // Whole song is 32x same 4 chords
    bool rythm_changes = false;    // Verse = C rythm changes (Anatole en Do |C|a|d|G|)
    bool has_bridge = false;       // Bridge after second chorus
    bool intro_is_root_x8 = false; // Force intro to be 8 bars of root chord
    bool harmonic_march = false;   // Force outro to take the form of an harmonic march i | iv | VII | III || VI | II | V | I x4
    int force_verse = 0;           // Force verse to be 1: C | f | Bb | Eb || D▲| G7 | C7 | G7 x2   2: | V | i | x16
    // Big data
    std::string email;             // For profit
};

class SongFactory
{
public:
    SongFactory(int seed);

    // Create a song object from a descriptor (new)
    Song* generate_song(SongDescriptor& descriptor);

protected:
    // Auxiliary functions
    void generate_verse(std::vector<Step>& steps,
                        SongDescriptor& descriptor);

    int generate_intro(std::vector<Step>& steps,
                       const std::vector<Step>& verse,
                       SongDescriptor& descriptor);

    void generate_chorus(std::vector<Step>& steps,
                         const std::vector<Step>& verse,
                         SongDescriptor& descriptor);

    bool generate_outro(std::vector<Step>& steps,
                        const std::vector<Step>& intro,
                        const std::vector<Step>& verse,
                        const std::vector<Step>& chorus,
                        SongDescriptor& descriptor);

private:
    std::default_random_engine entropy_;
};

} // namespace harm

#endif // SONG_FACTORY_H
