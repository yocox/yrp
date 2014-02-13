#ifndef YOCO_PARSER_DYNAMIC_HH
#define YOCO_PARSER_DYNAMIC_HH

#include <string>

namespace yrp {

template <typename Rule, int N>
struct back_ref
{
    template <typename Parser>
    static bool match(Parser& p) {
        typename Parser::iterator begin = p.pos();
        if(!Rule::template match(p))
            return false;
        typename Parser::iterator end = p.pos();
        std::wstring w(begin, end);
        p.template set_back_reference<N>(begin, end);
        return true;
    }
} ;

template <int N>
struct ref_to
{
    template <typename Parser>
    static bool match(Parser& p) {
        typedef typename Parser::iterator iter;
        const std::pair<iter, iter> range = p.template get_back_reference<N>();
        int size = range.second - range.first;
        if(p.end() - p.pos() < size) {
            return false;
        }
        if(std::equal(range.first, range.second, p.pos())) {
            p.pos(p.pos() + size);
            return true;
        } else {
            return false;
        }
    }
} ;

} // namespace yrp

#endif // YOCO_PARSER_DYNAMIC_HH

