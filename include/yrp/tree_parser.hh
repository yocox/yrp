#ifndef YOCO_PARSER_TREE_PARSER_HH
#define YOCO_PARSER_TREE_PARSER_HH

#include <vector>
#include <typeinfo>
#include "parser.hh"

namespace yrp {

template <typename IterType>
struct node_base {
    node_base(IterType b, IterType e) : begin(b), end(e) {
    } ;

    node_base<IterType>* parent;
    std::vector<node_base<IterType>*> children;

    ~node_base() {
        for(auto c : children) {
            delete c;
        }
    } ;

    IterType begin;
    IterType end;

    void add_child(node_base* n) {
        children.push_back(n);
    }

    template <typename FilterType, typename Func>
    void foreach(Func& func) {
        for(auto& c : children) {
            if(typeid(c) == typeid(FilterType))
                func(c);
        }
    }
} ;

template <typename IterType, typename Rule>
struct node : public node_base<IterType> {
    node(IterType b, IterType e) : node_base<IterType>(b, e)
    { } ;
} ;

template <typename Rule>
struct store {
    template <typename Parser>
    static bool match(Parser &p) {
        typename Parser::iterator orig_pos = p.pos();
        node_base<typename Parser::iterator>* c = p.current;
        node_base<typename Parser::iterator>* n = new node<typename Parser::iterator, Rule>(orig_pos, orig_pos);
        n->parent = p.current;
        p.current = n;
        if(Rule::template match(p)) {
            n->end = p.pos();
            c->children.push_back(n);
            p.current = c;
            return true;
        } else {
            p.pos(orig_pos);
            p.current = c;
            delete n;
            return false;
        }
    }
} ;

//////////////////////////////////////////////////////////////////////////////
// Parser
//////////////////////////////////////////////////////////////////////////////

//using WsIterType = std::wstring::const_iterator;
template <typename IterType>
struct TreeParser : public yrp::parser<IterType>
{
  public:

    // constructor
    TreeParser(IterType b, IterType e)
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
    using NodeType = node_base<IterType>;

    // parse
    template <typename Rule>
    bool parse() {
        root = new node<IterType, Rule>(begin(), end());
        current = root;
        return Rule::match(*this);
    }

    // data member
    node_base<IterType>* root;
    node_base<IterType>* current;
};

} // namespace yrp

#endif // YOCO_PARSER_TREE_PARSER_HH
