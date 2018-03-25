#ifndef __WRAP_CLASS_H_
#define __WRAP_CLASS_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "class_file.h"

template<class T, class align_t=jint>
class wrap_class : class_file {
    public: 

    wrap_class(std::string path)
        : class_file(path) {

        // space allocation
        constexpr int sz = sizeof(T)/sizeof(align_t)
            + ( sizeof(T)%sizeof(align_t) ? 1 : 0 );
        for (int i=0; i<sz; i++)
            class_file::var(
                    java_types::v<align_t>(), 
                    "",
                    java_access_flags::PRIVATE | java_access_flags::FINAL
                );

        // constructor
        // TODO: add constructors by default ?
        //class_file::native<java_types::construct<T>>(name, java_access_flags::PROTECTED);

        // destructor
        class_file::native<java_types::destruct<T>>("finalize", java_access_flags::PROTECTED);
    }

    /* class method */
    template<auto fp, class Tr, class ... Targs>
    inline void __handle_native(Tr(T::*_fp)(Targs...), std::string name, u2 access) {
        class_file::native<fp>(name, access);
    }

    /* static method */
    template<auto fp, class Tr, class ... Targs>
    inline void __handle_native(Tr(*_fp)(Targs...), std::string name, u2 access) {
        access |= java_access_flags::STATIC;
        class_file::native<fp>(name, access);
    }

    template<auto fp>
    auto &native(std::string name, u2 access = java_access_flags::PUBLIC) {
        __handle_native<fp>(fp, name, access);
        return *this;
    }

    template<class ... Targs_c>
    auto &constructor(u2 access = java_access_flags::PUBLIC, std::string n = "getInstance") {
        native<java_types::get_instance<T, Targs_c...>> (n, access);
        return *this;
    }

    auto &var(std::string t, std::string n, u2 access = java_access_flags::PUBLIC) {
        class_file::var(t, n, access);
        return *this;
    }

    auto &reg_methods(JNIEnv* e) {
        jclass clazz = e->FindClass(path.c_str());
        if ( !clazz ) {
            if (e->ExceptionOccurred()) {
                e->ExceptionDescribe();
            } // TODO: unify error handling
            ERR("Class by path '%s' not found", path.c_str());
            throw std::runtime_error("Class not found");
        }
        java_types::class_factory<T>::ref_class(e);
        class_file::reg_methods(e, clazz);
        return *this;
    }

    int create_dir(std::string &d) {
        if (d.size() == 0) return -1;
__recreate:
        if (!mkdir(d.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) return 0;
        switch(errno) {
            case EEXIST: return 0;
            case ENOENT: {
                auto pos = d.find_last_of('/');
                if (pos == d.npos) return -1;
                auto parent = d.substr(0, pos);
                if (!create_dir(parent))
                    goto __recreate;
            }
            default: return -1;
        }
        return 0;
    }

    auto &store(std::string base_dir) {
        auto full_path = base_dir+"/"+path_dir();
        if (create_dir(full_path)) {
            ERR("Could not create directory '%s', error: %s", full_path.c_str(), strerror(errno));
            throw std::runtime_error("Could not create directory");
        }
        class_file::build().to_file(base_dir+"/"+path+".class");
        return *this;
    }
};

#endif /* __WRAP_CLASS_H_ */

