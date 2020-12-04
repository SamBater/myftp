run : myserver myftp shaderd_function.c
	./myftp root:passwd@0.0.0.0:0000 ;
clean:
	sudo rm myserver myftp
myserver:myserver.c shaderd_function.c
	gcc myserver.c -o myserver -lpthread -lcrypt;

myftp:myftp.c shaderd_function.c
	gcc myftp.c -o myftp