# compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

# dossiers
SRC = src
OBJ = obj
BIN = bin

# exécutables
EXECS = $(BIN)/client_udp $(BIN)/server_udp $(BIN)/client_tcp $(BIN)/server_tcp

# règles principales
all: $(EXECS)

# règle générique : .c -> .o
$(OBJ)/%.o: $(SRC)/%.c
	@mkdir -p $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

# protocol.o
$(OBJ)/protocol.o: $(SRC)/protocol.c include/protocol.h
	@mkdir -p $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

# liens pour chaque exécutable
$(BIN)/client_udp: $(OBJ)/client_udp.o $(OBJ)/protocol.o
	@mkdir -p $(BIN)
	$(CC) $^ -o $@

$(BIN)/server_udp: $(OBJ)/server_udp.o $(OBJ)/protocol.o
	@mkdir -p $(BIN)
	$(CC) $^ -o $@

$(BIN)/client_tcp: $(OBJ)/client_tcp.o $(OBJ)/protocol.o
	@mkdir -p $(BIN)
	$(CC) $^ -o $@

$(BIN)/server_tcp: $(OBJ)/server_tcp.o $(OBJ)/protocol.o
	@mkdir -p $(BIN)
	$(CC) $^ -o $@

# nettoyage
clean:
	rm -rf $(OBJ) $(BIN)
