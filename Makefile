
CC = /usr/bin/clang++
SRC_DIR = src/
OUT_DIR = _build/

CFLAGS = -g -std=c++11

SRC_Names = global main input output state literal evaluatelit search determinate order \
 	join utility finddef interpret prune constants

SRC = $(addprefix $(SRC_DIR), $(addsuffix .cpp, $(SRC_Names)))
OBJ = $(addprefix $(OUT_DIR), $(notdir $(SRC:.cpp=.o)))

foil: $(OBJ) Makefile
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(OUT_DIR)%.o: $(SRC_DIR)%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ): $(SRC_DIR)/defns.h $(SRC_DIR)/extern.h

clean:
	rm $(OBJ)
