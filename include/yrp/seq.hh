#ifndef YOCO_PARSER_SEQ_HH
#define YOCO_PARSER_SEQ_HH

namespace yrp {

namespace internal {

template <typename Head, typename ... Tail>
struct seq_impl {
    template <typename Parser>
    static bool match(Parser& p) {
        return Head::template match(p) && seq_impl<Tail...>::template match(p);
    }
};

template <typename Last>
struct seq_impl<Last> {
    template <typename Parser>
    static bool match(Parser& p) {
        return Last::template match(p);
    }
};

} // namespace internal

template <typename ... Rules>
struct seq{
    template <typename Parser>
    static bool match(Parser& p) {
        typename Parser::iterator orig_pos = p.pos();
        if(internal::seq_impl<Rules...>::template match(p)) {
            return true;
        } else {
            p.pos(orig_pos);
            return false;
        }
    }
};

} // namespace yrp

#endif // YOCO_PARSER_SEQ_HH
