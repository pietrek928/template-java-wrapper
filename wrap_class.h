#ifndef __WRAP_CLASS_H_
#define __WRAP_CLASS_H_

#include "class_file.h"

template<class T, class align_t=jint>
class wrap_class : class_file {
    public: 

    wrap_class(std::string path)
        : class_file(path) {
        // space allocation
        // TODO: check alignment
        constexpr int sz = sizeof(T)/sizeof(align_t)
            + ( sizeof(T)%sizeof(align_t) ? 1 : 0 );
        for (int i=0; i<sz; i++) class_file::var(java_types::v<align_t>());

        // constructor
        // TODO: more constructors
        //class_file::native<java_types::construct<T>>(name, java_access_flags::PROTECTED);

        // destructor
        class_file::native<java_types::destruct<T>>("finalize", java_access_flags::PROTECTED);
    }

    template<class ... Targs_c>
    auto &constructor() {
        class_file::native<java_types::construct<T, Targs_c...>> (
                name, java_access_flags::PROTECTED);
        return *this;
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

