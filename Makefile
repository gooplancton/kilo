all: kilo forth_standalone

kilo: kilo.c
	$(CC) -o kilo kilo.c -Wall -W -pedantic -std=c11

forth_standalone: forth.c ./forth/ForthParser.c ./forth/ForthObject.c
	$(CC) -I./forth -o forth_standalone forth.c ./forth/ForthParser.c ./forth/ForthObject.c -Wall -W -pedantic -std=c11

clean:
	rm kilo
	rm forth_standalone
