PROG=teeest
JAVA_PROG=test_classpath/elo

all: lib$(PROG).so

CCFLAGS=-I. -I/usr/lib/jvm/java-9-openjdk/include -I/usr/lib/jvm/java-9-openjdk/include/linux -std=c++17

$(JAVA_PROG).class: $(JAVA_PROG).java
	javac -cp ./test_classpath $(JAVA_PROG).java

lib$(PROG).so: $(PROG).cc class_file.h
	g++ $(CCFLAGS) $(PROG).cc -o lib$(PROG).so -fpic -shared -O2 -s -DPACKAGE_REGISTER_METHODS

gen-classes: $(PROG).cc class_file.h
	g++ $(CCFLAGS) $(PROG).cc -o gen-classes -O2 -s -DPACKAGE_WRITE_CLASS -DCLASSPATH_ROOT=test_classpath

test_classpath/test/teeest/test/$(PROG).class: gen-classes
	./gen-classes

run: test_classpath/test/teeest/test/$(PROG).class lib$(PROG).so $(JAVA_PROG).class
	java -Djava.library.path=. -cp ./test_classpath elo

clean:
	rm -f ./test_classpath/test/teeest/test/$(PROG).class lib$(PROG).so gen-classes $(JAVA_PROG).class

