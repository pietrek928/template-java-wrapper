#ifndef __JAVA_TYPES_H_
#define __JAVA_TYPES_H_

#include "define.h"
#include "exceptions.h"

namespace java_access_flags {
    const static int
        PUBLIC      = 0x0001,
        PRIVATE     = 0x0002,
        PROTECTED   = 0x0004,
        STATIC      = 0x0008,
        FINAL       = 0x0010,
        SUPER       = 0x0020,
        VOLATILE    = 0x0040,
        TRANSIENT   = 0x0080,
        NATIVE      = 0x0100,
        INTERFACE   = 0x0200,
        ABSTRACT    = 0x0400,
        STRICT      = 0x0800,
        SYNTHETIC   = 0x1000,
        ANNOTATION  = 0x2000,
        ENUM        = 0x4000;
};

namespace java_types {

    /*
     * Type conversions
     * */

#define VAR_DESCR(t, d)            \
    inline std::string _v(t *tv) {  \
        return STR(d);               \
    }
    VAR_DESCR(void, V)
    VAR_DESCR(jboolean, Z)
    VAR_DESCR(jbyte, B)
    VAR_DESCR(jshort, S)
    VAR_DESCR(jint, I)
    VAR_DESCR(jlong, L)
    VAR_DESCR(jfloat, F)
    VAR_DESCR(jdouble, D)
    VAR_DESCR(jchar, C)
#undef VAR_DESCR

    template<class Tr, class ... Ta>
    constexpr bool returns_void(Tr(*f)(Ta...)) {
        return std::is_same<Tr, void>::value;
    }

    template<class Tr, class To, class ... Ta>
    constexpr bool returns_void(Tr(To::*f)(Ta...)) {
        return std::is_same<Tr, void>::value;
    }

    template<class T>
    class tclass;

    template<> /* handles void returning functions */
    class tclass<void> {
        public:
            inline void r(JNIEnv *e) {}
    };
 
#define MAP_TYPE(tc, tj)               \
    template<>                          \
    class tclass<tc> {                   \
        public:                           \
            tc v;                          \
            inline tclass(tc &v)            \
                : v(v) {}                    \
            inline tclass(JNIEnv *e, tj v)    \
                : v(v) {}                      \
            operator tc() {return v;}           \
            inline tj r(JNIEnv *e) {return v;}   \
    };
    MAP_TYPE(int, jint)
    MAP_TYPE(bool, jboolean)
    MAP_TYPE(uint8_t, jbyte)
    MAP_TYPE(int8_t, jbyte)
    MAP_TYPE(uint16_t, jchar)
    MAP_TYPE(int16_t, jshort)
#undef MAP_TYPE

    template<>
    class tclass<JNIEnv*> {
        public:
            JNIEnv *ee;
            tclass(JNIEnv *e, jobject o) : ee(e) {}
            inline operator JNIEnv*() {return ee;}
            inline void r(JNIEnv *e) {}
    };

    /* special object function */
    class special_object_func {};
    template<>
    class tclass<special_object_func> {
        public:
            inline void r(JNIEnv *e) {}
    };
    template<class Tr, class ... Ta>
    constexpr bool returns_special(Tr(*f)(Ta...)) {
        return std::is_same<Tr, special_object_func>::value;
    }

    /*
     * Stores class pointer and creates classes
     * */
    typedef struct {
        jclass clazz;
        jmethodID constr_id; // TODO: optional field
        std::string path;

        void clear() {
            clazz = NULL;
            constr_id = NULL;
            path = "";
        }
    } class_ref_info;
    std::vector<class_ref_info*> class_holder;
    template<class Tc, bool full_object=true>
    class class_factory {
        static class_ref_info descr;

        public:

