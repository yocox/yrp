#include <vector>
#include <iostream>
#include <tuple>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <yrp/star.hh>
#include <yrp/text.hh>
#include <yrp/opt.hh>
#include <yrp/parser.hh>
#include <yrp/act.hh>
#include <yrp/list.hh>

struct nil {} ;

template <typename IntType = int>
struct int_v {
    using ValueType = IntType;
    template <typename Parser>
    static bool match(Parser& p, ValueType& v) {
        if(p.at_end()) {
            return false;
        }

        // parse first digit, false if not digit
        v = ValueType(0);
        if(p.elem() < L'0' || p.elem() > L'9') {
            return false;
        } else {
            v = p.elem() - L'0';
        }
        p.next();

        // parse following digit
        while(!p.at_end() && p.elem() >= L'0' && p.elem() <= L'9') {
            v *= 10;
            v += p.elem() - L'0';
            p.next();
        }

        return true;
    }
} ;

template <wchar_t C>
struct char_v {
    using ValueType = wchar_t;
    template <typename Parser>
    static bool match(Parser& p, ValueType& v) {
        if(p.at_end()) {
            return false;
        }
        if(p.elem() == C) {
            v = p.elem();
            p.next();
            return true;
        } else {
            return false;
        }
    }
    template <typename Parser>
    static bool match(Parser& p) {
        if(p.at_end()) {
            return false;
        }
        if(p.elem() == C) {
            p.next();
            return true;
        } else {
            return false;
        }
    }
} ;

//////////////////////////////////////////////////////////////////////////////
// str_v
//////////////////////////////////////////////////////////////////////////////

template <typename Rule>
struct str_v {
    using ValueType = std::wstring;
    template <typename Parser>
    static bool match(Parser& p, ValueType& v) {
        typename Parser::iterator orig_pos = p.pos();
        if(!Rule::template match(p)) {
            return false;
        }
        v = std::wstring(orig_pos, p.pos());
        return true;
    }
} ;

//////////////////////////////////////////////////////////////////////////////
// list_v
//////////////////////////////////////////////////////////////////////////////

template <typename Elem, typename Deli>
struct list_v {
    using ValueType = std::vector<typename Elem::ValueType>;
    template <typename Parser>
    static bool match(Parser& p, ValueType& v) {
        typename Parser::iterator orig_pos = p.pos();
        typename Elem::ValueType elem_value;
        bool success = Elem::template match(p, elem_value);

        // parse first elem
        if(!success) {
            return false;
        }

        // parse following "delimeter, elem" pair
        v.clear();
        v.push_back(elem_value);
        while(true) {
            if (!Deli::template match(p)) {
                return true;
            }
            bool success = Elem::template match(p, elem_value);
            if(!success) {
                p.pos(orig_pos);
                return false;
            } else {
                v.push_back(elem_value);
            }
        }
    }
};

//////////////////////////////////////////////////////////////////////////////
// opt_v
//////////////////////////////////////////////////////////////////////////////

template <typename Rule>
struct opt_v {
    using ValueType = boost::optional<typename Rule::ValueType>;
    template <typename Parser>
    static bool match(Parser& p, ValueType& v) {
        v = typename Rule::ValueType();
        bool success = Rule::template match(p, v.get());
        if(!success) {
            v = boost::none;
        }
        return true;
    }
} ;

//////////////////////////////////////////////////////////////////////////////
// or_v
//////////////////////////////////////////////////////////////////////////////

template <typename ... Rule>
struct or_v_impl { } ;

template <typename Head, typename ... Tail>
struct or_v_impl<Head, Tail...> {
    template <typename Parser, typename VariantType>
    static bool match(Parser& p, VariantType& v) {
        typename Head::ValueType hv;
        if(Head::template match(p, hv)) {
            v = hv;
            return true;
        }
        return or_v_impl<Tail...>::template match(p, v);
    }
} ;

