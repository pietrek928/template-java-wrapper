all:
	make -C test

.PHONY: test clean

test:
	make -C test test

clean:
	make -C test clean

