#include "shaderd_function.c"
#include <sys/socket.h>
#include <error.h>

#define PORT 8080
#define SA struct sockaddr

trans_mode mode = binary;
//将远程用户、服务器信息读入
login_info *check_cmd(char *argv)
{
	const char *token = ":";
	const char *token2 = "@";

	char *user = strtok(argv, token);
	char *pwd = strtok(NULL, token2);
	char *ip = strtok(NULL, token);
	char *port = strtok(NULL, token);
	if (user != NULL && pwd != NULL && ip != NULL && port != NULL)
	{
		login_info *cmd = (login_info *)malloc(sizeof(login_info));
		cmd->ip = ip;
		cmd->port = port;
		cmd->user = user;
		cmd->pwd = pwd;
		return cmd;
	}
	printf("useage:ftpclient <user>:<passwd>@<host>:<port>\n");
	return NULL;
}

int login(int sockfd,login_info* info)
{
	char buff[MAX];
	bzero(buff, MAX);
	sprintf(buff, "%s %s", info->user, info->pwd);
	//写入用户、明码 然后等待显示服务器响应.
	send(sockfd, buff, sizeof(buff), 0);
	bzero(buff,MAX);
	char stat;
	read(sockfd,&stat,1);
	recv(sockfd, buff, MAX, 0);
	puts(buff);
	return stat;
	//TODO:错误时退出.
}

void ftp_cmd(int sockfd, char* buff)
{
	char stat;
	char cmd[MAX];
	char parm[MAX];
	bzero(cmd,MAX);
	bzero(parm,MAX);
	sscanf(buff,"%s %s",cmd,parm);

	puts("this is ok");
	if(strcmp(cmd,"binary"))
	{
		mode = binary;
	}

	else if(strcmp(cmd,"ascii"))
	{
		mode = ascii;
	}

	else if(strncmp(cmd,"lmdir",5) == 0)
	{
		//在main loop中已发送
	}

	else if(strncmp(cmd,"lrmdir",6) == 0)
	{
		//在Main loop中已发送.
	}

	else if(strncmp(cmd,"dir",3) == 0)
	{
		while (1)
		{
			recv(sockfd,buff,MAX,0);
			if(buff[0]!=-100)
			{
				printf("%s\t",buff);	
			}
			else
				break;
		}
	}

	else if(strncmp(buff,"get",3) == 0)
	{
		char target[MAX];
		sprintf(target,"(getb)%s",parm);
		
		if(mode == binary)
			recive_binaryFile(sockfd,target);
		else
			receive_file(sockfd,buff,target);
	}

	else if(strncmp(buff,"mget",4) == 0)
	{
		char tmp[MAX];
		strcpy(tmp,buff);
		char * token = strtok(tmp," ");
		//token 为parms
		while(token !=NULL)
		{
			token = strtok(NULL," ");
			if(token) 
			{
				char target[MAX];
				sprintf(target,"(mgetb)%s",token);

				if(mode == binary)
					recive_binaryFile(sockfd,target);
				else
					receive_file(sockfd,buff,target);
			}
		}
	}

	else if(strncmp(buff,"put",3) == 0)
	{
		if(mode == binary)
			send_binaryfile(sockfd,buff,parm);
		else
			send_file(sockfd,buff,parm);
	}
	else if(strncmp(buff,"mput",4) == 0)
	{
		char tmp[MAX];
		strcpy(tmp,buff);
		char * token = strtok(tmp," ");
		//token 为parms
		while(token !=NULL)
		{
			token = strtok(NULL," ");
			if(token) 
			{
				if(mode == binary)
					send_binaryfile(sockfd,buff,token);
				else
					send_file(sockfd,buff,token);
			}
		}
	}
	else
	{
		recv(sockfd,buff,MAX,0);
		puts(buff);
	}
}

void func(int sockfd)
{
	char buff[MAX];
	bzero(buff, MAX);
	recv(sockfd,buff,MAX,0);
	puts(buff);
	while (1)
	{
		putchar('>');
		gets(buff);
		int n = send(sockfd, buff, MAX, 0);

		if(strncmp(buff,"quit",4) == 0) break;
		
		ftp_cmd(sockfd, buff);
		bzero(buff, sizeof(buff));
	}
}

int main(int argc, char **argv)
{
	login_info *info = NULL;
	if (argc != 2)
	{
		printf("useage:ftpclient <user>:<passwd>@<host>:<port>\n");
		exit(0);
	}

	if ((info = check_cmd(argv[1])) == NULL)
	{
		printf("useage:ftpclient <user>:<passwd>@<host>:<port>\n");
		exit(0);
	}

	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	// socket create and varification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
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
	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		printf("connection with the server failed...\n");
		printf("%s\n", strerror(errno));
		exit(0);
	}
	else
		printf("connected to the server..\n");

	
	if(login(sockfd,info) == ok)
		func(sockfd);

	// close the socket
	close(sockfd);
}
