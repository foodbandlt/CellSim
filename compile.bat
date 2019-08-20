:g++ -Wall -std=c++11 -o main.exe main.cpp -lmingw32 -lSDL2 -lSDL2_Image
g++ -ISDL2-2.0.5\x86_64-w64-mingw32\include -LSDL2-2.0.5\x86_64-w64-mingw32\lib -ISDL2_image-2.0.1\x86_64-w64-mingw32\include -LSDL2_image-2.0.1\x86_64-w64-mingw32\lib -ISDL2-2.0.5\x86_64-w64-mingw32\include\SDL2 -Wall -std=c++11 -Wl,-subsystem,windows -o main.exe main.cpp -lmingw32 -lSDL2 -lSDL2_Image
:-Wl,-subsystem,windows
pause