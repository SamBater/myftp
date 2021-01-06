user = sam
passwd = w1ww11w
ip = 127.0.0.1
port = 8080
server = ./ftpserver/myserver.c
client = ./ftpclient/myftp.c

all:myserver myftp

server : myserver shaderd_function.c
	sudo ./myserver $(port) ;

client :myftp shaderd_function.c
	./myftp $(user):$(passwd)@$(ip):$(port)

clean:
	sudo rm myserver myftp

myserver:$(server) shaderd_function.c
	gcc $(server) -g -o myserver -lpthread -lcrypt;

myftp:$(client) shaderd_function.c
	gcc $(client) -o myftp

