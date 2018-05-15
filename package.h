#ifndef __PACKAGE_H_
#define __PACKAGE_H_

#include <fstream>
#include <functional>

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
    JNIEnv* e = NULL;
    if (vm->GetEnv((void**)&e, USE_JNI_VERSION) != JNI_OK || !e) {
        ERR("Could not get JNIEnv while loading module");
        return JNI_FALSE;
    }
    CPP2JAVA_TRY(
        java_types::detect_object_offset(e);
        __load_classes(e);
        INFO("Module loaded");
        return USE_JNI_VERSION;
    )
    return JNI_FALSE;
}

void JNICALL JNI_OnUnload(JavaVM *vm, void* /*reserved*/) {
    JNIEnv* e = NULL;
    if (vm->GetEnv((void**)&e, USE_JNI_VERSION) != JNI_OK || !e) {
        ERR("Could not get JNIEnv while unloading module");
        return;
    }
    CPP2JAVA_TRY(
        java_types::unreference_classes(e);
        INFO("Module unloaded");
    )
}

#define JNI_ENV_ARGS_N , e
#define JNI_ENV_ARGS_T , JNIEnv*
#define JNI_ENV_ARGS_F , JNIEnv* e

#else /* defined(PACKAGE_REGISTER_CLASS) || defined(PACKAGE_REGISTER_METHODS) */

/* generate classes on classpath */
#define LOAD_CLASSES_DECL void __load_classes()
LOAD_CLASSES_DECL;
int main(int argc, char *argv[]) {
    __load_classes();
    return 0;
}

#define JNI_ENV_ARGS_N , __proj_dir
#define JNI_ENV_ARGS_T , std::string&
#define JNI_ENV_ARGS_F , std::string &__proj_dir

#endif /* defined(PACKAGE_REGISTER_CLASS) || defined(PACKAGE_REGISTER_METHODS) */

#ifdef PACKAGE_WRITE_CLASS
#undef PACKAGE_WRITE_CLASS
#define PACKAGE_WRITE_CLASS .store(__proj_dir)
#else
#define PACKAGE_WRITE_CLASS
#endif /* PACKAGE_WRITE_CLASS */

#ifdef PACKAGE_REGISTER_CLASS
#undef PACKAGE_REGISTER_CLASS
#define __BUILD_CLASS reg_class(e) // !!!!!!!!!
#else
#define PACKAGE_REGISTER_CLASS 
#endif /* PACKAGE_REGISTER_CLASS */

#ifdef PACKAGE_REGISTER_METHODS
#undef PACKAGE_REGISTER_METHODS
#define PACKAGE_REGISTER_METHODS .reg_methods(e)
#else
#define PACKAGE_REGISTER_METHODS
#endif /* PACKAGE_REGISTER_METHODS */


#define PACKAGE_ROOT(p, ...)                   \
LOAD_CLASSES_DECL {                             \
    std::vector<std::pair<std::function<void(std::string& JNI_ENV_ARGS_T)>, std::string>> F; \
    std::string __package_path = STR(p) ".",     \
                __proj_dir = STR(CLASSPATH_ROOT); \
    __VA_ARGS__;                                   \
}

#define PACKAGE(p, ...)       \
{                              \
    auto &rn = __package_path;  \
    std::string __package_path   \
        = rn + ( STR(p) "." );    \
    __VA_ARGS__;                   \
    for (auto f : F) f.first(f.second JNI_ENV_ARGS_N); \
}

#define CLASSN(cj, cc, ...)              \
{                                         \
    std::string __class_path = __package_path+STR(cj); \
    std::replace(__class_path.begin(), __class_path.end(), '.', '/'); \
    java_types::class_factory<cc>::set_path(__class_path); \
    F.push_back(std::make_pair([](std::string &__class_path JNI_ENV_ARGS_F) { \
        wrap_class<cc>(__class_path) \
            __VA_ARGS__                         \
            PACKAGE_WRITE_CLASS                  \
            PACKAGE_REGISTER_CLASS                \
            PACKAGE_REGISTER_METHODS;              \
    }, __class_path)); \
}

#define CLASS(cn, ...) CLASSN(cn, cn, __VA_ARGS__)

#endif /* __PACKAGE_H_ */

