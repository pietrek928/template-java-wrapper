
#include <exception>

int save_ret_val = 1;
template<class ... Targs>
auto sum_func(Targs ... args) {
    save_ret_val += (args + ... + 1);
    return save_ret_val;
}

int yyy(int a, JNIEnv *e, bool b) {
    save_ret_val += a+b;
    return a+b;
}

short ooo(int a, int b, JNIEnv *e) {
    save_ret_val += a+b;
    return a+b;
}

void kkk(JNIEnv *e) {
    save_ret_val += 1;
}

auto get_ret_val() {
    return save_ret_val;
}

void err_func() {
    save_ret_val += 1;
    throw std::runtime_error("An error occured");
}

void err_func_strange() {
    save_ret_val += 1;
    throw 15;
}

template<class Tr>
class teeest {
    public:
    Tr a = 1;
    teeest() {
        a += 1;
    }
    teeest(int oo) {
        a += oo;
    }
    teeest(int oo, int pp) {
        a += oo+pp;
    }
    template<class ... Targs>
    Tr sum(Targs ... args) {
        auto r = (args + ... + 1);
        a += r;
        return r;
    }
    template<class ... Targs>
    void sumv(JNIEnv *e, Targs ... args) {
        a += (args + ... + 1);
    }
    ~teeest() {
        a += 1;
        save_ret_val += a;
    }
};

