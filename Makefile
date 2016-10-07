
SRC_DIR = src/
OUT_DIR = _build/

CFLAGS = -g

SRC_Names =	global.c main.c input.c output.c state.c\
 	literal.c evaluatelit.c search.c determinate.c order.c\
 	join.c utility.c finddef.c interpret.c prune.c constants.c

SRC = $(addprefix $(SRC_DIR), $(SRC_Names))
OBJ = $(addprefix $(OUT_DIR), $(notdir $(SRC:.c=.o)))

foil: $(OBJ) Makefile
	cc -g -o foil $(OBJ) -lm

$(OUT_DIR)%.o: $(SRC_DIR)%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ): $(SRC_DIR)/defns.i $(SRC_DIR)/extern.i

clean:
	rm $(OBJ)
