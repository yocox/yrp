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
struct int_v {
    using ValueType = IntType;
    template <typename Parser>
    static std::pair<bool, ValueType> match(Parser& p) {
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
struct char_v {
    using ValueType = nil;
    template <typename Parser>
    static std::pair<bool, ValueType> match(Parser& p) {
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
struct list_v {
    using ValueType = std::vector<typename Elem::ValueType>;
    template <typename Parser>
    static std::pair<bool, ValueType> match(Parser& p) {
        typename Parser::iterator orig_pos = p.pos();
        ValueType result_value;
        auto e = Elem::template match(p);

        // parse first elem
        if(!e.first) {
            return {false, ValueType()};
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
                return {false, ValueType()};
            } else {
                result_value.push_back(e2.second);
            }
        }
    }
};

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////

template <typename T>
struct value {};

template <typename ... Ts>
struct ValueTypeFilter { } ;

template <typename Head, typename ... Tail, typename ... Filtered>
struct ValueTypeFilter<std::tuple<Filtered...>, Head, Tail...> {
    using ValueType = typename ValueTypeFilter<std::tuple<Filtered...>, Tail...>::ValueType;
} ;

template <typename Head, typename ... Tail, typename ... Filtered>
struct ValueTypeFilter<std::tuple<Filtered...>, value<Head>, Tail...> {
    using ValueType = typename ValueTypeFilter<std::tuple<Filtered..., typename Head::ValueType>, Tail...>::ValueType;
} ;

template <typename Tail, typename ... Filtered>
struct ValueTypeFilter<std::tuple<Filtered...>, Tail> {
    using ValueType = std::tuple<Filtered...>;
} ;

template <typename Tail, typename ... Filtered>
struct ValueTypeFilter<std::tuple<Filtered...>, value<Tail>> {
    using ValueType = std::tuple<Filtered..., typename Tail::ValueType>;
} ;

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////

namespace internal {
template <typename ... Ts>
struct seq_p_impl;

template <typename Head, typename ... Tail, typename ... Filtered>
struct seq_p_impl<std::tuple<Filtered...>, Head, Tail...> {
    using ValueType = typename ValueTypeFilter<std::tuple<Filtered...>, Tail...>::ValueType;
    template <int Index, typename Parser, typename FinalResultType>
    static bool match(Parser& p, FinalResultType& result_value) {
        auto head_result = Head::template match(p);
        if(!head_result.first) {
            return false;
        }
        //std::get<Index>(result_value) = head_result.second;

        bool tail_result = seq_p_impl<std::tuple<Filtered...>, Tail...>::template match<Index>(p, result_value);
        if(!tail_result) {
            return false;
        }
        return true;
    }
};

template <typename Head, typename ... Tail, typename ... Filtered>
struct seq_p_impl<std::tuple<Filtered...>, value<Head>, Tail...> {
    using ValueType = typename ValueTypeFilter<std::tuple<Filtered..., typename Head::ValueType>, Tail...>::ValueType;
    template <int Index, typename Parser, typename FinalResultType>
    static bool match(Parser& p, FinalResultType& result_value) {
        auto head_result = Head::template match(p);
        if(!head_result.first) {
            return false;
        }
        //static_assert(std::is_same<decltype(std::get<Index>(result_value)), decltype(head_result.second)>::value, "rdenwdor");
        std::get<Index>(result_value) = head_result.second;
        //std::get<Index>(result_value) = 1;

        bool tail_result = seq_p_impl<std::tuple<Filtered..., typename Head::ValueType>, Tail...>::template match<Index + 1>(p, result_value);
        if(!tail_result) {
            return false;
        }
        return true;
    }
};

template <typename Last, typename ... Filtered>
struct seq_p_impl<std::tuple<Filtered...>, Last> {
    using ValueType = std::tuple<Filtered...>;
    template <int Index, typename Parser, typename FinalResultType>
    static bool match(Parser& p, FinalResultType&) {
        auto r = Last::template match(p);
        if(r.first) {
            return true;
        } else {
            return false;
        }
    }
};

template <typename Last, typename ... Filtered>
struct seq_p_impl<std::tuple<Filtered...>, value<Last>> {
    using ValueType = std::tuple<Filtered..., typename Last::ValueType>;
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

} // namespace internal

template <typename ... Rules>
struct seq_v {
    using ValueType = typename ValueTypeFilter<std::tuple<>, Rules...>::ValueType;
    template <typename Parser>
    static std::pair<bool, ValueType> match(Parser& p) {
        typename Parser::iterator orig_pos = p.pos();
        ValueType result_value;
        auto r = internal::seq_p_impl<std::tuple<>, Rules...>::template match<0>(p, result_value);
        if(r) {
            return {true, std::move(result_value)};
        } else {
            p.pos(orig_pos);
            return {false, std::move(result_value)};
        }
        return {true, ValueType()};
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
    std::pair<bool, typename Rule::ValueType> parse() {
        return Rule::match(*this);
    }

    // data member
};

//////////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////////

struct int_pair :
    seq_v<
        value<int_v<int>>,
        char_v<L','>,
        value<int_v<int>>
    >
{} ;

struct int_pairs :
    list_v<
        int_pair,
        char_v<L' '>
    >
{} ;

int main() {
    std::wstring i = L"123,456 34,33 324,11 234,3";
    ValueBuildingParser<std::wstring::const_iterator> p(i.begin(), i.end());
    auto result = p.parse<int_pairs>();
    if(result.first) {
        std::cout << "parse success" << std::endl;
        std::cout << "result value is " << std::endl;
        for(const auto& ip : result.second) {
            std::cout << "    " << std::get<0>(ip) << ", " << std::get<1>(ip) << std::endl;
        }
    } else {
        std::cout << "parse fail" << std::endl;
    }
}
