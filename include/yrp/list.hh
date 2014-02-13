#ifndef YOCO_PARSER_LIST_HH
#define YOCO_PARSER_LIST_HH

#include "seq.hh"
#include "star.hh"

namespace yrp {

template <typename Elem, typename Deli>
struct list :
    seq<
        Elem,
        star<seq<Deli, Elem>>
    >
{};

} // namespace yrp

#endif // YOCO_PARSER_LIST_HH



