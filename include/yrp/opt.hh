#ifndef YOCO_PARSER_OPT_HH
#define YOCO_PARSER_OPT_HH

namespace yrp {

template <typename Rule>
struct opt {
    template <typename Parser>
    static bool match(Parser& p) {
        Rule::template match(p);
        return true;
    }
};

} // namespace yrp

#endif // YOCO_PARSER_OPT_HH

