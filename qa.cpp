#include <iostream>

#include "qa.h"

QA::QA(const std::string& text,
       const std::string& on_yes,
       const std::string& on_no):
text_(text),
on_yes_(on_yes),
on_no_(on_no),
yn_blocking_(true)
{

}

QA::QA(const std::string& text,
       const std::string& output):
text_(text),
on_yes_(output),
on_no_(output),
yn_blocking_(true)
{

}

QA::QA(const std::string& text):
text_(text),
yn_blocking_(false)
{

}


bool QA::pop()
{
    std::cout << std::endl << "[Harmonizer] > " << text_ << std::endl;

    if(yn_blocking_)
    {
        while(true)
        {
            std::getline(std::cin, user_answer_);

            // "oui" in user string
            if(user_answer_.find("non")!=std::string::npos)
            {
                std::cout << "[Harmonizer] > " << on_no_ << std::endl;
                return false;
            }
            // "non" in user string
            else if(user_answer_.find("oui")!=std::string::npos)
            {
                std::cout << "[Harmonizer] > " << on_yes_ << std::endl;
                return true;
            }
            else
            {
                std::cout << "[Harmonizer] > " << "Entrée atteinte d'une incapacité." << std::endl;
            }
        }
    }
    else
    {
        std::getline(std::cin, user_answer_);
        return true;
    }

    return false;
}
