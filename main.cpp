#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <chrono>
#include <regex>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <locale>

#include "stringselector.h"
#include "qa.h"
#include "hash.h"

struct SongDescriptor
{
    // Harmony
    int first_chord = 0;           // 0: IV, 1: 6b, 2: III, 3: I
    int second_chord = 0;          // 0: =first_chord, 1: IV, 2: VII, 3: IIIb
    int cadence = 0;               // 0: root, 1: (V-i), 2: (iv-I), 3: (IIb-I)
    int root = 0;                  // root chord
    bool minor = false;            // true: song is minor, output is translated to relative minor
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

    Chord to_chord(int root, bool song_is_minor)
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

    int offset;
    bool is_minor;
};

static std::locale locc("C");

bool is_lower(const std::string& str)
{
    for(int ii=0; ii<str.size(); ++ii)
    {
        if(!std::islower(str[ii], locc))
            return false;
    }
    return true;
}

std::unordered_map<std::string, int> step_notation =
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

Step operator "" _s(const char* notation, size_t)
{
    std::string not_str(notation);
    bool is_minor = is_lower(not_str);
    // Force lower case
    std::transform(not_str.begin(), not_str.end(), not_str.begin(), ::tolower);
    int offset = step_notation[not_str];

    return Step(offset, is_minor);
}

static std::vector<int> fifths{0,5,10,3,8,1,6,11,4,9,2,7};
static std::vector<std::string> tones   {"C", "Db", "D", "Eb", "E", "F", "Gb",  "G",  "Ab", "A", "Bb", "B"};
static std::vector<std::string> tones_fr{"Do","Réb","Ré","Mib","Mi","Fa","Solb","Sol","Lab","La","Sib","Si"};

std::string to_lower(const std::string& str)
{
    std::string ret(str);
    std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
    return ret;
}

std::ostream& operator<<(std::ostream& stream, const Chord& chord)
{
    stream << (chord.is_minor ? to_lower(tones_fr[chord.base]) : tones_fr[chord.base]);
    return stream;
}

void display_block(const std::vector<Chord> pattern, int nrepeat)
{
    for(int ii=0; ii<nrepeat; ++ii)
    {
        std::cout << "| ";
        for(int jj=0; jj<pattern.size(); ++jj)
            std::cout << pattern[jj] << " | ";
        std::cout << std::endl;
    }
}

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


void generate_verse(std::vector<Step>& steps, SongDescriptor& descriptor)
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
        steps.push_back(second_chord_select_steps[descriptor.second_chord]);
        steps.push_back(cadence_1_select_steps[descriptor.cadence]);
        steps.push_back(cadence_2_select_steps[descriptor.cadence]);

        // Repeat 4 bars
        for(int ii=0; ii<4; ++ii)
            steps.push_back(steps[ii]);
    }
}

int generate_intro(std::vector<Step>& steps,
                   const std::vector<Step>& verse,
                   SongDescriptor& descriptor,
                   std::default_random_engine entropy)
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
        bool intro_type = bool(intro_type_dis(entropy));
        intro_length = intro_len_dis(entropy)+1;

        steps.push_back(verse[0]);
        steps.push_back(intro_type ? verse[1] : verse[0]);
        steps.push_back(intro_type ? verse[0] : verse[1]);
        steps.push_back(verse[1]);
    }

    return intro_length;
}

