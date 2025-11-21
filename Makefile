all: kilo forth_standalone

kilo_vanilla: kilo.c
	$(CC) -o kilo kilo.c -Wall -W -pedantic -std=c11

forth_standalone: forth.c ./forth/ForthParser.c ./forth/ForthObject.c ./forth/ForthInterpreter.c ./forth/ForthBuiltins.c
	$(CC) -I./forth -o forth_standalone forth.c ./forth/ForthParser.c ./forth/ForthObject.c ./forth/ForthInterpreter.c ./forth/ForthBuiltins.c -Wall -W -pedantic -std=c11

kilo: kilo.c ./forth/ForthParser.c ./forth/ForthObject.c ./forth/ForthInterpreter.c ./forth/ForthBuiltins.c
	$(CC) -DPLUGINS_ENABLED=1 -I./forth -o kilo kilo.c ./forth/ForthParser.c ./forth/ForthObject.c ./forth/ForthInterpreter.c ./forth/ForthBuiltins.c -Wall -W -pedantic -std=c11

clean:
	rm kilo
	rm forth_standalone
