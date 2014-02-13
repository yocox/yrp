#ifndef YOCO_PARSER_PARSER_HH
#define YOCO_PARSER_PARSER_HH

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>
#include <type_traits>

namespace yrp {

template <typename IterType>
class parser {

  public:

    // ctor
    parser(IterType begin, IterType end)
        : begin_(begin)
        , end_(end)
        , pos_(begin)
    {}

    // type
    using iterator = IterType;
    using reference = typename iterator::reference;
    using value_type = typename iterator::value_type;

    // query
    reference  elem    () const { return *pos_  ; }
    iterator   pos     () const { return pos_   ; }
    iterator   begin   () const { return begin_ ; }
    iterator   end     () const { return end_   ; }
    bool       at_begin() const { return pos() == begin() ; }
    bool       at_end  () const { return pos() == end() ; }

    // action
    void next() { assert(pos() != end()); ++pos_; }
    void pos(const iterator p) { pos_ = p; }

    template <typename Rule>
    bool parse() {
        return Rule::match(*this);
    }

    // back reference
    template <int N>
    void set_back_reference(iterator begin, iterator end) {
        back_reference_[N] = {begin, end};
    }

    template <int N>
    std::pair<iterator, iterator> get_back_reference() {
        return back_reference_[N];
    }

    void inc_trace_level() { ++trace_level_; }
    void dec_trace_level() { --trace_level_; }
    int trace_level() { return trace_level_; }

  protected:
    iterator begin_;
    iterator end_;
    iterator pos_;

  private:
    std::pair<iterator, iterator> back_reference_[10];
    int trace_level_ = 0;
    std::deque<std::string> trace_info_;
};

} // namespace yrp

#endif // YOCO_PARSER_PARSER_HH

