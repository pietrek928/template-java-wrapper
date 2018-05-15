PROG=teeest
JAVA_PROG=test_classpath/elo
TEST_CLASSPATH=test_classpath/test/teeest/test/$(PROG)

HDRS=../java_types.h ../class_file.h ../wrap_class.h teeest.cc teeest.cc

all: lib$(PROG).so

CCFLAGS=-I. -I.. -I/usr/lib/jvm/java-10-oracle/include -I/usr/lib/jvm/java-10-oracle/include/linux -std=c++17
GPP=g++-7 $(CCFLAGS)

$(JAVA_PROG).class: lib$(PROG).so $(JAVA_PROG).java
	javac -cp ./test_classpath $(JAVA_PROG).java

lib$(PROG).so: $(PROG).cc $(HDRS)
	$(GPP) $(PROG).cc -o lib$(PROG).so -fpic -shared -O2 -s -DPACKAGE_REGISTER_METHODS

gen-classes: $(PROG).cc $(HDRS)
	$(GPP) $(PROG).cc -o gen-classes -O2 -s -DPACKAGE_WRITE_CLASS -DCLASSPATH_ROOT=test_classpath

$(TEST_CLASSPATH).class: gen-classes
	./gen-classes

test: $(TEST_CLASSPATH).class lib$(PROG).so $(JAVA_PROG).class
	java -ea -Djava.library.path=. -cp ./test_classpath elo

clean:
	rm -rf test_classpath/test lib$(PROG).so gen-classes $(JAVA_PROG).class

