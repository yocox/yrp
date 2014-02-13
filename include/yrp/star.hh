#ifndef YOCO_PARSER_STAR_HH
#define YOCO_PARSER_STAR_HH

#include "seq.hh"

namespace yrp {

template <typename Rule>
struct star {
    template <typename Parser>
    static bool match(Parser& p) {
        while(Rule::template match(p));
        return true;
    }
};

template <typename Rule>
struct plus : seq<Rule, star<Rule>>
{ };

} // namespace yrp

#endif // YOCO_PARSER_STAR_HH

