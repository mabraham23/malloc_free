NAME=malloc_free
CFLAGS=gcc -Wall -Werror -Wno-unknown-pragmas

.PHONY: test

all: $(NAME)

run: $(NAME)
	./$(NAME).exe

test: $(NAME)
	./$(NAME).exe test

$(NAME): main.o malloc_free.o tests.o
	$(CFLAGS) -o $(NAME).exe main.o malloc_free.o tests.o

main.o: main.c main.h
	$(CFLAGS) -c main.c

malloc_free.o: malloc_free.c malloc_free.h
	$(CFLAGS) -c malloc_free.c

tests.o: tests.c tests.h
	$(CFLAGS) -c tests.c

clean:
	rm -f *.o *.exe
