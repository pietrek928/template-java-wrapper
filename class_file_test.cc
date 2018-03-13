#include <iostream>
#include <fstream>
#include "class_file.h"
#include "wrap_class.h"

using namespace std;

inline int oo(int a, int b, bool c, unsigned char d) {
    cout << "ooooooo" << endl;
    return a+b+c+d;
}

class tst {
    int a;
    public:
    tst() {
        a = 123;
        cout << a << endl;
    }
    int u() {return 2;}
        void rr( int a, bool b ) {
            cout << a+b << endl;
        }
    ~tst() {
        cout << "]]]" << endl;
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
    //wrap_class<tst> cw("eeeeeelo");
    //cout << &tst::tst << endl;
    return 0;
}

