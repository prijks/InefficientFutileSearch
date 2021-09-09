CC=gcc
LIBS=-lgmp

DEPS = knownfactors.h pn.h

OBJ = factorpn.o knownfactors.o pn.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

factorpn: $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

clean:
	rm -f *.o core factorpn