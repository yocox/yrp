#include <iostream>
#include <yrp/star.hh>
#include <yrp/text.hh>
#include <yrp/opt.hh>
#include <yrp/parser.hh>
#include <yrp/act.hh>
#include <yrp/list.hh>

//////////////////////////////////////////////////////////////////////////////
// Parser
//////////////////////////////////////////////////////////////////////////////

using WsIterType = std::wstring::const_iterator;
struct IntListsParser : public yrp::parser<std::wstring::const_iterator>
{
  public:

    // constructor
    IntListsParser(WsIterType b, WsIterType e)
        : yrp::parser<WsIterType>(b, e)
    {
    }

    // using super's member，該死的 C++，不 using 看不見
    using IterType = WsIterType;
    using RangeType = std::pair<IterType, IterType>;
    using typename yrp::parser<IterType>::iterator;
    using typename yrp::parser<IterType>::reference;
    using typename yrp::parser<IterType>::value_type;
    using yrp::parser<IterType>::pos_;
    using yrp::parser<IterType>::elem;
    using yrp::parser<IterType>::pos;
    using yrp::parser<IterType>::begin;
    using yrp::parser<IterType>::end;
    using yrp::parser<IterType>::at_begin;
    using yrp::parser<IterType>::at_end;
    using yrp::parser<IterType>::next;

    // parse
    template <typename Rule>
    bool parse() {
        return Rule::match(*this);
    }

    // semantic action 函數們!!!!
    // 一概都拿一對 iterator 當作輸入，不管用的到用不到
    // return bool
    bool my_push_back_integer(RangeType r) {
        std::wstring s(r.first, r.second);
        int i = std::stoi(s);
        _v.push_back(i);
        return true;
    }
    bool my_clear_vec(RangeType) {
        _v.clear();
        return true;
    }
    bool my_print_vec(RangeType) {
        for(int i : _v) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        return true;
    }

    // data member
    std::vector<int> _v;
};

//////////////////////////////////////////////////////////////////////////////
// Semantic Action
//////////////////////////////////////////////////////////////////////////////

namespace act {

#define RANGE_ACT(action_name) \
struct action_name \
{ \
    template <typename Parser> \
    bool operator()(Parser& p, typename Parser::iterator begin, typename Parser::iterator end) { \
        return p.action_name({begin, end}); \
    } \
} ;

RANGE_ACT(my_push_back_integer)
RANGE_ACT(my_clear_vec)
RANGE_ACT(my_print_vec)

#undef RANGE_ACT

}

//////////////////////////////////////////////////////////////////////////////
// Parsing Rules
//////////////////////////////////////////////////////////////////////////////

struct integer :
    yrp::plus<
        yrp::digit_char
    >
{};

struct integer_list :
    yrp::seq<
        yrp::char_<L'['>,
        yrp::just_act<act::my_clear_vec>, // 遇到 '[' 表示新的 list，你可能會想要清掉本來的
        yrp::list<
            yrp::post_act<integer, act::my_push_back_integer>,
            yrp::char_<L','>
        >,
        yrp::char_<L']'>,
        yrp::just_act<act::my_print_vec> // 遇到 ']'，list 完成了，印出來
    >
{};

struct int_lists :
    yrp::plus<
        integer_list
    >
{};

//////////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////////

int main() {
    std::wstring i = L"[1,3,4][23,45,2][383,34]";
    IntListsParser p(i.begin(), i.end());
    bool result = p.parse<int_lists>();
    std::cout << result << std::endl;
    if(result) {
        std::cout << "test pass" << std::endl;
    }
}
