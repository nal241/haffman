```
gcc -Wall -Iinclude   -c -o src/encode.o src/encode.c
gcc -Wall -Iinclude   -c -o src/main.o src/main.c
gcc -o bin/huffman src/encode.o src/main.o 
```