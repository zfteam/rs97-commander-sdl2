CC=g++
target = DinguxCommander

RESDIR:=res

SRCS=$(wildcard ./*.cpp)
OBJS=$(patsubst %cpp,%o,$(SRCS))

#INCLUDE = -I/usr/include/SDL2
INCLUDE =  $(shell sdl2-config --cflags)
#LIB = -L/usr/lib -lSDL2 -lSDL2_image -lSDL2_ttf 
#LIB = -lSDL2 -lSDL2_image -lSDL2_ttf 
LIB = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_gfx

all:$(OBJS)
	$(CC) $(OBJS) -o $(target) $(LIB)

%.o:%.cpp
	$(CC) -DRESDIR="\"$(RESDIR)\"" -DODROID_GO_ADVANCE  -c $< -o $@  $(INCLUDE) 

clean:
	rm $(OBJS) $(target) -f

