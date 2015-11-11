all:

	gcc -shared -c -fPIC sender.c -o function1.o

	gcc -shared -Wl,-soname,library.so -o library.so function1.o

	gcc -shared -c -fPIC sender1.c -o function.o

	gcc -shared -Wl,-soname,library.so -o library1.so function.o
