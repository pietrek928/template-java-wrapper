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
    inline std::string v();

#define VAR_DESCR(t, d)        \
    template <>                 \
    inline std::string v<t>() {  \
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

    template<class T>
    using to_java_t = decltype((*(tclass<T>*)NULL).r((JNIEnv*)NULL));

    const static int jvm_object_offset = 8; /* TODO: detect, from GetFieldId ? */
    template<class To>
    inline auto object_ptr(jobject* obj) {
        return (To*)(((uint8_t*)*obj)+jvm_object_offset);
    }

    template<>
    inline auto object_ptr<jobject>(jobject* obj) {
        return obj;
    }

    template<class T, class T2, class ... Targs>
    std::string v() {
        std::string r = v<T>() + v<T2>();
        if constexpr(sizeof...(Targs) != 0)
            return r + v<Targs...>();
        else return r;
    }

    template<class Tr, class To, class ... Targs>
    std::string v( Tr(*f)( JNIEnv*, To, Targs... ) ) {
        return "(" + v<Targs...>() + ")" + v<Tr>();
    }

    template<auto fpc, class Targc_n, class ... Targs_c, class Targj_n, class ... Targs_cw>
    inline auto __call_f( JNIEnv *e, auto obj, Targj_n& arg_n, Targs_cw&... args) {
        tclass<Targc_n> v_cvt(e, arg_n);
        return __call_f<fpc, Targs_c...>(e, obj, args..., v_cvt);
    }

    template<auto fpc, class ... Targs_cw>
    inline auto __call_ret_fpc(auto obj, Targs_cw&... args) {
        static constexpr bool obj_func
            = !std::is_same<decltype(obj), jobject*>::value;
        if constexpr (obj_func)
            return (obj->*fpc)(args...);
        else return fpc(args...);
    }

    template<auto fpc, class ... Targs_cw>
    inline auto __call_f( JNIEnv *e, auto obj, Targs_cw&... args ) {
        static constexpr bool obj_func
            = !std::is_same<decltype(obj), jobject*>::value;
        if constexpr(returns_void(fpc)) { /* the actual function call */
            if constexpr (obj_func) (obj->*fpc)(args...);
                else fpc(args...);
            return;
        } else {
            if constexpr (returns_special(fpc))
                return fpc(obj, args...);
            else {
                auto r = __call_ret_fpc<fpc>(obj, args...);
                return tclass<decltype(r)>(r).r(e);
            }
        }
    }

    template<auto fpc, class To, class ... Targs_c, class ... Targs_j>
    auto call_f( JNIEnv *e, jobject *_obj, Targs_j... args_j) { /* this function is called by java */
        return __call_f<fpc, Targs_c...>(
                e, object_ptr<To>(_obj), args_j... ); // TODO: handle exceptions
    }

    template<const int n, auto fpc, class To, class Tr_c, class Targc_n, class ... Targs_c, class ... Targs_j>
    inline auto f_cvt(Targs_j... args_j) {
        if constexpr (n != 0) {
            return f_cvt<n-1, fpc, To, Tr_c, Targs_c..., Targc_n>(
                    args_j...,
                    *(to_java_t<Targc_n>*)1/*clang warns here for NULL, 1 suspends it ;pp*/);
        } else {
            to_java_t<Tr_c>(*r)(JNIEnv*, jobject*, Targs_j...)
                = call_f<fpc, To, Targc_n, Targs_c...>;
            return r;
        }
    }

    template<const int n, auto fpc, class To, class Tr_c>
    inline auto f_cvt() {
        to_java_t<Tr_c>(*r)(JNIEnv*, jobject*) = call_f<fpc, To>;
        return r;
    }

    /* function pointer from class */
    template<auto fpc, class To, class Tr_c, class ... Targs_c>
    inline auto __f(Tr_c(To::*_fpc)(Targs_c...)) {
        return f_cvt<sizeof...(Targs_c), fpc, To, Tr_c, Targs_c...>();
    }

    /* function not from class */
    template<auto fpc, class Tr_c, class ... Targs_c>
    inline auto __f(Tr_c(*_fpc)(Targs_c...)) {
        return f_cvt<sizeof...(Targs_c), fpc, jobject, Tr_c, Targs_c...>();
    }

    /* handles special function - operating on object but not from object */
    template<auto fpc, class To, class ... Targs_c>
    inline auto __f(special_object_func(*_fpc)(To*, Targs_c...)) {
        return f_cvt<sizeof...(Targs_c), fpc, To, void, Targs_c...>();
    }

    template<auto fpc>
    auto f() {
        return __f<fpc>(fpc);
    }

};

#endif /* __JAVA_TYPES_H_ */

