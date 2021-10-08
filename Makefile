all: gccjit libjit lightning mir

CFLAGS += -g -O2
mir: mir.c
	$(CC) $(CFLAGS) $< -o $@ -static -lmir -pthread

gccjit: gccjit.c
	$(CC) $(CFLAGS) $< -o $@ -lgccjit

lightning: lightning.c
	$(CC) $(CFLAGS) $< -o $@ -llightning

libjit: libjit.c
	$(CC) $(CFLAGS) $< -o $@ -ljit

clean:
	$(RM) libjit lightning gccjit mir
