#ifndef YOCO_PARSER_OR_HH
#define YOCO_PARSER_OR_HH

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>
#include <typeinfo>
#include <cxxabi.h>

namespace yrp {

template <typename Head, typename ... Tail>
struct or_ {
    template <typename Parser>
    static bool match(Parser& p) {
        if(Head::template match(p)) {
            return true;
        } else {
            return or_<Tail...>::template match(p);
        }
    }
};

template <typename Last>
struct or_<Last> {
    template <typename Parser>
    static bool match(Parser& p) {
        if(Last::template match(p)) {
            return true;
        } else {
            return false;
        }
    }
};

namespace internal {

template <typename T>
std::string demangle_name() {
    int status;
    return abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
}

template <typename Head, typename ... Tail>
struct trace_or_impl {
    template <typename Parser>
    static bool match(Parser& p) {
        std::cout << std::endl;
        for(int i = 0; i < p.trace_level(); ++i) { std::cout << " "; }
        std::cout << "try " << demangle_name<Head>();
        if(Head::template match(p)) {
            std::cout << std::endl;
            for(int i = 0; i < p.trace_level(); ++i) { std::cout << " "; }
            std::cout << demangle_name<Head>() << " match";
            return true;
        } else {
            std::cout << std::endl;
            for(int i = 0; i < p.trace_level(); ++i) { std::cout << " "; }
            std::cout << demangle_name<Head>() << " miss";
            return trace_or_impl<Tail...>::template match(p);
        }
    }
};

template <typename Last>
struct trace_or_impl<Last> {
    template <typename Parser>
    static bool match(Parser& p) {
        std::cout << std::endl;
        for(int i = 0; i < p.trace_level(); ++i) { std::cout << " "; }
        std::cout << "try " << demangle_name<Last>();
        if(Last::template match(p)) {
            std::cout << std::endl;
            for(int i = 0; i < p.trace_level(); ++i) { std::cout << " "; }
            std::cout << demangle_name<Last>() << " match";
            return true;
        } else {
            std::cout << std::endl;
            for(int i = 0; i < p.trace_level(); ++i) { std::cout << " "; }
            std::cout << demangle_name<Last>() << " miss";
            return false;
        }
    }
};

} // namespace internal

//template <typename ... Rules>
//struct trace_or {
//    template <typename Parser>
//    static bool match(Parser& p) {
//        p.inc_trace_level();
//        bool result = internal::trace_or_impl<Rules...>::match(p);
//        p.dec_trace_level();
//        return result;
//    }
//};

template <typename ... Rules>
using trace_or = or_<Rules...>;

} // namespace yrp

#endif // YOCO_PARSER_OR_HH

