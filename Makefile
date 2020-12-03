run : myserver myftp shaderd_function.c
	./myftp root:w1ww11w123@0:0XBEAF ;
clean:
	sudo rm myserver myftp
myserver:myserver.c shaderd_function.c
	gcc myserver.c -o myserver -lpthread -lcrypt;

myftp:myftp.c shaderd_function.c
	gcc myftp.c -o myftp