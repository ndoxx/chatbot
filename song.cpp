#include <algorithm>
#include <unordered_map>

#include "song.h"

namespace harm
{

static std::locale locc("C");

static std::vector<std::string> tones   {"C", "Db", "D", "Eb", "E", "F", "Gb",  "G",  "Ab", "A", "Bb", "B"};
static std::vector<std::string> tones_fr{"Do","Réb","Ré","Mib","Mi","Fa","Solb","Sol","Lab","La","Sib","Si"};

static std::unordered_map<std::string, int> step_notation =
{
    {"//",  -1}, // Repeat previous bar
    {"i",    0},
    {"iib",  1},
    {"ii",   2},
    {"iiib", 3},
    {"iii",  4},
    {"iv",   5},
    {"vb",   6},
    {"v",    7},
    {"vib",  8},
    {"vi",   9},
    {"vii",  10}
};

static bool is_lower(const std::string& str)
{
    for(int ii=0; ii<str.size(); ++ii)
    {
        if(!std::islower(str[ii], locc))
            return false;
    }
    return true;
}

Step operator "" _s(const char* notation, size_t)
{
    std::string not_str(notation);
    bool is_minor = is_lower(not_str);
    // Force lower case
    std::transform(not_str.begin(), not_str.end(), not_str.begin(), ::tolower);
    int offset = step_notation[not_str];

    return Step(offset, is_minor);
}

static std::string to_lower(const std::string& str)
{
    std::string ret(str);
    std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
    return ret;
}

std::ostream& operator<<(std::ostream& stream, const Chord& chord)
{
    stream << (chord.is_minor ? to_lower(tones[chord.base]) : tones[chord.base]);
    return stream;
}

Chord Step::to_chord(int root, bool song_is_minor) const
{
    // Compute actual chord, translate 3 steps down if song is minor
    int base = (root + offset - (song_is_minor ? 3 : 0)) % 12;
    // Wrap
    if(base<0)
        base += 12;

    // Either the chord is already minor, or the song is minor and the step is I or IV
    bool is_minor = is_minor || (song_is_minor && (offset==0 || offset==5));

    return Chord(base, is_minor);
}

Song::Song(const std::vector<std::vector<Step>>& abstract_song,
           const std::string& structure,
           int root_chord,
           int intro_length,
           bool song_is_minor,
           bool has_bridge,
           bool ad_lib):
structure_(structure),
intro_length_(intro_length),
has_bridge_(has_bridge),
ad_lib_(ad_lib)
{
    // * Convert to actual chords
    for(int ii=0; ii<8; ++ii)
    {
        if(ii<4)
        {
            intro_pattern_.push_back(abstract_song[INTRO][ii].to_chord(root_chord, song_is_minor));
            chorus_pattern_.push_back(abstract_song[CHORUS][ii].to_chord(root_chord, song_is_minor));
            bridge_pattern_.push_back(abstract_song[BRIDGE][ii].to_chord(root_chord, song_is_minor));
        }
        verse_pattern_.push_back(abstract_song[VERSE][ii].to_chord(root_chord, song_is_minor));
        outro_pattern_.push_back(abstract_song[OUTRO][ii].to_chord(root_chord, song_is_minor));
    }
}

void Song::display(std::ostream& stream)
{
    for(int ii=0; ii<structure_.size(); ++ii)
    {
        switch(structure_[ii])
        {
            case 'I':
                std::cout << "    [INTRO]" << std::endl;
                display_block(stream, intro_pattern_, intro_length_);
                //std::cout << std::endl;
                break;

            case 'C':
                std::cout << "    [CHORUS]" << std::endl;
                display_block(stream, chorus_pattern_, 4);
                //std::cout << std::endl;
                break;

            case 'B':
                if(has_bridge_)
                {
                    std::cout << "    [BRIDGE]" << std::endl;
                    display_block(stream, bridge_pattern_, 4);
                    //std::cout << std::endl;
                }
                break;

            case 'V':
                std::cout << "    [VERSE]" << std::endl;
                display_block(stream, verse_pattern_, 4);
                //std::cout << std::endl;
                break;

            case 'O':
                std::cout << "    [OUTRO]" << std::endl;
                display_block(stream, outro_pattern_, 8);
                if(ad_lib_)
                    std::cout << "ad lib";
                //std::cout << std::endl;
                break;
        }
    }
    if(!structure_.compare("VVVVVVVV"))
        std::cout << "Segmentation fault (core dumped)               (just kidding)" << std::endl;
}

void Song::display_block(std::ostream& stream, const std::vector<Chord>& pattern, int nrepeat)
{
    stream << "| ";
    for(int jj=0; jj<pattern.size(); ++jj)
        stream << pattern[jj] << " | ";
    stream << " x" << nrepeat << std::endl;
}

} // namespace harm
