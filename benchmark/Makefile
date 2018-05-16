PROG=bench
JAVA_PROG=path/bench_test
TEST_CLASSPATH=path/bench/$(PROG)

HDRS=../java_types.h ../class_file.h ../wrap_class.h $(PROG).cc $(PROG).cc

all: lib$(PROG).so

CCFLAGS=-I"." -I".." -I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/linux" -std=c++17 -Wall -O2 -S
GPP=clang++ $(CCFLAGS)

$(JAVA_PROG).class: lib$(PROG).so $(JAVA_PROG).java
	javac -cp ./path $(JAVA_PROG).java

lib$(PROG).so: $(PROG).cc $(HDRS)
	$(GPP) $(PROG).cc -o lib$(PROG).so -fpic -shared -s -DPACKAGE_REGISTER_METHODS

gen-classes: $(PROG).cc $(HDRS)
	$(GPP) $(PROG).cc -o gen-classes -s -DPACKAGE_WRITE_CLASS -DCLASSPATH_ROOT=path

$(TEST_CLASSPATH).class: gen-classes
	./gen-classes

bench: $(TEST_CLASSPATH).class lib$(PROG).so $(JAVA_PROG).class
	java -ea -Djava.library.path=. -cp ./path bench_test

clean:
	rm -rf path/test lib$(PROG).so gen-classes $(JAVA_PROG).class

