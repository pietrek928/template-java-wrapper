#ifndef __PACKAGE_H_
#define __PACKAGE_H_

#include <fstream>
#include <wrap_class.h>

#define __BUILD_CLASS

#define USE_JNI_VERSION JNI_VERSION_9

#ifndef CLASSPATH_ROOT
#define CLASSPATH_ROOT "."
#endif /* CLASSPATH_ROOT */

#if defined(PACKAGE_REGISTER_CLASS) || defined(PACKAGE_REGISTER_METHODS)

#define LOAD_CLASSES_DECL void __load_classes(JNIEnv *e)
LOAD_CLASSES_DECL;
jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    try {
        JNIEnv* e = NULL;
        if (vm->GetEnv((void**)&e, USE_JNI_VERSION) != JNI_OK || !e)
            throw std::runtime_error("Could not get NJIEnv");
        java_types::detect_object_offset(e);
        __load_classes(e);
        INFO("Module loaded","")
    } catch (const std::exception &e) {
        ERR("%s",e.what())
        return JNI_FALSE;
    } catch(...) {
        ERR("Unknown exception thrown","")
        return JNI_FALSE;
    }
    return USE_JNI_VERSION;
}

#else /* defined(PACKAGE_REGISTER_CLASS) || defined(PACKAGE_REGISTER_METHODS) */

/* generate classes on classpath */
#define LOAD_CLASSES_DECL void __load_classes()
LOAD_CLASSES_DECL;
int main(int argc, char *argv[]) {
    __load_classes();
    return 0;
}

#endif /* defined(PACKAGE_REGISTER_CLASS) || defined(PACKAGE_REGISTER_METHODS) */

#ifdef PACKAGE_WRITE_CLASS
#undef PACKAGE_WRITE_CLASS
#define PACKAGE_WRITE_CLASS .store(__proj_dir)
#else
#define PACKAGE_WRITE_CLASS
#endif /* PACKAGE_WRITE_CLASS */

#ifdef PACKAGE_REGISTER_CLASS
#undef PACKAGE_REGISTER_CLASS
#define __BUILD_CLASS .reg_class(e)
#else
#define PACKAGE_REGISTER_CLASS
#endif /* PACKAGE_REGISTER_CLASS */

#ifdef PACKAGE_REGISTER_METHODS
#undef PACKAGE_REGISTER_METHODS
#define PACKAGE_REGISTER_METHODS .reg_methods(e)
#else
#define PACKAGE_REGISTER_METHODS
#endif /* PACKAGE_REGISTER_METHODS */


#define PACKAGE_ROOT(n, ...)                   \
LOAD_CLASSES_DECL {                             \
    std::string __package_name = STR(n) ".",     \
                __proj_dir = STR(CLASSPATH_ROOT); \
    __VA_ARGS__;                                   \
}

#define PACKAGE(n, ...)       \
{                              \
    auto &rn = __package_name;  \
    std::string __package_name   \
        = rn + ( STR(n) "." );    \
    __VA_ARGS__;                   \
}

#define CLASSN(cj, cc, ...)              \
{                                         \
    wrap_class<cc>(__package_name+STR(cj)) \
        __VA_ARGS__                         \
        PACKAGE_WRITE_CLASS                  \
        PACKAGE_REGISTER_CLASS                \
        PACKAGE_REGISTER_METHODS;              \
}

#define CLASS(cn, ...) CLASSN(cn, cn, __VA_ARGS__)

#endif /* __PACKAGE_H_ */

