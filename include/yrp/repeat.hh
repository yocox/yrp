#ifndef YOCO_PARSER_REPEAT_HH
#define YOCO_PARSER_REPEAT_HH

namespace yrp {

template <typename Rule, int From, int To = From>
struct repeat {
    template <typename Parser>
    static bool match(Parser& p) {
        typename Parser::iterator orig_pos = p.pos();
        for(int i = 0; i < From; ++i) {
            if(!Rule::template match(p)) {
                p.pos(orig_pos);
                return false;
            }
        }
        for(int i = From; i < To; ++i) {
            if(!Rule::template match(p)) {
                break;
            }
        }
        return true;
    }
};

} // namespace yrp

#endif // YOCO_PARSER_REPEAT_HH


