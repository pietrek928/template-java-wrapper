all:
	make -C test

.PHONY: test clean bench

test:
	make -C test test

bench:
	make -C benchmark bench

clean:
	make -C test clean
	make -C benchmark clean

