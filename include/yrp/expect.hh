#ifndef YOCO_PARSER_EXPECT_HH
#define YOCO_PARSER_EXPECT_HH

#include <typeinfo>
#include <stdexcept>

namespace yrp {

template <typename Rule, typename Exception>
struct expect_except {
    template <typename Parser>
    static bool match(Parser& p) {
        if(Rule::template match(p)) {
            return true;
        } else {
            throw Exception();
            return false;
        }
    }
};

template <typename Rule>
struct expect {
    template <typename Parser>
    static bool match(Parser& p) {
        if(Rule::template match(p)) {
            return true;
        } else {
            throw std::logic_error(typeid(Rule).name());
            return false;
        }
    }
};

} // namespace yrp

#endif // YOCO_PARSER_EXPECT_HH

