#include <vector>
#include <iostream>
#include <tuple>
#include <yrp/star.hh>
#include <yrp/text.hh>
#include <yrp/opt.hh>
#include <yrp/parser.hh>
#include <yrp/act.hh>
#include <yrp/list.hh>

struct nil {} ;

template <typename IntType>
struct int_p {
    using ResultType = IntType;
    template <typename Parser>
    static std::pair<bool, ResultType> match(Parser& p) {
        if(p.at_end()) {
            return {false, 0};
        }

        // parse first digit, false if not digit
        IntType result = IntType(0);
        if(p.elem() < L'0' || p.elem() > L'9') {
            return {false, 0};
        } else {
            result = p.elem() - L'0';
        }
        p.next();

        // parse following digit
        while(!p.at_end() && p.elem() >= L'0' && p.elem() <= L'9') {
            result *= 10;
            result += p.elem() - L'0';
            p.next();
        }

        return {true, result};
    }
} ;

template <wchar_t C>
struct char_p {
    using ResultType = nil;
    template <typename Parser>
    static std::pair<bool, ResultType> match(Parser& p) {
        if(p.at_end()) {
            return {false, nil()};
        }
        if(p.elem() == C) {
            p.next();
            return {true, nil()};
        } else {
            return {false, nil()};
        }
    }
} ;

template <typename Elem, typename Deli>
struct list_p {
    using ResultType = std::vector<typename Elem::ResultType>;
    template <typename Parser>
    static std::pair<bool, ResultType> match(Parser& p) {
        typename Parser::iterator orig_pos = p.pos();
        ResultType result_value;
        auto e = Elem::template match(p);

        // parse first elem
        if(!e.first) {
            return {false, ResultType()};
        }

        // parse following "delimeter, elem" pair
        result_value.push_back(e.second);
        while(true) {
            if (!Deli::template match(p).first) {
                return {true, std::move(result_value)};
            }
            auto e2 = Elem::template match(p);
            if(!e2.first) {
                p.pos(orig_pos);
                return {false, ResultType()};
            } else {
                result_value.push_back(e2.second);
            }
        }
    }
};

namespace internal {
template <typename Head, typename ... Tail>
struct seq_p_impl {
    using ResultType = std::tuple<typename Head::ResultType, typename Tail::ResultType...>;
    template <int Index, typename Parser, typename FinalResultType>
    static bool match(Parser& p, FinalResultType& result_value) {
        auto head_result = Head::template match(p);
        if(!head_result.first) {
            return false;
        }
        std::get<Index>(result_value) = head_result.second;

        bool tail_result = seq_p_impl<Tail...>::template match<Index + 1>(p, result_value);
        if(!tail_result) {
            return false;
        }
        return true;
    }
};

template <typename Last>
struct seq_p_impl<Last> {
    using ResultType = std::tuple<typename Last::ResultType>;
    template <int Index, typename Parser, typename FinalResultType>
    static bool match(Parser& p, FinalResultType& result_value) {
        auto r = Last::template match(p);
        if(r.first) {
            std::get<Index>(result_value) = r.second;
            return true;
        } else {
            return false;
        }
    }
};
}

template <typename ... Rules>
struct seq_p {
    using ResultType = std::tuple<typename Rules::ResultType...>;
    template <typename Parser>
    static std::pair<bool, ResultType> match(Parser& p) {
        typename Parser::iterator orig_pos = p.pos();
        ResultType result_value;
        auto r = internal::seq_p_impl<Rules...>::template match<0>(p, result_value);
        if(r) {
            return {true, std::move(result_value)};
        } else {
            p.pos(orig_pos);
            return {false, std::move(result_value)};
        }
    }
};

//////////////////////////////////////////////////////////////////////////////
// Parser
//////////////////////////////////////////////////////////////////////////////

template <typename IterType>
struct ValueBuildingParser : public yrp::parser<IterType>
{
  public:

    // constructor
    ValueBuildingParser(IterType b, IterType e)
        : yrp::parser<IterType>(b, e)
    {
    }

    // using super's member，該死的 C++，不 using 看不見
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
    std::pair<bool, typename Rule::ResultType> parse() {
        return Rule::match(*this);
    }

    // data member
};

//////////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////////

struct ints :
    list_p<
        int_p<int>,
        char_p<L' '>
    >
{} ;

struct int_pair :
    seq_p<
        int_p<int>,
        char_p<L','>,
        int_p<int>
    >
{} ;

int main() {
    std::wstring i = L"123,456";
    ValueBuildingParser<std::wstring::const_iterator> p(i.begin(), i.end());
    auto result = p.parse<int_pair>();
    if(result.first) {
        std::cout << "parse success" << std::endl;
        std::cout << "result value is " << std::endl;
        std::cout << "    " << std::get<0>(result.second) << std::endl;
        std::cout << "    " << std::get<2>(result.second) << std::endl;
    } else {
        std::cout << "parse fail" << std::endl;
    }
}
