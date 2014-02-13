#ifndef YOCO_PARSER_END_OF_INPUT_HH
#define YOCO_PARSER_END_OF_INPUT_HH

namespace yrp {

struct end_of_input {
    template <typename Parser>
    static bool match(Parser& p) {
        return p.at_end();
    }
};

} // namespace yrp

#endif // YOCO_PARSER_END_OF_INPUT_HH

