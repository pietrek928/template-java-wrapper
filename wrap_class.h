#ifndef __WRAP_CLASS_H_
#define __WRAP_CLASS_H_

#include "class_file.h"

template<class T, class align_t=jint>
class wrap_class : class_file {
    public:
    /*template<class ... Targs>
    inline static auto construct(T *p, args...) {
        new (p) tst(a);
        return java_types::special_object_func(args);
    }

    inline static auto destruct(T *p, class ... Targs) {
        p->~T();
        return java_types::special_object_func();
    }*/

    wrap_class(std::string path)
        : class_file(path) {
        // space allocation
        // TODO: check alignment
        constexpr int sz = sizeof(T)/sizeof(align_t)
            + ( sizeof(T)%sizeof(align_t) ? 1 : 0 );
        for (int i=0; i<sz; i++) class_file::var(java_types::v<align_t>());

        // constructor
        //native<>(name);

        // destructor
    }

    template<auto(T::*fp)(...)>
    auto &native(std::string name, u2 access = java_access_flags::PUBLIC) { // TODO: constructor
        class_file::native<fp>(name, access);
        return *this;
    }

    auto &var(std::string t, std::string n, u2 access = java_access_flags::PUBLIC) {
        class_file::var(t, n, access);
        return *this;
    }
};

#endif /* __WRAP_CLASS_H_ */

