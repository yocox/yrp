#ifndef YOCO_PARSER_ANY_HH
#define YOCO_PARSER_ANY_HH

#include "seq.hh"
#include "not.hh"

namespace yrp {

struct any
{
    template <typename Parser>
    static bool match(Parser& p) {
        if(p.at_end()) {
            return false;
        } else {
            p.next();
            return true;
        }
    }
};

template <typename Rule>
struct any_but :
    seq<
        not_<Rule>,
        any
    >
{} ;

} // namespace yrp

#endif // YOCO_PARSER_ANY_HH