        static jclass get_class() {
            if (!descr.clazz) {
                ERR("Tried to retrieve unreferenced C++ class %s\n", typeid(Tc).name());
                throw std::runtime_error("Cannot retrieve unreferenced class");
            }
            return descr.clazz;
        }
        static jobject alloc(JNIEnv *e) {
            if constexpr(full_object) {
                // java object can use only empty constructor
                // C++ constructors are mapped to static initializers
                return e->NewObject(descr.clazz, descr.constr_id);
            } else {
                // CAUTION: for such object there's no finalize !!!!!!!
                // be aware of memory leak
                return e->AllocObject(descr.clazz);
            }
        }
        static std::string &get_path() {
            if (descr.path.size()==0) {
                ERR("Tried to get path of unregister C++ class %s", typeid(Tc).name());
                throw std::runtime_error("Cannot get unregistred class' path");
            }
            return descr.path;
        }
        static void set_path(std::string &p) {
            descr.path = p;
        }
        static void ref_class(JNIEnv *e) {
            if (descr.clazz) return; // already referenced
            class_holder.push_back(&descr);
            jclass c = e->FindClass(get_path().c_str());
            if (!c) {
                ERR("Could not find java class '%s' for C++ class %s", descr.path.c_str(), typeid(Tc).name());
                throw std::runtime_error("Could not find java class");
            }
            if constexpr(full_object) {
                descr.clazz = (jclass)e->NewGlobalRef((jobject)c);
                descr.constr_id = e->GetMethodID(descr.clazz, "<init>", "()V");
                if (!descr.constr_id) {
                    ERR("Could not find constructor id java class %s", descr.path.c_str());
                    throw std::runtime_error("Could not find constructor id");
                }
            }
        }
    };
    template<class Tc, bool full_object>
    class_ref_info class_factory<Tc, full_object>::descr = {NULL, NULL, ""};

    void unreference_classes(JNIEnv *e) {
        for (auto c : class_holder) {
            e->DeleteGlobalRef((jobject)c->clazz);
            c->clear();
        }
        class_holder.clear();
    }

    /*
     *  Extracts C++ object from java object
     * */
    int jvm_object_offset; /* shall be set up by java_types::detect_object_offset */
    template<class To>
    inline auto object_ptr(jobject obj) {
        return (To*)((*(uint8_t**)obj)+jvm_object_offset);
    }

    /* jobject -> jobject - no conversion needed */
    template<>
    inline auto object_ptr<jobject>(jobject obj) {
        return obj;
    }

    template<class Tc>
    inline std::string _v(class_factory<Tc>* *tv) {
        return "L"+class_factory<Tc>::get_path()+";";
    }

    /*
     * passing java objects to and from C++
     * */
    template<class To>
    class object_param {
        public:
        jobject o;
        object_param(jobject o) : o(o) {}
    };
    template<class To>
    class tclass<object_param<To>> {
        public:
        jobject o;
        inline tclass(object_param<To> w) : o(w.o) {}
        inline tclass(JNIEnv *e, class_factory<To> *o) : o((jobject)o) {}
        inline operator To*() {return object_ptr<To>(o);}
        inline class_factory<To> *r(JNIEnv *e) {return (class_factory<To>*)o;}
    };

    /* type conversion to mathing java type */
    template<class T>
    using to_java_t = decltype((*(tclass<T>*)NULL).r((JNIEnv*)NULL)); 

    /* convert function arguments */
    template<class ... Targs>
    std::string v() {
        return (_v((Targs*)1) + ... + "");
    }

    /* generate function signature */
    template<class Tr, class To, class ... Targs>
    std::string v( Tr(*f)( JNIEnv*, To, Targs... ) ) {
        return "(" + v<Targs...>() + ")" + v<Tr>();
    } 


    /*
     * Function conversion
     * */

    /* handles returning non-void function */
    template<auto fpc, class To, class ... Targs_cw>
    inline auto __call_ret_fpc(To *obj, Targs_cw&... args) {
        static constexpr bool obj_func
            = !std::is_same<To*, jobject>::value;
        if constexpr (obj_func)
            return (obj->*fpc)(args...);
        else return fpc(args...);
    }

    /* handles function call with converted arguments */
    template<auto fpc, class To, class ... Targs_cw>
    inline auto __call_f( JNIEnv *e, jobject _obj, Targs_cw&... args ) {
        auto obj = object_ptr<To>(_obj);
        static constexpr bool obj_func
            = !std::is_same<To, jobject>::value;
        if constexpr(returns_void(fpc)) {
            if constexpr (obj_func) {
                (obj->*fpc)(args...);
            } else {
                fpc(args...);
            }
            return;
        } else {
            if constexpr (returns_special(fpc)) {
                fpc(obj, args...);
                return;
            } else {
                auto r = __call_ret_fpc<fpc>(obj, args...);
                return tclass<decltype(r)>(r).r(e);
            }
        }
    }

