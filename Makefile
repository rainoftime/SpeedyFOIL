
CC = /usr/bin/clang++
SRC_DIR = src/
OUT_DIR = _build/

CFLAGS = -g 

SRC_Names = global main input output state literal evaluatelit search determinate order \
 	join utility finddef interpret prune constants

SRC = $(addprefix $(SRC_DIR), $(addsuffix .cpp, $(SRC_Names)))
OBJ = $(addprefix $(OUT_DIR), $(notdir $(SRC:.cpp=.o)))

foil: $(OBJ) Makefile
	$(CC) -g $(OBJ) -o $@

$(OUT_DIR)%.o: $(SRC_DIR)%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ): $(SRC_DIR)/defns.i $(SRC_DIR)/extern.i

clean:
	rm $(OBJ)
