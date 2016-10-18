
CC = /usr/bin/clang++
SRC_DIR = src/
OUT_DIR = _build/

CFLAGS = -g -std=c++11

ORG_SRC_Names = global main input output state literal evaluatelit search determinate order \
 	join utility finddef interpret prune constants 

NEW_SRC_Names = template matching datalog

ORG_SRC = $(addprefix $(SRC_DIR), $(addsuffix .cpp, $(ORG_SRC_Names)))
NEW_SRC = $(addprefix $(SRC_DIR), $(addsuffix .cpp, $(NEW_SRC_Names)))

ORG_OBJ = $(addprefix $(OUT_DIR), $(notdir $(ORG_SRC:.cpp=.o)))
NEW_OBJ = $(addprefix $(OUT_DIR), $(notdir $(NEW_SRC:.cpp=.o)))

OBJ = $(ORG_OBJ) $(NEW_OBJ)

ifeq ($(MAKECMDGOALS),clean)
	# doing clean, so dont make deps.
	deps=
else
	# doing build, so make deps.
	deps=$(srcs:.cpp=.d)

endif

foil: $(OBJ) Makefile
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(OBJ): $(SRC_DIR)/defns.h $(SRC_DIR)/extern.h

#$(NEW_OBJ):$(OUT_DIR)%.o: $(SRC_DIR)%.h
$(NEW_OBJ): $(SRC_DIR)/template.h $(SRC_DIR)/matching.h  $(SRC_DIR)/datalog.h 


$(OUT_DIR)%.o: $(SRC_DIR)%.cpp
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean


#matching: src/template.cpp  src/template.h
#	$(CC) $(CFLAGS) src/matching.cpp 

#all: foil clause_template matching
all: foil

clean:
	rm $(OBJ)
