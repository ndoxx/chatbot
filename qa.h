#ifndef QA_H
#define QA_H

#include <string>

namespace harm
{

class QA
{
public:
    QA(const std::string& text,
       const std::string& on_yes,
       const std::string& on_no);

    QA(const std::string& text,
       const std::string& output);

    QA(const std::string& text);

    bool pop();

    inline const std::string& get_user_answer() { return user_answer_; }

private:
    std::string text_;
    std::string on_yes_;
    std::string on_no_;
    std::string user_answer_;
    bool yn_blocking_;
};

} // namespace harm

#endif // QA_H
