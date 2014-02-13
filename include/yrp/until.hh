#ifndef YOCO_PARSER_UNTIL_HH
#define YOCO_PARSER_UNTIL_HH

#include "at.hh"

namespace yrp {

template <typename Rule>
struct until_pass {
    template <typename Parser>
    static bool match(Parser& p) {
        typename Parser::iterator orig_pos = p.pos();
        while(true) {
            if(p.at_end()) {
                p.pos(orig_pos);
                return false;
            }
            if(Rule::template match(p)) {
                return true;
            } else {
                p.next();
            }
        }
    }
} ;

template <typename Rule>
struct until_at : until_pass<at<Rule>> {};

} // namespace yrp

#endif // YOCO_PARSER_UNTIL_HH

