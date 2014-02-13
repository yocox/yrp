#ifndef YOCO_PARSER_AT_HH
#define YOCO_PARSER_AT_HH

namespace yrp {

template <typename Rule>
struct at {
    template <typename Parser>
    static bool match(Parser& p) {
        typename Parser::iterator orig_pos = p.pos();
        if(Rule::template match(p)) {
            p.pos(orig_pos);
            return true;
        } else {
            return false;
        }
    }
} ;

template <typename Rule>
struct not_at {
    template <typename Parser>
    static bool match(Parser& p) {
        typename Parser::iterator orig_pos = p.pos();
        if(Rule::template match(p)) {
            p.pos(orig_pos);
            return false;
        } else {
            return true;
        }
    }
} ;

} // namespace yrp

#endif // YOCO_PARSER_AT_HH

