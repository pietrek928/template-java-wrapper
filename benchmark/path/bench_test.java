import java.lang.Runnable;
import test.bench;
import static test.bench.test1;

class bench_test { 

    private static final long n = 100000000;


    static double bench_func(String name, Runnable test) {
        long startTime = System.nanoTime();
        test.run();
        long estimatedTime = System.nanoTime() - startTime;
        double r = ((double)estimatedTime)/n;
        System.out.println("" + name + ": " + r + "ns");
        return r;
    }

    static void test_nothing() {
    }

    public static void main(String[] args) {
        System.loadLibrary("bench");

        bench t = bench.getInstance();

        bench_func("no jni func", () -> {
            for (long i=0; i<n; i++) test_nothing();
        });

        bench_func("static method", () -> {
            for (long i=0; i<n; i++) test1();
        });

        bench_func("object method", () -> {
            for (long i=0; i<n; i++) t.test1();
        });

        bench_func("object method gets int", () -> {
            for (long i=0; i<n; i++) t.test2(0);
        });

        bench_func("object method returns int", () -> {
            for (long i=0; i<n; i++) t.test3();
        });

        bench_func("object method gets int, returns int", () -> {
            for (long i=0; i<n; i++) t.test4(0);
        });

        bench_func("object method gets string", () -> {
            for (long i=0; i<n; i++) t.tests1("1234567890");
        });

        bench_func("object method returns string", () -> {
            String s;
            for (long i=0; i<n; i++) s=t.tests2();
        });

        bench_func("object method gets string returns string", () -> {
            String s = "1234567890";
            for (long i=0; i<n; i++) s=t.tests3(s);
        });
    }

}

