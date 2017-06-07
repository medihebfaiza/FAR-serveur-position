# Makefile

GCC=gcc
FILES=serveur.c client.c
OBJ=$(FILES:%.c=%.o)
BIN=serveur client

all: $(OBJ)
	$(GCC) -o serveur serveur.o
	$(GCC) -o client client.o

clean:
	rm -f *~ *.o $(BIN)

.SUFFIXES: .c .o

.zf.res:
	$(GCC) -c $<
