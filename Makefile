CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11 -MMD -MP $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_ttf -lm
TARGET  = gravitySimulator
 
SRC     = src/gravitySimulator.c src/vec.c src/linearBH.c src/render.c src/input.c src/setup.c src/camera.c src/hud.c
OBJ     = $(SRC:.c=.o)
DEPS    = $(OBJ:.o=.d)
 
FILE ?= setups/solar_system.txt

all: $(TARGET)
 
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)
 
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

run: $(TARGET)
	./$(TARGET) $(FILE)
 
clean:
	rm -f $(TARGET) $(OBJ) $(DEPS)
 
.PHONY: all run clean
