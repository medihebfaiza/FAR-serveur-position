# Makefile

GCC=gcc
FILES=src/serveur.c src/tokenRing.c
OBJ=$(FILES:%.c=%.o)

all: $(OBJ)
	$(GCC) -o ./serveur src/serveur.o
	$(GCC) -o ./token src/tokenRing.o

clean:
	rm -f src/*~ src/*.o $(BIN)

.SUFFIXES: .c .o

.zf.res:
	$(GCC) -c $<
