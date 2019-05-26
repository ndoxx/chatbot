#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <regex>
#include <algorithm>
#include <cstring>

#include "stringselector.h"
#include "qa.h"

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

static int rythm_changes_offsets[8]{0, 9, 2, 7, 0, 9, 2, 7};
static int forced_verse_1_offsets[8]{0, 5, 10, 3, 8, 7, 0, 7};
static int forced_verse_2_offsets[8]{7, 0, 7, 0, 7, 0, 7, 0};
static int first_chord_select[4]{5, 8, 4, 0};   // IV, 6b, III, I
static int second_chord_select[4]{0, 5, 10, 3}; // 0 interpreted as "same as first chord", IV, VII, IIIb
static int cadence_1_select[4]{0,7,5,1};        // I, V, iv, IIb
static int cadence_2_select[4]{0,0,0,0};        // I, i, I, I

static int chorus_offsets_1[4]{7,0,7,0};
static int chorus_offsets_2[4]{0,1,3,5};

static int forced_outro_offsets[8]{0, 5, 10, 4, 9, 2, 7, 0}; // | i | iv | VII | III | VI | II | V | I
static int bridge_offsets[4]{7, 7, 7, 7};

static std::vector<int> fifths{0,5,10,3,8,1,6,11,4,9,2,7};
static std::vector<std::string> tones   {"C", "Db", "D", "Eb", "E", "F", "Gb",  "G",  "Ab", "A", "Bb", "B"};
static std::vector<std::string> tones_fr{"Do","Réb","Ré","Mib","Mi","Fa","Solb","Sol","Lab","La","Sib","Si"};

std::string to_lower(const std::string& str)
{
    std::string ret(str);
    std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
    return ret;
}

int to_chord(int root, int offset, bool minor)
{
    int ret = (root + offset - (minor ? 3 : 0)) % 12;

    if(ret<0)
        ret += 12;

    return ret;
}

void display_block(int root, int* array, int array_sz, int nrepeat, bool minor)
{
    for(int ii=0; ii<nrepeat; ++ii)
    {
        std::cout << "| ";
        for(int jj=0; jj<array_sz; ++jj)
        {
            int chord = to_chord(root, array[jj], minor);

            // if minor, force I and IV to i and iv
            if(minor && (array[jj]==0 || array[jj]==5))
                std::cout << to_lower(tones_fr[chord]);
            else
                std::cout << tones_fr[chord];
            std::cout << " | ";
        }
        std::cout << std::endl;
    }
}

