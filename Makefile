
CC = /usr/bin/clang++
SRC_DIR = src/
OUT_DIR = _build/

CFLAGS = -g -std=c++11 -I /usr/local/include/ -L /usr/local/lib/

SRC_Names = global main input output state literal evaluatelit search determinate order \
 	join utility finddef interpret prune constants template matching datalog

SRC = $(addprefix $(SRC_DIR), $(addsuffix .cpp, $(SRC_Names)))
OBJ = $(addprefix $(OUT_DIR), $(notdir $(SRC:.cpp=.o)))


foil: $(OBJ) Makefile
	$(CC) $(CFLAGS) $(OBJ) -lz3 -o $@

ifeq ($(MAKECMDGOALS),clean)
# doing clean, so dont make deps.
DEPS=
else
# doing build, so make deps.
DEPS = $(addprefix $(OUT_DIR), $(notdir $(SRC:.cpp=.d)))

$(OUT_DIR)%.d: $(SRC_DIR)%.cpp
	$(CC) $(CFLAGS) -MM -MT '$(patsubst $(SRC_DIR)%.cpp,$(OUT_DIR)%.o,$<)' $< > $@
	
-include $(DEPS)
endif


$(OUT_DIR)%.o: $(SRC_DIR)%.cpp
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean all

all: foil

clean:
	rm $(OBJ)