void generate_chorus(std::vector<Step>& steps,
                     const std::vector<Step>& verse,
                     SongDescriptor& descriptor,
                     std::default_random_engine entropy)
{
    std::uniform_int_distribution<int> chorus_type_dis(0,2);
    int chorus_type = chorus_type_dis(entropy);
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

bool generate_outro(std::vector<Step>& steps,
                    const std::vector<Step>& intro,
                    const std::vector<Step>& verse,
                    const std::vector<Step>& chorus,
                    SongDescriptor& descriptor,
                    std::default_random_engine entropy)
{
    bool chorus_ad_lib = false;

    if(descriptor.harmonic_march)
        std::copy(forced_outro_steps.begin(), forced_outro_steps.end(), std::back_inserter(steps));
    else
    {
        std::uniform_int_distribution<int> outro_type_dis(0,2);
        int outro_type = outro_type_dis(entropy);
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

void parse_descriptor(SongDescriptor& descriptor, std::default_random_engine entropy)
{
    std::vector<Step> intro_steps,
                      chorus_steps,
                      verse_steps,
                      outro_steps;

    // * Generate verse
    generate_verse(verse_steps, descriptor);

    // * Generate intro
    int intro_length = generate_intro(intro_steps, verse_steps, descriptor, entropy);

    // * Generate chorus
    generate_chorus(chorus_steps, verse_steps, descriptor, entropy);

    // * Generate outro
    bool ad_lib = generate_outro(outro_steps, intro_steps, verse_steps, chorus_steps, descriptor, entropy);

    // * Convert to actual chords
    std::vector<Chord> intro_pattern,
                       chorus_pattern,
                       verse_pattern,
                       bridge_pattern,
                       outro_pattern;

    for(int ii=0; ii<8; ++ii)
    {
        if(ii<4)
        {
            intro_pattern.push_back(intro_steps[ii].to_chord(descriptor.root, descriptor.minor));
            chorus_pattern.push_back(chorus_steps[ii].to_chord(descriptor.root, descriptor.minor));
            bridge_pattern.push_back(bridge_steps[ii].to_chord(descriptor.root, descriptor.minor));
        }
        verse_pattern.push_back(verse_steps[ii].to_chord(descriptor.root, descriptor.minor));
        outro_pattern.push_back(outro_steps[ii].to_chord(descriptor.root, descriptor.minor));
    }

    // * Stop here if only 4 chords need to be displayed
    if(descriptor.force_4_chords)
    {
        display_block(verse_pattern, 32);
        return;
    }

    // * Structure
    std::uniform_int_distribution<int> structure_type_dis(0,2);
    std::string structure;
    switch(structure_type_dis(entropy))
    {
        case 0:
            structure = "CVCBVCVO";
            break;

        case 1:
            structure = "IVCVCBCO";
            break;

        case 2:
            structure = "IVCVCBVCO";
            break;
    }

    // * Display
    for(int ii=0; ii<structure.size(); ++ii)
    {
        switch(structure[ii])
        {
            case 'I':
                std::cout << "[INTRO]" << std::endl;
                display_block(intro_pattern, intro_length);
                std::cout << std::endl;
                break;

            case 'C':
                std::cout << "[CHORUS]" << std::endl;
                display_block(chorus_pattern, 4);
                std::cout << std::endl;
                break;

            case 'B':
                if(descriptor.has_bridge)
                {
                    std::cout << "[BRIDGE]" << std::endl;
                    display_block(bridge_pattern, 4);
                    std::cout << std::endl;
                }
                break;

            case 'V':
                std::cout << "[VERSE]" << std::endl;
                display_block(verse_pattern, 4);
                std::cout << std::endl;
                break;

            case 'O':
                std::cout << "[OUTRO]" << std::endl;
                display_block(outro_pattern, 8);
                std::cout << std::endl;
                break;
        }
    }
}

int main(int argc, char** argv)
{
    typedef std::chrono::high_resolution_clock clk;
    clk::time_point beginning = clk::now();
    int SEED = beginning.time_since_epoch().count();

    std::string cb_invite("[Harmonizer] > ");

    std::cout << "Bonjour ! Bonsoir ! On est en $year avec Internet,"
              << "on ne sait plus vraiment. Je suis l'Harmonizer !" << std::endl;

    StringSelector selector(SEED);
    selector.add_string("Je t'aide à connecter tes vibes quantiques et celles de tes amis vers un mood vachement spirituel.", 80);
    selector.add_string("Je te lèche le bas-côté en diagonale.", 15);
    selector.add_string(" [UNDEFINED:Charset[MosquitoRampage]::null and non-advented. Exception 3310 UNBREAKABLE", 3);
    selector.add_string("Il est l'heure de se lever, frérot. Prends du pain et du vin pour la route. Dégaine-toi du rêve anxieux des bien-assis.", 2);

    std::cout << selector.generate_string() << std::endl;

    std::smatch m;
    SongDescriptor descriptor;

    QA q1("Est-ce que tu regardes des émissions de télé avec Cyril Hanouna dedans qui présente, dis ?",
          "Quatre accords en tout pas plus !",
          "Tu es vaillant et hardi, comme les musiques tyroliennes.");

    QA q2("Vous avez fumé ? ça se voit que vous avez fumé, je m'en fous mais te fous pas de ma gueule non plus maintenant.",
          "Comme dit, tu vois, je m'en fiche, c'est pour toi, je me fais du souci ces derniers temps.");

    QA q3("Est-ce que vous vous sentez taquins ?",
          "Voyons-voir...");

    QA q4("Dis-moi une note.");

    QA q5("Année de naissance ?");

    QA q6("Écris-moi un petit mot.");

    QA q7("Est-ce que ça va jammer sévère si je te donne une bonne grille ?",
          "Ok.");

    QA q8("J'ai presque fini de concocter votre suite d'accords. Il me faut ton autorisation pour tes données personnelles. Les utiliser, juste comme ça au cas où. Tape ton adresse email suivie du mot 'Oui' comme ça je te file une super suite d'accords !");

    QA q9("J'ai une suite d'accords non-aléatoire, de la bombe. Si tu veux fais-toi plaiz en écrivant \"SUITE AU 81212\".");

    std::default_random_engine entropy(SEED);

    // * Troll a bit
    descriptor.force_4_chords = q1.pop();
    q2.pop();

    // * Select verse first chord
    if(q3.pop())
    {
        std::uniform_int_distribution<int> choose(0,2);
        descriptor.first_chord = choose(entropy);
        switch(descriptor.first_chord)
        {
            case 0:
                std::cout << cb_invite << "On va partir sur el famoso, la spéciale, tu m'en diras des nouvelles" << std::endl;
                break;
            case 1:
                std::cout << cb_invite << "Je vois bien quelque chose de boisé, un peu musqué, avec beaucoup de tanins, ainsi que des gens qui sont morts, tout le temps" << std::endl;
                break;
            case 2:
                std::cout << cb_invite << "D'accord. J'ai choisi d'implémenter une astuce harmonique yougoslave traditionnelle" << std::endl;
                break;
        }
    }
    else
    {
        std::cout << cb_invite << "OK, petit joueur, je retiens." << std::endl;
        descriptor.first_chord = 3;
    }

    // * Select verse cadence
    q4.pop();
    auto&& chord_ua = q4.get_user_answer();

    std::regex reg_opt1("(do|ré|re|mi)");
    std::regex reg_opt2("(fa|sol|la|si)");
    std::regex reg_opt3("(ut)");
    bool opt_1 = std::regex_search(chord_ua, m, reg_opt1);
    bool opt_2 = std::regex_search(chord_ua, m, reg_opt2);
    bool opt_3 = std::regex_search(chord_ua, m, reg_opt3);

    if(opt_1)
        descriptor.cadence = 1;
    else if(opt_2)
        descriptor.cadence = 2;
    else if(opt_3)
        descriptor.cadence = 3;
    else
        descriptor.cadence = 0;

    switch(descriptor.cadence)
    {
        case 0:
            std::cout << cb_invite << "Cadence fondante" << std::endl;
            break;

        case 1:
            std::cout << cb_invite << "Cadence parfaite" << std::endl;
            break;

        case 2:
            std::cout << cb_invite << "Cadence plagale" << std::endl;
            break;

        case 3:
            std::cout << cb_invite << "Cadence Rock'n'Roll" << std::endl;
            break;
    }

    // * Rythm changes (C) or root chord pattern
    while(true)
    {
        q5.pop();
        auto&& year_ua = q5.get_user_answer();
        std::regex reg_year("(\\d{4})");

        if(std::regex_search(year_ua, m, reg_year))
        {
            std::string year_str(m[0]);
            int year = std::stoi(year_str);
            if(year>2005)
                descriptor.rythm_changes = true;
            else
                descriptor.root = fifths[(year-1)%12];
            break;
        }

        std::cout << cb_invite << "Année atteinte d'une incapacité." << std::endl;
    }

    std::uniform_int_distribution<int> choose(0,1);
    descriptor.minor = bool(choose(entropy));
    std::cout << cb_invite << "Ok, on sera en " << tones_fr[descriptor.root] << (descriptor.minor ? "mineur" : "") << "." << std::endl;


    // * Bridge
    q6.pop();
    auto&& le_ptit_mot = q6.get_user_answer();
    if(le_ptit_mot.size()<6)
        descriptor.has_bridge = true;
    else if(le_ptit_mot.size()<25)
        descriptor.intro_is_root_x8 = true;
    else
        descriptor.harmonic_march = true;

    // * Select verse second chord
    if(q7.pop())
    {
        std::uniform_int_distribution<int> choose(1,3);
        descriptor.second_chord = choose(entropy);
    }
    else
        descriptor.second_chord = 0;


    // * Step 1: Get e-mail address, Step 2: Step 3: profit
    q8.pop();
    auto&& email_ua = q8.get_user_answer();

    std::regex reg_email("(.+@.+\\..+?)\\sOui");
    if(std::regex_search(email_ua, m, reg_email))
    {
        descriptor.email = m[1];
        std::cout << cb_invite << "Let's go, on y est presque !" << std::endl;
    }
    else
    {
        std::cout << cb_invite << "Il va me falloir plus de précision, petit.e rebelle !" << std::endl;

        // * Force verse?
        q9.pop();
        auto&& suite_ua = q9.get_user_answer();
        if(!suite_ua.compare("SUITE AU 81212"))
            descriptor.force_verse = int(!suite_ua.compare("SUITE AU 81212"));
    }

    // * ------------------------- PARSING -------------------------
    /*descriptor.first_chord = 0;
    descriptor.second_chord = 2;
    descriptor.cadence = 1;

    descriptor.root = 0;
    descriptor.minor = false;
    descriptor.has_bridge = true;

    descriptor.force_4_chords = false;*/

    std::cout << std::endl
              << cb_invite << "Voici, je vous ai bricolé un truc trop fresh: "
              << std::endl << std::endl;

    parse_descriptor(descriptor, entropy);

    return 0;
}

