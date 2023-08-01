.PHONY = all main clean
DEBUG ?= 0

ifeq ($(DEBUG), 1)
	DebugFlag = -g -Og
else
	DebugFlag = -Wl,-subsystem,windows
endif

all: main

main:
	g++ -ISDL2-2.28.1\x86_64-w64-mingw32\include\SDL2 -LSDL2-2.28.1\x86_64-w64-mingw32\lib \
	-ISDL2_image-2.6.3\x86_64-w64-mingw32\include\SDL2 -LSDL2_image-2.6.3\x86_64-w64-mingw32\lib \
	-ISDL2_ttf-2.20.2\x86_64-w64-mingw32\include\SDL2 -LSDL2_ttf-2.20.2\x86_64-w64-mingw32\lib \
	-Wall -std=c++11 -o main.exe main.cpp \
	-lmingw32 -lSDL2 -lSDL2_Image -lSDL2_ttf $(DebugFlag)


run: main
	./main.exe
	

clean:
	del $(wildcard *.exe)