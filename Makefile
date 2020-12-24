user = sam
passwd = w1ww11w
ip = 0.0.0.0
port = 8080
server : myserver shaderd_function.c
	sudo ./myserver $(port) ;

client :myftp shaderd_function.c
	./myftp $(user):$(passwd)@$(ip):$(port)

clean:
	sudo rm myserver myftp

myserver:myserver.c shaderd_function.c
	gcc myserver.c -g -o myserver -lpthread -lcrypt;

myftp:myftp.c shaderd_function.c
	gcc myftp.c -o myftp

