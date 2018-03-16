#ifndef __JAVA_TYPES_H_
#define __JAVA_TYPES_H_

#include "define.h"

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
    template<class T>
    inline std::string _v();

#define VAR_DESCR(t, d)        \
    template <>                 \
    inline std::string _v<t>() {  \
        return STR(d);            \
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
     *  Extracts C++ object from java object
     * */
    int jvm_object_offset; /* TODO: detect, from GetFieldId ? */
    template<class To>
    inline auto object_ptr(jobject* obj) {
        return (To*)(((uint8_t*)*obj)+jvm_object_offset);
    }

    template<class T>
    using to_java_t = decltype((*(tclass<T>*)NULL).r((JNIEnv*)NULL));

    template<>
    inline auto object_ptr<jobject>(jobject* obj) {
        return obj;
    }

    template<class ... Targs>
    std::string v() {
        return (_v<Targs>() + ... + "");
    }

    template<class Tr, class To, class ... Targs>
    std::string v( Tr(*f)( JNIEnv*, To, Targs... ) ) {
        return "(" + v<Targs...>() + ")" + v<Tr>();
    }

    template<auto fpc, class To, class Targc_n, class ... Targs_c, class Targj_n, class ... Targs_cw>
    inline auto __call_f( JNIEnv *e, To *obj, Targj_n& arg_n, Targs_cw&... args) {
        tclass<Targc_n> v_cvt(e, arg_n);
        return __call_f<fpc, To, Targs_c...>(e, obj, args..., v_cvt);
    }

    template<auto fpc, class To, class ... Targs_cw>
    inline auto __call_ret_fpc(To *obj, Targs_cw&... args) {
        static constexpr bool obj_func
            = !std::is_same<To, jobject>::value;
        if constexpr (obj_func)
            return (obj->*fpc)(args...);
        else return fpc(args...);
    }

    template<auto fpc, class To, class ... Targs_cw>
    inline auto __call_f( JNIEnv *e, To *obj, Targs_cw&... args ) {
        static constexpr bool obj_func
            = !std::is_same<To, jobject>::value;
        if constexpr(returns_void(fpc)) { /* the actual function call */
            if constexpr (obj_func) (obj->*fpc)(args...);
                else fpc(args...);
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

    template<auto fpc, class To, class ... Targs_c, class ... Targs_j>
    auto call_f( JNIEnv *e, jobject *_obj, Targs_j... args_j) { /* this function is called by java */
        return __call_f<fpc, To, Targs_c...>(
                e, object_ptr<To>(_obj), args_j... ); // TODO: handle exceptions
    }

    /* argument types conversion( neccessary for java function signature ) */
    template<auto fpc, class To, class Tr_c, class ... Targs_c>
    inline auto f_cvt() {
        to_java_t<Tr_c>(*r)(JNIEnv*, jobject*, to_java_t<Targs_c> ...)
            = call_f<fpc, To, Targs_c...>;
        return r;
    }

    /* function pointer from class */
    template<auto fpc, class To, class Tr_c, class ... Targs_c>
    inline auto __f(Tr_c(To::*_fpc)(Targs_c...)) {
        return f_cvt<fpc, To, Tr_c, Targs_c...>();
    }

    /* function not from class */
    template<auto fpc, class Tr_c, class ... Targs_c>
    inline auto __f(Tr_c(*_fpc)(Targs_c...)) {
        return f_cvt<fpc, jobject, Tr_c, Targs_c...>();
    }

    /* handles special function - operating on object but not declareed inside object */
    template<auto fpc, class To, class ... Targs_c>
    inline auto __f(special_object_func(*_fpc)(To*, Targs_c...)) {
        return f_cvt<fpc, To, special_object_func, Targs_c...>();
    }

    template<auto fpc>
    auto f() {
        return __f<fpc>(fpc);
    }

    /* constructor wrapper */
    template<class T, class ... Targs>
    inline auto construct(T *p, Targs ... args) {
        new (p) T(args...);
        return java_types::special_object_func();
    }

    /* destructor wrapper */
    template<class T>
    inline auto destruct(T *p) {
        p->~T();
        return java_types::special_object_func();
    }

};

#endif /* __JAVA_TYPES_H_ */

