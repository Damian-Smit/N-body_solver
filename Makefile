CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11 -MMD -MP $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -lm
TARGET  = gravitySimulator
 
SRC     = src/gravitySimulator.c src/vec.c src/linearBH.c src/render.c src/input.c src/setup.c
OBJ     = $(SRC:.c=.o)
DEPS    = $(OBJ:.o=.d)
 
FILE ?= setups/solar_system.txt
##FILE = setups/base.txt
#FILE = setups/planetary_system_3000.txt
##FILE = setups/uniform_planets_3000.txt

all: $(TARGET)
 
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)
 
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEP)

run: $(TARGET)
	./$(TARGET) $(FILE)
 
clean:
	rm -f $(TARGET) $(OBJ) $(DEP)
 
.PHONY: all run clean
