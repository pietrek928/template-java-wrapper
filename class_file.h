#ifndef __CLASS_FILE__
#define __CLASS_FILE__

#include <vector>
#include <algorithm>
#include <string>
#include <stdexcept>

#include <stdint.h>
#include <jni.h>

#include "define.h"

#include "java_types.h"

typedef uint8_t  u1;
typedef uint16_t u2;
typedef uint32_t u4;

#define BUILD(...)              \
constexpr static int composable = 1; \
template<class Tr>                \
void compose(Tr &v) {              \
v                                   \
 __VA_ARGS__;                        \
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
        template<class ... Targs>
        T &add(Targs... args) {
            this->emplace_back(args...);
            return this->back();
        }
    };

    class cp_info {
        public:
        u1 tag = 0;
        void *attr = NULL;

        class Methodref_info {
            public:
            u2 class_index;
            u2 name_and_type_index;

            Methodref_info(u2 class_index, u2 name_and_type_index)
                : class_index(class_index), name_and_type_index(name_and_type_index) {}

            BUILD(
                .add(class_index)
                .add(name_and_type_index)
            )
        };

        class NameAndType_info {
            public:
            u2 name_index;
            u2 descriptor_index;

            NameAndType_info(u2 name_index, u2 descriptor_index)
                : name_index(name_index), descriptor_index(descriptor_index) {}

            BUILD(
                .add(name_index)
                .add(descriptor_index)
            )
        };

        enum tag_type {
            Utf8 = 1,
            Class = 7,
            Methodref = 10,
            NameAndType = 12
        };

        static void init_str(cp_info &t, std::string s ) { // TODO: hashmap to avoid repeat
            t.tag = Utf8;
            auto *el = new entry_list<u1>;
            el->insert(el->end(), s.begin(), s.end());
            t.attr = (void*)el;
        }

        static void init_nameandtype(cp_info &t, u2 name_index, u2 descriptor_index) {
            t.tag = NameAndType;
            t.attr = (void*) new NameAndType_info(name_index, descriptor_index);
        }

        static void init_methodref(cp_info &t, u2 class_index, u2 name_index) {
            t.tag = Methodref;
            t.attr = (void*) new Methodref_info(class_index, name_index);
        }

        static void init_class( cp_info &t, u2 name_index ) {
            t.tag = Class;
            t.attr = (void*) new u2(name_index);
        }
        
        BUILD(
            .add(tag);
            switch( tag ) {
                case Utf8: v.add(*(entry_list<u1>*)attr); break;
                case Class: v.add(*(u2*)attr); break;
                case Methodref: v.add(*(Methodref_info*)attr); break;
                case NameAndType: v.add(*(NameAndType_info*)attr); break;
                default:
                    // TODO: support rest
                    break;
            }
        )

        cp_info() {}

        cp_info(cp_info &&v) {
            tag = v.tag;
            attr = v.attr;
            v.tag=0; v.attr=NULL;
        }

        ~cp_info() {
            if (attr)
            switch( tag ) {
                case Utf8: delete (entry_list<u1>*)attr; break;
                case Class: delete (u2*)attr; break;
                case Methodref: delete (Methodref_info*)attr; break;
                case NameAndType: delete (NameAndType_info*)attr; break;
                default:
                    // TODO: error
                    break;
            }
        }

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

    class exception_info {
        u2 start_pc = 0;
        u2 end_pc = 0;
        u2 handler_pc = 0;
        u2 catch_type = 0;
    };

    class named_attribute_info {
        public:
        class Code_attribute {
            public:
            u2 max_stack = 0;
            u2 max_locals = 0;
            entry_list<u1> code;
            entry_list<exception_info> exception_table;
            entry_list<attribute_info> attributes;

            BUILD(
                .add(max_stack)
                .add(max_locals)
                .template add<u4>(code)
                .add(exception_table)
                .add(attributes)
            )
        };

        enum named_attr_t {
            None=0,
            CODE
        };

        named_attr_t type = None;
        u2 name_index;
        void *attr = NULL;

        named_attribute_info(named_attr_t type, u2 name_index)
                : type(type), name_index(name_index) {
            switch(type) {
                case CODE: attr = (void*)new Code_attribute; break;
            }
        }

        auto code_attr() {
            return (Code_attribute*)attr;
        }

        BUILD(
            .add(name_index);
            class_file_builder vt;
            if (attr)
            switch(type) {
                case CODE: vt.add(*(Code_attribute*)attr); break;
            }
            v.template add<u4>(vt);
        )

        named_attribute_info() {}

        named_attribute_info(named_attribute_info &&v) {
            type = v.type;
            attr = v.attr;
            v.type=None; v.attr=NULL;
        }

        ~named_attribute_info() {
            if (attr)
            switch(type) {
                case CODE: delete (Code_attribute*)attr; break;
            }
        }
    };

    class method_info {
        public:
        u2 access_flags;
        u2 name_index;
        u2 descriptor_index;
        entry_list<named_attribute_info> attributes;

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

        ATTR_CHECKER(composable)
        template<class Tl=u2, class Tv>
        inline auto &add(entry_list<Tv> &v) {
            add(Tl(v.size()));
            if constexpr(hasattr_t(Tv, composable)) {
                for (auto &i : v) i.compose(*this);
            } else
                insert(end(), (u1*)&*v.begin(), (u1*)&*v.end()); // TODO: little endian -> big endian
            return *this;
        }

        template<class Tl=u2>
        inline auto &add(class_file_builder &v) {
            add(Tl(v.size()));
            insert(end(), (u1*)&*v.begin(), (u1*)&*v.end());
            return *this;
        }

#ifdef _GLIBCXX_FSTREAM
        auto &to_file(std::string f) {
            std::ofstream F(f, std::ofstream::binary);
            F.write((const char *)&*begin(), size());
            F.close();
            return *this;
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
        u2 major_version = 51;
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

    std::string path;
    std::vector<native_method> native_methods; 

    auto build() {
        class_file_builder r;
        file_descr.compose(r);
        return r;
    }

    jclass reg_class(JNIEnv *e) {
        auto class_data = build();
        jclass clazz = e->DefineClass(path.c_str(), NULL, (const jbyte*)&*class_data.begin(), class_data.size());
        if (!clazz) {
            ERR("class %s registering error", path.c_str());
            throw std::runtime_error("Could not register class in JNI");
        }
        return clazz;
    }

    void reg_methods(JNIEnv *e, jclass clazz) {
        std::vector<JNINativeMethod> ncvt;
        for (auto &n : native_methods) {
            JNINativeMethod md = {(char*)n.name.c_str(), (char*)n.descr.c_str(), n.fp};
            ncvt.push_back(md);
        } 
        if (e->RegisterNatives(clazz, &*ncvt.begin(), ncvt.size()) != JNI_OK) {
            ERR("Registering %d JNI methods in class %s failed", ncvt.size(), path.c_str());
            throw std::runtime_error("Could not register methods in JNI");
        }
    }

    template<typename Tf, class ... Cargs>
    u2 new_const(Tf f, Cargs ... args) {
        auto &v = file_descr.constant_pool.add();
        f(v, args...);
        return file_descr.constant_pool.size();
    }

    inline auto add_str(std::string s) {
        return new_const(cp_info::init_str, s);
    }

    int last_var_n = 0;
    auto &var(std::string t, std::string n = "", u2 access = java_access_flags::PRIVATE) {
        auto &v = file_descr.fields.add();
        if (n.empty()) n = "v" + std::to_string(last_var_n++);
        v.name_index = add_str(n);
        v.descriptor_index = add_str(t);
        v.access_flags = access;
        return *this;
    }

    void empty_constr() {
        auto &v = file_descr.methods.add();
        v.access_flags = java_access_flags::PUBLIC;
        v.name_index = add_str("<init>");
        v.descriptor_index = add_str("()V");

        // add superclass constructor reference
        u2 constr_ref = new_const(cp_info::init_methodref,
            file_descr.super_class,
            new_const(cp_info::init_nameandtype,
                add_str("<init>"),
                add_str("()V")
            )
        );
        
        // empty code
        auto &a = v.attributes.add(named_attribute_info::CODE, add_str("Code"));
        auto &descr = *a.code_attr();
        descr.max_stack = 1;
        descr.max_locals = 1;

        descr.code.add(0x2a); // aload_0

        descr.code.add(0xb7); // invokespecial
        descr.code.add(constr_ref>>8);
        descr.code.add(constr_ref&0xFF);

        descr.code.add(0xb1); // return
    }

    template<auto fp>
    auto &native(std::string n, u2 access = java_access_flags::PUBLIC) {
        access |= java_access_flags::NATIVE;
        auto &v = file_descr.methods.add();
        v.access_flags = access;
        v.name_index = add_str(n);

        auto fj = java_types::f<fp>();
        auto descr = java_types::v(fj);
        v.descriptor_index = add_str(descr);

        native_methods.push_back({n, descr, (void*)fj});

        return *this;
    }

    std::string path_dir() {
        auto last_delim = path.find_last_of("/");
        return path.substr(0, last_delim);
    }

    std::string path_name() {
        auto last_delim = path.find_last_of("/");
        return (last_delim != path.npos ? path.substr(last_delim+1) : path);
    }

    class_file(std::string _path) {
        std::replace( _path.begin(), _path.end(), '.', '/');
        path = _path;

        file_descr.this_class = new_const(cp_info::init_class,
                add_str(path)
        );
        file_descr.super_class = new_const(cp_info::init_class,
                add_str("java/lang/Object")
        );
        
        empty_constr();
    }
};

#undef BUILD

namespace java_types {
    typedef u4 java_align_t;

    int find_pos(JNIEnv *e, jclass clazz, jfieldID field_id, java_align_t m, int end = 12) {
        jobject o = e->AllocObject(clazz); // free ?
        e->SetIntField(o, field_id, m);

        java_align_t *b = *(java_align_t**)o;
        int r = 0;
        while (r < end) {
            if (b[r] == m) return r;
            r++;
        }
        return -1;
    }

    void detect_object_offset(JNIEnv *e) {
        java_align_t search_magic = 0xFE3A5638u;

        /* We assume that object offset is as aligned as java_align_t */
        class_file cf("class_path");
        cf.var("I", "vtest", java_access_flags::PUBLIC);
        jclass clazz = cf.reg_class(e);
        jfieldID field_id = e->GetFieldID(clazz, "vtest", "I");
        INFO("field id=%d", field_id);

        /* search for magic in object */
        __search: 
        int pos1 = find_pos(e, clazz, field_id, search_magic);
        int pos2 = find_pos(e, clazz, field_id, search_magic);
        if (pos1 != pos2) goto __search;

        if (pos1 == -1) {
            ERR("Looking for position field in java class failed", "");
            throw std::runtime_error("Could not jni object offset");
        }

        pos1 *= sizeof(java_align_t);
        INFO("jvm object offset: %d\n", pos1);
        jvm_object_offset = pos1;
    }
};

#endif