void parse_descriptor(SongDescriptor& descriptor, std::default_random_engine entropy)
{
    // * Generate verse
    int verse_offsets[8];
    if(descriptor.rythm_changes)
    {
        descriptor.minor = false;
        descriptor.root = 0;
        memcpy(verse_offsets, rythm_changes_offsets, 8*sizeof(int));
    }
    else if(descriptor.force_verse)
    {
        if(descriptor.force_verse == 1)
            memcpy(verse_offsets, forced_verse_1_offsets, 8*sizeof(int));
        if(descriptor.force_verse == 2)
            memcpy(verse_offsets, forced_verse_2_offsets, 8*sizeof(int));
    }
    else
    {
        verse_offsets[0] = first_chord_select[descriptor.first_chord];
        verse_offsets[1] = second_chord_select[descriptor.second_chord];
        verse_offsets[2] = cadence_1_select[descriptor.cadence];
        verse_offsets[3] = cadence_2_select[descriptor.cadence];

        // Repeat 4 bars
        for(int ii=0; ii<4; ++ii)
            verse_offsets[4+ii] = verse_offsets[ii];
    }

    if(descriptor.force_4_chords)
    {
        display_block(descriptor.root, verse_offsets, 4, 4, descriptor.minor);
        return;
    }

    // * Generate intro
    int intro_offsets[4];
    int intro_length = 1;
    if(descriptor.intro_is_root_x8)
    {
        for(int ii=0; ii<4; ++ii)
            intro_offsets[ii] = 0;
    }
    else
    {
        std::uniform_int_distribution<int> intro_type_dis(0,1);
        std::uniform_int_distribution<int> intro_len_dis(0,2);
        bool intro_type = bool(intro_type_dis(entropy));
        intro_length = intro_len_dis(entropy)+1;

        intro_offsets[0] = intro_type ? verse_offsets[0] : verse_offsets[0];
        intro_offsets[1] = intro_type ? verse_offsets[1] : verse_offsets[0];
        intro_offsets[2] = intro_type ? verse_offsets[0] : verse_offsets[1];
        intro_offsets[3] = intro_type ? verse_offsets[1] : verse_offsets[1];
    }

    // * Generate chorus
    std::uniform_int_distribution<int> chorus_type_dis(0,2);
    int chorus_type = chorus_type_dis(entropy);
    int chorus_offsets[4];
    switch(chorus_type)
    {
        case 0:
            memcpy(chorus_offsets, chorus_offsets_1, 4*sizeof(int));
            break;

        case 1:
            chorus_offsets[0] = verse_offsets[2];
            chorus_offsets[1] = verse_offsets[3];
            chorus_offsets[2] = verse_offsets[2];
            chorus_offsets[3] = verse_offsets[3];
            break;

        case 2:
            memcpy(chorus_offsets, chorus_offsets_2, 4*sizeof(int));
            break;
    }

    // * Generate outro
    bool chorus_ad_lib = false;
    int outro_offsets[8];
    if(descriptor.harmonic_march)
        memcpy(outro_offsets, forced_outro_offsets, 8*sizeof(int));
    else
    {
        std::uniform_int_distribution<int> outro_type_dis(0,2);
        int outro_type = outro_type_dis(entropy);
        switch(outro_type)
        {
            case 0:
                for(int ii=0; ii<8; ++ii)
                    outro_offsets[ii] = intro_offsets[ii%4];
                break;
            case 1:
                for(int ii=0; ii<8; ++ii)
                    outro_offsets[ii] = verse_offsets[ii];
                break;
            case 2:
                for(int ii=0; ii<8; ++ii)
                    outro_offsets[ii] = chorus_offsets[ii%4];
                chorus_ad_lib = true;
                break;
        }
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
                display_block(descriptor.root, intro_offsets, 4, intro_length, descriptor.minor);
                std::cout << std::endl;
                break;

            case 'C':
                std::cout << "[CHORUS]" << std::endl;
                display_block(descriptor.root, chorus_offsets, 4, 4, descriptor.minor);
                std::cout << std::endl;
                break;

            case 'B':
                if(descriptor.has_bridge)
                {
                    std::cout << "[BRIDGE]" << std::endl;
                    display_block(descriptor.root, bridge_offsets, 4, 1, descriptor.minor);
                    std::cout << std::endl;
                }
                break;

            case 'V':
                std::cout << "[VERSE]" << std::endl;
                display_block(descriptor.root, verse_offsets, 8, 4, descriptor.minor);
                std::cout << std::endl;
                break;

            case 'O':
                std::cout << "[OUTRO]" << std::endl;
                display_block(descriptor.root, outro_offsets, 8, 4, descriptor.minor);
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

    descriptor.force_4_chords = q1.pop();
    q2.pop();

    // * Sélection de la note de la première mesure
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

    // * Sélection de la cadence du verse
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

    // * Anatole en Do ou bien modif fdtl
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

    // * Second tone
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
            descriptor.force_verse = true;
    }


    // * ------------------------- PARSING -------------------------
    /*descriptor.first_chord = 0;
    descriptor.second_chord = 2;
    descriptor.cadence = 1;

    descriptor.root = 0;
    descriptor.minor = true;
    descriptor.has_bridge = true;

    descriptor.force_4_chords = false;*/

    parse_descriptor(descriptor, entropy);

    return 0;
}

