all: build

build:
	gcc -g -Wall -Werror list.o s-talk.c keyboard.c network.c screen.c -lpthread -o s-talk

valgrind: build
	valgrind --leak-check=full ./s-talk

clean:
	rm s-talk