template <typename Last>
struct or_v_impl<Last> {
    template <typename Parser, typename VariantType>
    static bool match(Parser& p, VariantType& v) {
        typename Last::ValueType hv;
        if(Last::template match(p, hv)) {
            v = hv;
            return true;
        }
        return false;
    }
} ;

template <typename ... Rule>
struct or_v {
    using ValueType = boost::variant<typename Rule::ValueType...>;
    template <typename Parser>
    static bool match(Parser& p, ValueType& v) {
        return or_v_impl<Rule...>::template match(p, v);
    }
} ;

//////////////////////////////////////////////////////////////////////////////
// star_v, plus_v
//////////////////////////////////////////////////////////////////////////////

template <typename Rule>
struct star_v {
    using ValueType = std::vector<typename Rule::ValueType>;
    template <typename Parser>
    static bool match(Parser& p, ValueType& v) {
        typename Rule::ValueType e;
        while(Rule::template match(p, e)) {
            v.push_back(e);
        }
        return true;
    }
} ;

template <typename Rule>
struct plus_v {
    using ValueType = std::vector<typename Rule::ValueType>;
    template <typename Parser>
    static bool match(Parser& p, ValueType& v) {
        typename Rule::ValueType e;
        if(Rule::template match(p, e)) {
            v.push_back(e);
        } else {
            return false;
        }
        while(Rule::template match(p, e)) {
            v.push_back(e);
        }
        return true;
    }
} ;

//////////////////////////////////////////////////////////////////////////////
// seq_v
//////////////////////////////////////////////////////////////////////////////

template <typename T>
struct novalue {};

namespace internal {
template <typename ... Ts>
struct seq_p_impl;

template <typename Head, typename ... Tail, typename ... Filtered>
struct seq_p_impl<std::tuple<Filtered...>, Head, Tail...> {
    using ValueType = typename seq_p_impl<std::tuple<Filtered..., typename Head::ValueType>, Tail...>::ValueType;
    template <int Index, typename Parser, typename FinalResultType>
    static bool match(Parser& p, FinalResultType& result_value) {
        bool head_result = Head::template match(p, std::get<Index>(result_value));
        if(!head_result) {
            return false;
        }

        bool tail_result = seq_p_impl<std::tuple<Filtered..., typename Head::ValueType>, Tail...>::template match<Index + 1>(p, result_value);
        if(!tail_result) {
            return false;
        }
        return true;
    }
};

template <typename Head, typename ... Tail, typename ... Filtered>
struct seq_p_impl<std::tuple<Filtered...>, novalue<Head>, Tail...> {
    using ValueType = typename seq_p_impl<std::tuple<Filtered...>, Tail...>::ValueType;
    template <int Index, typename Parser, typename FinalResultType>
    static bool match(Parser& p, FinalResultType& result_value) {
        auto head_result = Head::template match(p);
        if(!head_result) {
            return false;
        }

        bool tail_result = seq_p_impl<std::tuple<Filtered...>, Tail...>::template match<Index>(p, result_value);
        if(!tail_result) {
            return false;
        }
        return true;
    }
};

template <typename Last, typename ... Filtered>
struct seq_p_impl<std::tuple<Filtered...>, Last> {
    using ValueType = std::tuple<Filtered..., typename Last::ValueType>;
    template <int Index, typename Parser, typename FinalResultType>
    static bool match(Parser& p, FinalResultType& result_value) {
        auto r = Last::template match(p, std::get<Index>(result_value));
        if(r) {
            return true;
        } else {
            return false;
        }
    }
};

template <typename Last, typename ... Filtered>
struct seq_p_impl<std::tuple<Filtered...>, novalue<Last>> {
    using ValueType = std::tuple<Filtered...>;
    template <int Index, typename Parser, typename FinalResultType>
    static bool match(Parser& p, FinalResultType&) {
        auto r = Last::template match(p);
        if(r) {
            return true;
        } else {
            return false;
        }
    }
};

} // namespace internal

