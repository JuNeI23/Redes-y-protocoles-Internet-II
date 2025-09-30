# compilateur et options
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Iinclude

# dossiers
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# sources et objets
SRCS        := $(wildcard $(SRC_DIR)/*.c)
OBJS        := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
PROTO_OBJ   := $(OBJ_DIR)/protocol.o

# exécutables détectés automatiquement (client_* et server_*)
PROGS := $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%,$(filter $(SRC_DIR)/client_%.c $(SRC_DIR)/server_%.c,$(SRCS)))

.PHONY: all clean
all: $(PROGS)

# règle générique .c -> .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# protocol.o dépend explicitement du header
$(PROTO_OBJ): $(SRC_DIR)/protocol.c include/protocol.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# lien de chaque binaire avec son objet + protocol.o
$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(PROTO_OBJ) | $(BIN_DIR)
	$(CC) $^ -o $@

# création des dossiers
$(OBJ_DIR):
	@mkdir -p $@

$(BIN_DIR):
	@mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
