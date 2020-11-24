#include "shaderd_function.c"
#include <sys/socket.h> 
#include <error.h>
#define MAX 80 
#define PORT 8080
#define SA struct sockaddr

//将远程用户、服务器信息读入
login_info* check_cmd(char*argv)
{
	const char* token = ":";
	const char* token2 = "@";

	char * user = strtok(argv,token);
	char * pwd = strtok(NULL,token2);
	char * ip = strtok(NULL,token);
	char * port = strtok(NULL,token);
	if(user!=NULL && pwd != NULL && ip !=NULL && port != NULL)
	{
		login_info* cmd = (login_info*)malloc(sizeof(login_info));
		cmd->ip = ip;
		cmd->port = port;
		cmd->user = user;
		cmd->pwd = pwd;
		printf("cmd->pwd = %s\n",pwd);
		return cmd;
	}
	printf("connecction error!\n");
	return NULL;
}

 
void func(int sockfd,login_info* info) 
{ 
    char buff[MAX]; 
	bzero(buff,sizeof(buff));
	write(sockfd,info->user,sizeof(info->user));
	write(sockfd,info->pwd,sizeof(info->pwd));
	while(read(sockfd,buff,sizeof(buff))!=0)
	{
		puts(buff);
	}

} 

int main(int argc,char** argv) 
{ 
	login_info* info = NULL;
	if(argc != 2)
	{
		printf("useage:ftpclient <user>:<passwd>@<host>:<port>\n");
		exit(0);
	}

	if((info = check_cmd(argv[1])) == NULL)
	{
		printf("useage:ftpclient <user>:<passwd>@<host>:<port>\n");
		exit(0);
	}

	int sockfd, connfd; 
	struct sockaddr_in servaddr, cli; 

	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr(info->ip); 
	servaddr.sin_port = htons(8080);

	extern int erron;
	errno = 0;
	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) 
	{ 
		printf("connection with the server failed...\n"); 
		printf("%s\n",strerror(errno));
		exit(0); 
	} 
	else
		printf("connected to the server..\n"); 

	// function for chat 
	func(sockfd,info); 

	// close the socket 
	close(sockfd); 
} 

