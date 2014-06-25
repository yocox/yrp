#ifndef YOCO_PARSER_TREE_BUILDING_PARSER_HH
#define YOCO_PARSER_TREE_BUILDING_PARSER_HH

#include <vector>
#include <iostream>
#include <typeinfo>
#include "parser.hh"

namespace yrp {

template <typename IterType>
struct Node {
    Node(IterType b, IterType e) : begin(b), end(e) {
    } ;

    Node<IterType>* parent;
    std::vector<Node<IterType>*> children;

    ~Node() {
        for(auto c : children) {
            delete c;
        }
    } ;

    IterType begin;
    IterType end;

    void add_child(Node* n) {
        children.push_back(n);
    }

    template <typename FilterType, typename Func>
    void foreach(Func& func) {
        for(auto& c : children) {
            if(typeid(c) == typeid(FilterType))
                func(c);
        }
    }

    virtual std::wostream& dot_node(std::wostream& os) = 0;

    std::wostream& dump_graphviz(std::wostream& os) {
        os << L"digraph g {\n    rankdir=LR;\n";
        this->dot_node(os);
        os << L"}";
        return os;
    }
} ;

template <typename IterType, typename Rule>
struct TypedNode : public Node<IterType> {
    using super = Node<IterType>;
    TypedNode(IterType b, IterType e) : Node<IterType>(b, e)
    { } ;
    virtual std::wostream& dot_node(std::wostream& os) {
        os << L"    \"" << this << L"\" [label=\"" << typeid(Rule).name();
        if(super::children.empty()) {
            os << L" : ";
            for(auto i = super::begin; i < super::end; ++i) {
                os << *i;
            }
        }
        os << L"\"];" << std::endl;
        for(auto child : super::children) {
            os << L"    \"" << this << L"\" -> \"" << child << L"\";" << std::endl;
            child->dot_node(os);
        }
        return os;
    }
} ;

template <typename Rule>
struct store {
    template <typename Parser>
    static bool match(Parser &p) {
        typename Parser::iterator orig_pos = p.pos();
        Node<typename Parser::iterator>* c = p.current;
        Node<typename Parser::iterator>* n = new TypedNode<typename Parser::iterator, Rule>(orig_pos, orig_pos);
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
struct TreeBuildingParser : public yrp::parser<IterType>
{
  public:

    // constructor
    TreeBuildingParser(IterType b, IterType e)
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
    using NodeType = Node<IterType>;

    // parse
    template <typename Rule>
    bool parse() {
        root = new TypedNode<IterType, Rule>(begin(), end());
        current = root;
        return Rule::match(*this);
    }

    // data member
    Node<IterType>* root;
    Node<IterType>* current;
};

} // namespace yrp

#endif // YOCO_PARSER_TREE_BUILDING_PARSER_HH
