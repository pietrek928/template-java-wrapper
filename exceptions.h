#ifndef __EXCEPTIONS_H_
#define __EXCEPTIONS_H_


#include <exception>

namespace java_exceptions {

inline void throw_new(JNIEnv *e, const char *msg) {
    jclass ex_class = e->FindClass("java/lang/RuntimeException"); 
    if (!ex_class) {
        ERR("%s, \n !!!!!! Also Class by path '%s' not found", msg, "java/lang/RuntimeException");
    } else {
        e->ThrowNew(ex_class, msg);
    }
}

class JavaException {
    public:
    JavaException() {}
};

// propagate detected java exception
void propagate_exception() {
    WARN("Java exception found - passing it", "");
    throw java_exceptions::JavaException();
}

}

#define CPP2JAVA_TRY(__code__...) {  \
    try {                             \
        __code__;                      \
    } catch(java_exceptions::JavaException e) {          \
        WARN("Java exception thrown - passing it", ""); \
    } catch(std::exception &ex) {          \
        java_exceptions::throw_new(e, ex.what());              \
    } catch(...) {                           \
        java_exceptions::throw_new(e, "Unknown exception thrown from C++"); \
    }                                              \
}

#define JEX {if (e->ExceptionCheck()) java_exceptions::propagate_exception();}


#endif /* __EXCEPTIONS_H_ */

