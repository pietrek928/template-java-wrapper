PROG=Sample1

$(PROG).class: $(PROG).java
	javac -h . $(PROG).java

lib$(PROG).so: $(PROG).cc $(PROG).class
	g++ $(PROG).cc -o lib$(PROG).so -fpic -shared -O2 -I/usr/lib/jvm/java-9-openjdk/include -I/usr/lib/jvm/java-9-openjdk/include/linux

run: $(PROG).class lib$(PROG).so
	java -Djava.library.path=`pwd` -cp . $(PROG)

clean:
	rm -f $(PROG).class lib$(PROG).so $(PROG).h

