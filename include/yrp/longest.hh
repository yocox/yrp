#ifndef YOCO_PARSER_LONGEST_HH
#define YOCO_PARSER_LONGEST_HH

#include <array>
#include <algorithm>
#include <iostream>

#include "or.hh"

namespace yrp {

namespace internal {

template <int Index, typename Head, typename ... Tail>
struct longest_match_impl {
    template <typename Parser, typename Array>
    static bool get_match_len(Parser& p, Array& matched_len) {
        typename Parser::iterator orig_pos = p.pos();
        Head::template match(p);
        int len = p.pos() - orig_pos;
        matched_len[Index] = len;
        p.pos(orig_pos);
        longest_match_impl<Index + 1, Tail...>::template get_match_len(p, matched_len);
        return true;
    }
};

template <int Index, typename Last>
struct longest_match_impl<Index, Last> {
    template <typename Parser, typename Array>
    static bool get_match_len(Parser& p, Array& matched_len) {
        typename Parser::iterator orig_pos = p.pos();
        Last::template match(p);
        int len = p.pos() - orig_pos;
        matched_len[Index] = len;
        p.pos(orig_pos);
        return true;
    }
};

}

template <typename ... Rules>
struct longest {
    template <typename Parser>
    static bool match(Parser& p) {
        bool result = internal::longest_match_impl<0, Rules...>::template match(p);
        return result;
    }
};

} // namespace yrp

#endif // YOCO_PARSER_LONGEST_HH


