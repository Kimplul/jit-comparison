all: gccjit libjit lightning mir lightening-exe bytecode

CFLAGS += -Wall -Wextra -g -O2 -march=native -flto
mir: mir.c
	$(CC) $(CFLAGS) $< -o $@ -static -lmir -pthread

gccjit: gccjit.c
	$(CC) $(CFLAGS) $< -o $@ -lgccjit

lightning: lightning.c
	$(CC) $(CFLAGS) $< -o $@ -llightning

lightening-exe: lightening.c
	$(CC) $(CFLAGS) -c lightening/lightening/lightening.c -o lightening.o
	$(CC) $(CFLAGS) $< lightening.o -o $@

libjit: libjit.c
	$(CC) $(CFLAGS) $< -o $@ -ljit

bytecode: bytecode.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) libjit lightning lightening-exe gccjit mir lightening.o
