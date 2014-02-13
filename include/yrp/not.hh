#ifndef YOCO_PARSER_NOT_HH
#define YOCO_PARSER_NOT_HH

namespace yrp {

template <typename Rule>
struct not_ {
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
};

} // namespace yrp

#endif // YOCO_PARSER_NOT_HH

