#ifndef YOCO_PARSER_ACT_HH
#define YOCO_PARSER_ACT_HH

#include <stdexcept>

namespace yrp {

template <typename Act>
struct just_act
{
    template <typename Parser>
    static bool match(Parser& p) {
        Act actor;
        return actor(p, p.pos(), p.pos());
    }
};

template <typename Rule, typename Act>
struct post_act
{
    template <typename Parser>
    static bool match(Parser& p) {
        typename Parser::iterator orig_pos = p.pos();
        if(Rule::template match(p)) {
            Act actor;
            bool result = actor(p, orig_pos, p.pos());
            if(!result)
                throw std::logic_error("FAIL: post_act fail");
            else
                return true;
        } else {
            p.pos(orig_pos);
            return false;
        }
    }
};

} // namespace yrp

#endif // YOCO_PARSER_ACT_HH

