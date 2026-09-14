all: main

main: main.o vaderSentiment.o
    gcc -o main main.o vaderSentiment.o

main.o: main.c vaderSentiment.h
    gcc -c main.c

vaderSentiment.o: vaderSentiment.c vaderSentiment.h
    gcc -c vaderSentiment.c

clean:
    rm -f *.o main