template <typename ... Rules>
struct seq_v {
    using ValueType = typename internal::seq_p_impl<std::tuple<>, Rules...>::ValueType;
    template <typename Parser>
    static bool match(Parser& p, ValueType& v) {
        typename Parser::iterator orig_pos = p.pos();
        auto r = internal::seq_p_impl<std::tuple<>, Rules...>::template match<0>(p, v);
        if(r) {
            return true;
        } else {
            p.pos(orig_pos);
            return false;
        }
        return true;
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
        typename Rule::ValueType result_value;
        bool success = Rule::match(*this, result_value);
        return {success, result_value};
    }
};

//////////////////////////////////////////////////////////////////////////////
// grammar
//////////////////////////////////////////////////////////////////////////////

struct Point {
    int x;
    int y;
} ;

struct coor_v {
    using ValueType = Point;
    template <typename Parser>
    static bool match(Parser& p, Point& v) {
        typename Parser::iterator orig_pos = p.pos();
        int x, y;
        if(yrp::char_<L'('>::template match(p) &&
           int_v<>::template match(p, x) &&
           yrp::char_<L','>::template match(p) &&
           int_v<>::template match(p, y) &&
           yrp::char_<L')'>::template match(p))
        {
            v.x = x;
            v.y = y;
            return true;
        } else {
            p.pos(orig_pos);
            return false;
        }
    }
} ;

struct Points :
    plus_v<
        coor_v
    >
{};

struct int_list :
    seq_v<
        novalue<yrp::char_<L'['>>,
        list_v<
            int_v<>,
            yrp::char_<L','>
        >,
        novalue<yrp::char_<L']'>>
    >
{};

struct int_lists :
    seq_v<
        opt_v<or_v<
            char_v<L'!'>,
            int_v<int>,
            str_v<yrp::id_word>,
            plus_v<char_v<L'#'>>
        >>,
        list_v<
            int_list,
            yrp::char_<L' '>
        >
    >
{};

//////////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////////

int main() {
    std::wstring i = L"(3,5)(4,7)(5,9)";
    ValueBuildingParser<std::wstring::const_iterator> p(i.begin(), i.end());
    auto result = p.parse<Points>();
    for(const auto& p : result.second) {
        std::cout << p.x << ", " << p.y << std::endl;
    }

    //std::wstring i = L"yoco[124,33334,993] [123,45634,33324,11234,3]";
    //ValueBuildingParser<std::wstring::const_iterator> p(i.begin(), i.end());
    //auto result = p.parse<int_lists>();
    //if(result.first) {
    //    std::cout << "parse success" << std::endl;
    //    auto& init_token = std::get<0>(result.second);
    //    if(init_token) {
    //        auto& vari = init_token.get();
    //        std::cout << vari.which() << std::endl;
    //        if(vari.type() == typeid(wchar_t)) {
    //            std::wcout << L"wchar_t " << boost::get<wchar_t>(init_token.get()) << std::endl;
    //        } else if(vari.type() == typeid(int)) {
    //            std::wcout << L"int " << boost::get<int>(init_token.get()) << std::endl;
    //        } else if(vari.type() == typeid(std::wstring)) {
    //            std::wcout << L"str " << boost::get<std::wstring>(init_token.get()) << std::endl;
    //        } else {
    //            std::wcout << boost::get<std::vector<wchar_t>>(init_token.get()).size() << std::endl;
    //            for(auto c : boost::get<std::vector<wchar_t>>(init_token.get())) {
    //                std::wcout << L"char vector " << c << std::endl;
    //            }
    //        }
    //    }
    //    for(const auto& il : std::get<1>(result.second)) {
    //        for(const auto& i : std::get<0>(il)) {
    //            std::cout << i << " ";
    //        }
    //        std::cout << std::endl;
    //    }
    //} else {
    //    std::cout << "parse fail" << std::endl;
    //}
}
