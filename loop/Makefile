all: gccjit libjit lightning mir lightening bytecode bcgen copyjit

CFLAGS += -Wall -Wextra -g -O2

# some common tools
CC	= gcc
RM	= rm

mir: mir.c
	$(CC) $(CFLAGS) $< -o $@ -static -lmir -pthread

gccjit: gccjit.c
	$(CC) $(CFLAGS) $< -o $@ -lgccjit

lightning: lightning.c
	$(CC) $(CFLAGS) $< -o $@ -llightning

copyjit: copyjit.c
	cd ../lib/copyjit && $(MAKE) RELEASE=1
	$(CC) $(CFLAGS) $< ../lib/copyjit/src/copyjit.c -o $@

lightening: lightening.c
	$(CC) $(CFLAGS) -c ../lib/lightening/lightening/lightening.c -o lightening.o
	$(CC) $(CFLAGS) $< lightening.o -o $@

libjit: libjit.c
	$(CC) $(CFLAGS) $< -o $@ -ljit

bytecode: bytecode.c
	$(CC) $(CFLAGS) $< -o $@

bcgen: bcgen.c
	../lib/bcgen/bcgen bcgen.py
	$(CC) $(CFLAGS) $< bcode.c -o $@

clean:
	$(RM) -rf bcgen bcode.c bcode.h bcode.o
	$(RM) -rf lightening lightening.o
	$(RM) -rf libjit lightning gccjit mir copyjit bytecode
