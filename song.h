#ifndef SONG_H
#define SONG_H

#include <vector>
#include <string>
#include <iostream>

struct Chord
{
    Chord(int base, bool is_minor):
    base(base),
    is_minor(is_minor)
    {

    }

    friend std::ostream& operator<<(std::ostream& stream, const Chord& chord);

    int base;
    bool is_minor;
};

struct Step
{
    Step(int offset, bool is_minor=false):
    offset(offset),
    is_minor(is_minor)
    {

    }

    Chord to_chord(int root, bool song_is_minor) const;

    int offset;
    bool is_minor;
};

// Literal operator to handle musical notations for steps, eg. iii, VIb
Step operator "" _s(const char* notation, size_t);

class Song
{
public:
    typedef enum
    {
        INTRO,
        VERSE,
        CHORUS,
        BRIDGE,
        OUTRO
    } Section;

    Song(const std::vector<std::vector<Step>>& abstract_song,
         const std::string& structure,
         int root_chord,
         int intro_length,
         bool song_is_minor,
         bool has_bridge,
         bool ad_lib);

    void display(std::ostream& stream);

    inline const std::vector<Chord>& get_verse() const { return verse_pattern_; }

protected:
    void display_block(std::ostream& stream, const std::vector<Chord>& pattern, int nrepeat);

private:
    std::vector<Chord> intro_pattern_,
                       chorus_pattern_,
                       verse_pattern_,
                       bridge_pattern_,
                       outro_pattern_;
    std::string structure_;
    int intro_length_;
    bool has_bridge_;
    bool ad_lib_;
};

#endif // SONG_H
