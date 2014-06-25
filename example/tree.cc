#include <vector>
#include <iostream>
#include <yrp/star.hh>
#include <yrp/text.hh>
#include <yrp/opt.hh>
#include <yrp/parser.hh>
#include <yrp/act.hh>
#include <yrp/list.hh>
#include <yrp/tree_building_parser.hh>

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
        yrp::list<
            yrp::store<integer>,
            yrp::char_<L','>
        >,
        yrp::char_<L']'>
    >
{};

struct int_lists :
    yrp::plus<
        yrp::store<integer_list>
    >
{};

//////////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////////

int main() {
    std::wstring i = L"[1,3,4][23,45,2][383,34]";
    yrp::TreeBuildingParser<std::wstring::const_iterator> p(i.begin(), i.end());
    bool result = p.parse<int_lists>();
    std::cout << result << std::endl;
    if(result) {
        std::cout << "test pass" << std::endl;
        yrp::TreeBuildingParser<std::wstring::const_iterator>::NodeType* root = p.root;
        for(const auto l : root->children) {
            for(const auto i : l->children) {
                std::wstring s(i->begin, i->end);
                int v = std::stoi(s);
                std::cout << v << " ";
            }
            std::cout << std::endl;
        }
        root->dump_graphviz(std::wcout);
    }
}
