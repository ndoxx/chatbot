
Structures possibles :

CVC[B]VCVO
IVCVC[B]CO
IVCVC[B]VCO


I:0,II:2, iii:3,III:4,IV:5,V:7,6b:8,VI:9,VII:10

```cpp
struct SongDescriptor
{
    // Harmony
    int first_chord = 0;           // 0: IV, 1: 6b, 2: III, 3: I
    int second_chord = 0;          // 0: =first_chord, 1: IV, 2: VII, 3: IIIb
    int cadence = 0;               // 0: root, 1: (V-i), 2: (iv-I), 3: (IIb-I)
    int root = 0;                  // root chord
    bool minor = false;            // true: output is translated to relative minor
    // Structure
    bool force_4_chords = false;   // Whole song is 32x same 4 chords
    bool rythm_changes = false;    // Verse = Anatole en Do |C|a|d|G|
    bool has_bridge = false;       // Bridge after second chorus
    bool intro_is_root_x8 = false; // Force intro to be 8 bars of root chord
    bool harmonic_march = false;   // Force outro to take the form of an harmonic march i | iv | VII | III || VI | II | V | I x4
    int force_verse = 0;           // Force verse to be 1: C | f | Bb | Eb || D▲| G7 | C7 | G7 x2   2: | V | i | x16
    // Big data
    std::string email;             // For profit
};
```
