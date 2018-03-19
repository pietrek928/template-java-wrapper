
int save_ret_val;
template<class ... Targs>
auto sum_func(Targs ... args) {
    save_ret_val += (args + ... + 1);
    return save_ret_val;
}

short ooo(int a, int b, JNIEnv *e) {
    save_ret_val += a+b;
    return a+b;
}

int yyy(int a, JNIEnv *e, bool b) {
    save_ret_val += a+b;
    return a+b;
}

void kkk(JNIEnv *e) {
    save_ret_val += 1;
}

auto get_ret_val() {
    return save_ret_val;
}

template<class Tr>
class teeest {
    public:
    Tr a = 1;
    teeest() {
        a += sum_func();
        save_ret_val = a;
    }
    teeest(int oo) {
        a += sum_func(oo);
        save_ret_val = a;
    }
    teeest(int oo, int pp) {
        a += sum_func(oo, pp);
        save_ret_val = a;
    }
    template<class ... Targs>
    Tr sum(Targs ... args) {
        auto r = (args + ... + 1);
        a += r;
        save_ret_val = a;
        return r;
    }
    template<class ... Targs>
    void sumv(Targs ... args) {
        a += (args + ... + 1);
        save_ret_val = a;
    }
    ~teeest() {
        a += 1;
        save_ret_val = a;
    }
};

