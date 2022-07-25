#References
https://stackoverflow.com/questions/51870548/c-how-to-erase-from-vector-while-iterating

#Compile for windows
## Reference
https://stackoverflow.com/questions/23397536/how-to-compile-sfml-with-mingw
> g++ -c main.cpp -I SFML-2.5.1\64\include -DSFML_STATIC

> g++ main.o -o main -LSFML-2.5.1\64\lib -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lwinmm -lgdi32 -lfreetype --static

