import java.lang.Runnable;
import test.teeest.test.teeest;

class elo { 

    static void test1() {
        teeest v = teeest.getInstance();
        v.ooo(1, 2);
    }

    static void test2() {
        teeest v = teeest.getInstance(4);
        v.kkk();
        v.sumv( v.ooo(4, 5), (short)3);
        v.sumv(2, (short)3);
    }

    static void test3() {
        teeest v = teeest.getInstance();
        try {
            v.err_func();
        } catch(Exception e) {
            System.out.println(e);
            v.sumv(1, (short)2);
        }
    }

    static void test4() {
        teeest v = teeest.getInstance();
        try {
            v.err_func_strange();
        } catch(Exception e) {
            System.out.println(e);
            v.sumv(1, (short)2);
        }
    }

    static void test_wrap(Runnable test, int expected) {
        int vp = teeest.get_ret_val();
        test.run();
        System.gc();
        System.runFinalization();
        assert teeest.get_ret_val()-vp == expected;

        System.out.println(test.toString() + " OK");
    }

    public static void main(String[] args) {
        System.loadLibrary("teeest");

        test_wrap(elo::test1, 6);
        test_wrap(elo::test1, 6);
        test_wrap(elo::test2, 35);
        test_wrap(elo::test2, 35);
        test_wrap(elo::test3, 8);
        test_wrap(elo::test4, 8);

    }

}

