#ifndef __EXCEPTIONS_H_
#define __EXCEPTIONS_H_


#include <exception>

inline void __throw_new(JNIEnv *e, const char *msg) {
    jclass ex_class = e->FindClass("java/lang/RuntimeException"); 
    if (!ex_class) {
        ERR("%s, \n !!!!!! Also Class by path '%s' not found", msg, "java/lang/RuntimeException");
    } else {
        e->ThrowNew(ex_class, msg);
    }
}

#define CPP2JAVA_TRY(__code__...) {  \
    try {                             \
        __code__;                      \
    } catch(JavaException e) {          \
        WARN("Java exception thrown - passing it", ""); \
    } catch(std::exception &ex) {          \
        __throw_new(e, ex.what());              \
    } catch(...) {                           \
        __throw_new(e, "Unknown exception thrown from C++"); \
    }                                              \
}

class JavaException {
    public:
    JavaException() {}
};

#define JAVA2CPP_TRY(__code__...) { \
    __code__;                        \
    if (e->ExceptionCheck()) {        \
        WARN("Java exception found - passing it", ""); \
        throw JavaException();          \
    }                                    \
}


#endif /* __EXCEPTIONS_H_ */

