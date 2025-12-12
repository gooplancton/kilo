FORTH_DIR = ./forth
FORTH_SRCS = $(FORTH_DIR)/ForthParser.c $(FORTH_DIR)/ForthObject.c $(FORTH_DIR)/ForthInterpreter.c $(FORTH_DIR)/ForthBuiltins.c
FORTH_INCLUDE = -I$(FORTH_DIR)

LSP_DIR = ./forth_lsp
LSP_SRCS = $(LSP_DIR)/cJSON.c $(LSP_DIR)/jsonRPC.c $(LSP_DIR)/LspServer.c
LSP_INCLUDE = -I$(LSP_DIR)

all: kilo forth_standalone forth_lsp

kilo_vanilla: kilo.c
	$(CC) -o kilo kilo.c -Wall -W -pedantic -std=c11

forth_standalone: forth.c $(FORTH_SRCS)
	$(CC) $(FORTH_INCLUDE) -o forth_standalone forth.c $(FORTH_SRCS) -Wall -W -pedantic -std=c11

kilo: kilo.c $(FORTH_SRCS)
	$(CC) -DPLUGINS_ENABLED=1 $(FORTH_INCLUDE) -o kilo kilo.c $(FORTH_SRCS) -Wall -W -pedantic -std=c11

forth_lsp: $(LSP_SRCS) $(FORTH_SRCS)
	$(CC) $(LSP_INCLUDE) $(FORTH_INCLUDE) -o forth_ls forth_ls.c $(LSP_SRCS) $(FORTH_SRCS) -Wall -W -pedantic -std=c11 -lm

clean:
	rm -f kilo forth_standalone forth_lsp

