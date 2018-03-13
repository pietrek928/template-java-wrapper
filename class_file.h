#ifndef __CLASS_FILE__
#define __CLASS_FILE__

#include <vector>
#include <algorithm>
#include <string>

#include <stdint.h>
#include <jni.h>

#include "define.h"

#include "java_types.h"

typedef uint8_t  u1;
typedef uint16_t u2;
typedef uint32_t u4;

#define BUILD(...)              \
const static int composable = 1; \
template<class Tr>                \
void compose(Tr &v) {              \
v                                   \
__VA_ARGS__;                         \
}                                     \

class class_file {

    /*
     * file structures, copy-pasted from
     * https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html
     * */
    class cp_info;
    class field_info;
    class method_info;
    class attribute_info;

    template<class T>
    class entry_list : public std::vector<T> {
        public:
        T &add() {
            this->resize(this->size()+1);
            return this->back();
        }
    };

    class cp_info {
        public:
        u1 tag;
        entry_list<u1> data;

        enum tag_type {
            Utf8 = 1,
            Class = 7,
        };

        static void init_str( cp_info &t, std::string s ) {
            t.tag = Utf8;
            t.data.clear();
            t.data.insert(t.data.end(), s.begin(), s.end());
        }

        static void init_class( cp_info &t, u2 name_index ) {
            t.tag = Class;
            t.data.clear();
            t.data.insert(t.data.end(), (u1*)&name_index, (u1*)(&name_index  +1));
        }
        
        BUILD(
            .add(tag);
            switch( tag ) {
                case Utf8: v.add(data); break;
                case Class: v.add(*(u2*)&*data.begin()); break;
                default:
                    // TODO: support rest
                    break;
            }
        )

    };

    class field_info {
        public:
        u2 access_flags;
        u2 name_index;
        u2 descriptor_index;
        entry_list<attribute_info> attributes;

        BUILD(
            .add(access_flags)
            .add(name_index)
            .add(descriptor_index)
            .add(attributes)
        )
    };

    class attribute_info {
        public:
        u2 attribute_name_index;
        entry_list<u1> info;

        BUILD(
            .add(attribute_name_index)
            .template add<u4>(info)
        )
    };

    class method_info {
        public:
        u2 access_flags;
        u2 name_index;
        u2 descriptor_index;
        entry_list<attribute_info> attributes;

        BUILD(
            .add(access_flags)
            .add(name_index)
            .add(descriptor_index)
            .add(attributes)
        )
    };

    class class_file_builder : public std::vector<u1> {
        public:

        template<class Tc>
        inline decltype(std::declval<Tc>().compose(*(class_file_builder*)NULL),
                *(class_file_builder*)NULL)
                    &add(Tc &v) {
            v.compose(*this);
            return *this;
        }

        ATTR_CHECKER(composable)
        template<class Tl=u2, class Tv>
        inline class_file_builder &add(entry_list<Tv> &v) {
            add(Tl(v.size()));
            if constexpr(hasattr_t(Tv, composable)) {
                for (auto &i : v) i.compose(*this);
            } else
                insert(end(), (u1*)&*v.begin(), (u1*)&*v.end());
            return *this;
        }

        inline auto &add(u1 v) {
            push_back(v);
            return *this;
        }

        inline auto &add(u2 v) {
            push_back(v>>8);
            push_back(v);
            return *this;
        }

        inline auto &add(u4 v) {
            push_back(v>>24);
            push_back(v>>16);
            push_back(v>>8);
            push_back(v);
            return *this;
        }

#ifdef _GLIBCXX_FSTREAM
        void to_file(std::string name) {
            std::ofstream F(name, std::ofstream::binary);
            F.write((const char *)&*begin(), size());
            F.close();
        }
#endif
    };

    class ClassFile {
        class const_pool_inc_size {
            public:
            u2 v;
            const_pool_inc_size(u2 sz) : v(sz+1) {}
            operator u2() {return v;}
        };

        public:

        const static u4 class_file_magic = 0xCAFEBABE; 

        u4 magic = class_file_magic;
        u2 minor_version = 0;
        u2 major_version = 47;
        entry_list<cp_info> constant_pool;
        u2 access_flags = java_access_flags::PUBLIC;
        u2 this_class = 0;
        u2 super_class = 0;
        entry_list<u2> interfaces;
        entry_list<field_info> fields;
        entry_list<method_info> methods;
        entry_list<attribute_info> attributes;

        BUILD(
            .add(magic)
            .add(minor_version)
            .add(major_version)
            .template add<const_pool_inc_size>(constant_pool)
            .add(access_flags)
            .add(this_class)
            .add(super_class)
            .add(interfaces)
            .add(fields)
            .add(methods)
            .add(attributes)
        )
    };

    typedef struct{
        std::string name;
        std::string descr;
        void *fp;
    } native_method;

    public:
    ClassFile file_descr;

    std::string name;
    std::string path;
    std::vector<native_method> native_methods; 

    auto build() {
        class_file_builder r;
        file_descr.compose(r);
        return r;
    }

    void load_class(JNIEnv *e) {
        auto class_data = build();
        jclass clazz = e->DefineClass(name.c_str(), NULL, (const jbyte*)&*class_data.begin(), class_data.size());
        if (!clazz) {} // TODO: !!!!!!
        std::vector<JNINativeMethod> ncvt;
        for (auto &n : native_methods) {
            JNINativeMethod md = {(char*)n.name.c_str(), (char*)n.descr.c_str(), n.fp};
            ncvt.push_back(md);
        }
        if (e->RegisterNatives(clazz, &*ncvt.begin(), ncvt.size()) < 0) {
            // TODO: !!!!!!
        }
    } 

    template<typename Tf, class ... Cargs>
    u2 new_const(Tf f, Cargs ... args) {
        auto v = file_descr.constant_pool.add();
        f(v, args...);
        return file_descr.constant_pool.size();
    }

    inline auto add_str(std::string s) {
        return new_const(cp_info::init_str, s);
    }

    int last_var_n = 0;
    auto &var(std::string t, std::string n = "", u2 access = java_access_flags::PRIVATE) {
        auto v = file_descr.fields.add();
        if (n.empty()) n = "v" + std::to_string(last_var_n++);
        v.name_index = add_str(n);
        v.descriptor_index = add_str(t);
        v.access_flags = access;
        return *this;
    }

    template<auto fp>
    auto &native(std::string n, u2 access = java_access_flags::PUBLIC) {
        access |= java_access_flags::NATIVE;
        auto v = file_descr.methods.add();
        v.access_flags = access;
        v.name_index = add_str(n);

        auto fj = java_types::f<fp>();
        auto descr = java_types::v(fj);
        v.descriptor_index = add_str(descr);

        native_methods.push_back({n, descr, (void*)fp});

        return *this;
    }

    class_file(std::string _path) {
        std::replace( _path.begin(), _path.end(), '.', '/');
        path = _path;
        auto last_delim = _path.find_last_of("/");
        name = (last_delim != name.npos ? _path.substr(last_delim+1) : _path);

        file_descr.this_class = new_const(cp_info::init_class,
                add_str(name)
        );
    }
};

#undef BUILD

#endif

