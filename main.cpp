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

#include <cstdint>
#include <stdexcept>

#include "stringselector.h"
#include "qa.h"
#include "song_factory.h"
#include "song.h"

using namespace harm;

#define DBG_FORCE_DESCRIPTOR__

static std::vector<int> fifths{0,5,10,3,8,1,6,11,4,9,2,7};

void generate_random_descriptor(SongDescriptor& descriptor, std::default_random_engine& entropy)
{
    std::uniform_int_distribution<int> choose_2(0,1);
    std::uniform_int_distribution<int> choose_4(0,3);
    std::uniform_int_distribution<int> choose_12(0,11);
    std::uniform_int_distribution<int> choose_20(0,19);

    descriptor.first_chord = choose_4(entropy);
    descriptor.second_chord = choose_4(entropy);
    descriptor.cadence = choose_4(entropy);

    descriptor.root = choose_12(entropy);
    descriptor.minor = bool(choose_2(entropy));
    descriptor.has_bridge = bool(choose_2(entropy));

    descriptor.force_4_chords = bool(choose_12(entropy)==0);
    descriptor.rythm_changes = bool(choose_12(entropy)==0);
    descriptor.intro_is_root_x8 = bool(choose_12(entropy)==0);
    descriptor.harmonic_march = bool(choose_12(entropy)==0);
    descriptor.force_verse = (choose_12(entropy)==0 ? choose_2(entropy)+1 : 0);
}


// Helper functions to parse program arguments
static const char* get_cmd_option(const char** begin, const char ** end, const std::string & option)
{
    const char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

static bool cmd_option_exists(const char** begin, const char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

enum class HMode: std::uint8_t
{
    WIZARD,
    RANDOM
};

int main(int argc, const char** argv)
{
    // * Parse program arguments
    HMode mode;
    int n_songs = 1;
    if(cmd_option_exists(argv, argv + argc, "--random"))
        mode = HMode::RANDOM;
    else
        mode = HMode::WIZARD;

    if(mode != HMode::WIZARD && cmd_option_exists(argv, argv + argc, "-n"))
    {
        const char* n_songs_c = get_cmd_option(argv, argv + argc, "-n");
        try
        {
            n_songs = std::stoi(n_songs_c);
        }
        catch(const std::invalid_argument& e)
        {
            std::cerr << "Argument -n atteint d'une incapacité: " << e.what() << std::endl;
        }
    }

    // Random stuff (literally)
    typedef std::chrono::high_resolution_clock clk;
    clk::time_point beginning = clk::now();
    int SEED = beginning.time_since_epoch().count();
    std::default_random_engine entropy(SEED);
    std::uniform_int_distribution<int> choose_2(0,1);

    // Some locals
    std::string cb_invite("[Harmonizer] > "); // Invite used by chatbot
    std::smatch m; // For regex matching
    SongDescriptor descriptor;

    // * Say hello
    StringSelector selector(SEED);
    if(choose_2(entropy)==0)
    {
        std::cout << cb_invite
                  << "Bonjour ! Bonsoir ! On est en $year avec Internet, "
                  << "on ne sait plus vraiment. Je suis l'Harmonizer !"
                  << std::endl;

        selector.add_string("Je t'aide à connecter tes vibes quantiques et celles de tes amis vers un mood vachement spirituel.", 80);
        selector.add_string("Je te lèche le bas-côté en diagonale.", 15);
        selector.add_string(" [UNDEFINED:Charset[MosquitoRampage]::null and non-advented. Exception 3310 UNBREAKABLE", 3);
        selector.add_string("Il est l'heure de se lever, frérot. Prends du pain et du vin pour la route. Dégaine-toi du rêve anxieux des bien-assis.", 2);

        std::cout << cb_invite << selector.generate_string() << std::endl;
    }
    else
    {
        selector.add_string("carpes",        10);
        selector.add_string("grenouilles",   10);
        selector.add_string("galinettes",    10);
        selector.add_string("phacochères",   10);
        selector.add_string("trilobites",    10);
        selector.add_string("mille-pattes",  10);
        selector.add_string("rhododendrons", 10);
        selector.add_string("topinambours",  10);
        selector.add_string("fiottes",       10);
        selector.add_string("cacas",         10);

        std::cout << cb_invite
                  << "Salut les " << selector.generate_string() << " c'est moi, l'Harmonizer !"
                  << std::endl
                  << "Je vais vous poser neuf questions et grâce à elles, on va pouvoir générer une chanson pop en mode easy money, easy life. Fais bien attention à tes réponses, car je suis capricieux et vulnérable à toutes les attaques de type injection SQL : à ne pas tenter ! Réponds-y en conscience, en plénitude et en équanimité bouddhique."
                  << std::endl;
    }

    if(mode == HMode::WIZARD)
    {
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
        std::cout << std::endl
                  << cb_invite << "Voici, je vous ai bricolé un truc trop fresh: "
                  << std::endl << std::endl;

        SongFactory factory(SEED);
        Song* song = factory.generate_song(descriptor);
        song->display(std::cout);

        std::cout << std::endl
                  << "Joue ça et envoie ton résultat audio ou vidéo à Euterpex01@gmail.com !"
                  << std::endl;
        if(descriptor.email.size())
        {
            std::cout << "Un spam a été envoyé à l'adresse "
                      << descriptor.email
                      << " ! Merci de votre crédulité." << std::endl;
        }

        delete song;
    }
    else if(mode == HMode::RANDOM)
    {
        // * Prepare N different song descriptors
        std::map<std::size_t, SongDescriptor> descriptors;
        while(descriptors.size()!=n_songs)
        {
            generate_random_descriptor(descriptor, entropy);
            std::size_t h1 = std::hash<SongDescriptor>{}(descriptor);
            if(descriptors.find(h1) == descriptors.end())
                descriptors.insert(std::make_pair(h1, descriptor));
        }

        // * Generate and display songs
        SongFactory factory(SEED);
        for(auto&& [key, desc]: descriptors)
        {
            std::cout << "op.42 No." << key << std::endl;
            Song* song = factory.generate_song(desc);
            song->display(std::cout);
            std::cout << std::endl
                      << "-------------------------------------------"
                      << std::endl << std::endl;
            delete song;
        }
    }


    return 0;
}

