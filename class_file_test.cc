#include <iostream>
#include <fstream>
#include "class_file.h"
#include "wrap_class.h"

using namespace std;

int save_ret_val;
template<class Tr, class Targs...>
Tr sum_func(Targs ... args) {
    save_ret_val += args + ... + 1;
    return save_ret_val;
}

template<class Tr, class Targs...>
class tst {
    public:
    Tr a = 1;
    tst(Targs ... args) {
        a += sum_func(args...);
        save_ret_val = a;
    }
    Tr sum(Targs ... args) {
        auto r = args + ... + 1;
        a += r;
        save_ret_val = a;
        return r;
    }
    void sumv(Targs ... args) {
        a += args + ... + 1;
    }
    ~tst() {
        a += 1;
        save_ret_val = a;
    }
};

inline auto ooe(tst *p) {
    new (p) tst();
    //p->>~tst();
    return java_types::special_object_func();
}

void ppp(int a1, int a2) {
}

int main() {
    class_file f("eeelo");
    for (int i=0; i<1000; i++)
        f.var("I");
    f.build().to_file("eeelo.class");
    auto jf = java_types::f<oo>();
    auto jf2 = java_types::f<&tst::rr>();
    jf(NULL, NULL, 1, 2, true, 4);
    printf("%p\n", jf);
    //cout << jf(1, 2, true, 4) << endl;
    cout << java_types::v(jf) << endl;
    cout << java_types::v(jf2) << endl;
    tst *yy = NULL;
    jf2(NULL, (jobject*)&yy, 2, 1);

    auto fs = java_types::f<ooe>();
    tst a;
    tst *p = (tst*)(((char*)&a)-8);
    fs(NULL, (jobject*)&p);
    cout << java_types::v(fs) << endl;
    ppp(tst().u(), tst().u());
    cout << "kkkkk" << endl;
    wrap_class<tst> cw("eeeeeelo");
    //cout << &tst::tst << endl;
    return 0;
}

