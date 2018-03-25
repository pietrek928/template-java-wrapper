PROG=teeest
JAVA_PROG=test_classpath/elo
TEST_CLASSPATH=test_classpath/test/teeest/test/$(PROG)

HDRS=class_file.h wrap_class.h teeest.cc teeest.cc

all: lib$(PROG).so

CCFLAGS=-I. -I/usr/lib/jvm/java-9-openjdk/include -I/usr/lib/jvm/java-9-openjdk/include/linux -std=c++17

$(JAVA_PROG).class: $(JAVA_PROG).java
	javac -cp ./test_classpath $(JAVA_PROG).java

lib$(PROG).so: $(PROG).cc $(HDRS)
	g++ $(CCFLAGS) $(PROG).cc -o lib$(PROG).so -fpic -shared -O2 -s -DPACKAGE_REGISTER_METHODS

gen-classes: $(PROG).cc $(HDRS)
	g++ $(CCFLAGS) $(PROG).cc -o gen-classes -O2 -s -DPACKAGE_WRITE_CLASS -DCLASSPATH_ROOT=test_classpath

$(TEST_CLASSPATH).class: gen-classes
	./gen-classes

run: $(TEST_CLASSPATH).class lib$(PROG).so $(JAVA_PROG).class
	java -Djava.library.path=. -cp ./test_classpath elo

clean:
	rm -rf test_classpath/test lib$(PROG).so gen-classes $(JAVA_PROG).class

