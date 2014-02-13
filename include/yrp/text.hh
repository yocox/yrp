#ifndef YOCO_PARSER_TEXT_HH
#define YOCO_PARSER_TEXT_HH

#include "or.hh"
#include "not.hh"
#include "seq.hh"
#include "star.hh"

namespace yrp {

namespace detail {

template <wchar_t C, wchar_t... Tail>
struct is_one {
    static bool is(const wchar_t w) {
        if(w == C) {
            return true;
        } else {
            return is_one<Tail...>::is(w);
        }
    }
} ;

template <wchar_t LastC>
struct is_one<LastC> {
    static bool is(const wchar_t w) {
        return w == LastC;
    }
} ;

template <wchar_t From, wchar_t To, wchar_t... Tail>
struct in_range {
    static bool in(const wchar_t w) {
        if(w >= From && w <= To) {
            return true;
        } else {
            return in_range<Tail...>::in(w);
        }
    }
} ;

template <wchar_t LastFrom, wchar_t LastTo>
struct in_range<LastFrom, LastTo> {
    static bool in(const wchar_t w) {
        return w >= LastFrom && w <= LastTo;
    }
} ;

} // namespace detail

struct any_char {
    template <typename Parser>
    static bool match(Parser &p) {
        if(p.at_end()) return false;
        p.next();
        return true;
    }
} ;

template <wchar_t... CharList>
struct char_ {
    template <typename Parser>
    static bool match(Parser &p) {
        if(p.at_end()) return false;
        if(detail::is_one<CharList...>::is(p.elem())) {
            p.next();
            return true;
        } else {
            return false;
        }
    }
} ;

template <wchar_t ... CharList>
struct not_char {
    template <typename Parser>
    static bool match(Parser &p) {
        if(p.at_end()) return false;
        if(!detail::is_one<CharList...>::is(p.elem())) {
            p.next();
            return true;
        } else {
            return false;
        }
    }
} ;

template <wchar_t... RangeList>
struct range_char {
    template <typename Parser>
    static bool match(Parser &p) {
        if(p.at_end()) return false;
        if(detail::in_range<RangeList...>::in(p.elem())) {
            p.next();
            return true;
        } else {
            return false;
        }
    }
} ;

template <wchar_t... CharList>
struct str : seq<char_<CharList>...> {};

constexpr wchar_t NEW_LINE = L'\n';
constexpr wchar_t HORIZONTAL_TAB = L'\t';
constexpr wchar_t VERTICAL_TAB = L'\v';
constexpr wchar_t FORM_FEED = L'\f';
constexpr wchar_t WHITESPACE = L' ';

struct new_line_char   : char_<NEW_LINE> {};
struct sign_char       : char_<L'+', L'-'> {};
struct digit_char      : range_char<L'0', L'9'> {};
struct bin_char        : char_<L'0', L'1'> {};
struct oct_char        : range_char<L'0', L'7'> {};
struct hex_char        : range_char<L'0', L'9', L'a', L'f', L'A', L'F'> {};
struct lower_char      : range_char<L'a', L'z'> {};
struct upper_char      : range_char<L'A', L'Z'> {};
struct alpha_char      : range_char<L'a', L'z', L'A', L'Z'> {};
struct alpha_num_char  : range_char<L'a', L'z', L'A', L'Z', '0', '9'> {};
struct head_char       : or_<alpha_char, char_<L'_'>> {};
struct word_char       : or_<alpha_num_char, char_<L'_'>> {};
struct id_word         : seq<head_char, star<word_char>> {};
struct whitespace_char : char_<WHITESPACE, HORIZONTAL_TAB> {};
struct whitespace_seq  : plus<whitespace_char> {};

template <wchar_t Quote>
struct quoted_escape_char :
    or_<
        seq<char_<L'\\'>, any_char>,
        not_char<Quote, '\\'>
    >
{};

template <wchar_t Quote>
struct quoted_str : seq<char_<Quote>, star<quoted_escape_char<Quote>>, char_<Quote>> {};

} // namespace yrp

#endif // YOCO_PARSER_TEXT_HH

