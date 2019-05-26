#include <random>
#include <iostream>
#include "stringselector.h"


StringSelector::StringSelector(int seed):
seed_(seed)
{

}

void StringSelector::add_string(const std::string& str, int prob)
{
    if(count_+prob>100)
        prob = 0;

    strings_.push_back(str);
    probs_.push_back(prob / 100.f);
}

const std::string& StringSelector::generate_string()
{
    std::default_random_engine entropy(seed_);
    std::discrete_distribution<int> choose(probs_.begin(), probs_.end());

    return strings_[choose(entropy)];
}
