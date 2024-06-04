CC=gcc
CFLAGS=-pthread -I.
test: LAB4.c
    $(CC) -o test pwmTests.c $(CFLAGS)
.PHONY: clean
clean:
    rm -f test.o test