    /* no java arguments case - now template recursion can go on ;) */
    template<auto fpc, class To, class Targc_n, class ... Targs_c> 
    inline auto __call_f( JNIEnv *e, jobject obj) {
        tclass<Targc_n> v_cvt(e, obj);
        return __call_f<fpc, To, Targs_c...>(e, obj, v_cvt);
    }

    /* convert all function arguments recursively */
    template<auto fpc, class To, class Targc_n, class ... Targs_c, class Targj_n, class ... Targs_cw>
    inline auto __call_f( JNIEnv *e, jobject obj, Targj_n& arg_n, Targs_cw&... args) {
        if constexpr(!returns_void(&tclass<Targc_n>::r)) {
            tclass<Targc_n> v_cvt(e, arg_n);
            return __call_f<fpc, To, Targs_c...>(e, obj, args..., v_cvt);
        } else {
            tclass<Targc_n> v_cvt(e, obj);
            return __call_f<fpc, To, Targs_c...>(e, obj, arg_n, args..., v_cvt);
        }
    } 


    /* this function is called by JNI */
    template<auto fpc, class To, class ... Targs_c, class ... Targs_j>
    auto call_f( JNIEnv *e, jobject obj, Targs_j... args_j) {
        CPP2JAVA_TRY(
            return __call_f<fpc, To, Targs_c...>(
                e, obj, args_j...
            );
        )
    }


    /* argument types conversion( neccessary for java function signature ) */
    template<auto fpc, class To, class Tr_c, const int sz, class Targc_n, class ... Targs_c, class ... Targs_j>
    inline auto f_cvt(Targs_j&... args_j) {
        if constexpr(sz != 0) {
            if constexpr(!returns_void(&tclass<Targc_n>::r)) { /* check if the c++ argument needs java argument */
                return f_cvt<fpc, To, Tr_c, sz-1, Targs_c..., Targc_n>(args_j..., *((to_java_t<Targc_n>*)1));
            } else {
                return f_cvt<fpc, To, Tr_c, sz-1, Targs_c..., Targc_n>(args_j...);
            }
        } else {
            to_java_t<Tr_c>(*r)(JNIEnv*, jobject, Targs_j...)
                = call_f<fpc, To, Targc_n, Targs_c...>;
            return r;
        }
    }

    /* function with no arguments */
    template<auto fpc, class To, class Tr_c, const int sz>
    inline auto f_cvt() {
        to_java_t<Tr_c>(*r)(JNIEnv*, jobject) = call_f<fpc, To>;
        return r;
    }


    /* function pointer from class */
    template<auto fpc, class To, class Tr_c, class ... Targs_c>
    inline auto __f(Tr_c(To::*_fpc)(Targs_c...)) {
        return f_cvt<fpc, To, Tr_c, sizeof...(Targs_c), Targs_c...>();
    }

    /* function not from class */
    template<auto fpc, class Tr_c, class ... Targs_c>
    inline auto __f(Tr_c(*_fpc)(Targs_c...)) {
        return f_cvt<fpc, jobject, Tr_c, sizeof...(Targs_c), Targs_c...>();
    }

    /* handles special function - operating on object but not declared inside object */
    template<auto fpc, class To, class ... Targs_c>
    inline auto __f(special_object_func(*_fpc)(To*, Targs_c...)) {
        return f_cvt<fpc, To, special_object_func, sizeof...(Targs_c), Targs_c...>();
    } 


    /* convert function to a form callable by JNI */
    template<auto fpc>
    auto f() {
        return __f<fpc>(fpc);
    } 


    /* get new object instance */
    template<class T, class ... Targs>
    inline object_param<T> get_instance(JNIEnv *e, Targs ... args) {
        jobject obj = class_factory<T>::alloc(e);
        new (object_ptr<T>(obj)) T(args...);
        return obj;
    }


    /* destructor wrapper */
    template<class T>
    inline auto destruct(T *p) {
        p->~T();
        return special_object_func();
    }

};

#endif /* __JAVA_TYPES_H_ */

