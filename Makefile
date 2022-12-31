SRC := src
OBJ := obj
CFLAGS := -g -Wall 
LIBS   := -lncursesw -lmenu -lm -lpthread -lcurl
CC := cc

$(shell mkdir -p $(OBJ))
NAME := $(shell basename $(shell pwd))

SOURCES := $(wildcard $(SRC)/*.c)
OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))

all: $(OBJECTS)
	$(CC) $^ $(CFLAGS) $(LIBS) -o $@ -o $(NAME)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) -I$(SRC) $(CFLAGS) $(LIBS) -c $< -o $@
