#ifndef STRINGSELECTOR_H
#define STRINGSELECTOR_H

#include <vector>
#include <string>


class StringSelector
{
public:
    StringSelector(int seed);

    void add_string(const std::string& str, int prob);

    const std::string& generate_string();

private:
    std::vector<std::string> strings_;
    std::vector<float> probs_;
    int count_;
    int seed_;
};

#endif // STRINGSELECTOR_H
