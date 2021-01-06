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

int login(int sockfd, login_info *info)
{
	char buff[MAX];
	bzero(buff, MAX);
	sprintf(buff, "%s %s", info->user, info->pwd);
	//写入用户、明码 然后等待显示服务器响应.
	send(sockfd, buff, sizeof(buff), 0);
	bzero(buff, MAX);
	char stat;
	read(sockfd, &stat, 1);
	recv(sockfd, buff, MAX, 0);
	puts(buff);
	return stat;
	//TODO:错误时退出.
}

void ftp_cmd(int sockfd, char *buff)
{
	char stat;
	char cmd[MAX];
	char parm[MAX];
	sscanf(buff, "%s %s", cmd, parm);
	if (strncmp(cmd, "binary",6) == 0)
	{
		mode = binary;
	}

	else if (strncmp(cmd, "ascii",5) == 0)
	{
		mode = ascii;
	}

	else if(strcmp(cmd,"mdir") == 0 || strcmp(cmd,"rmdir") == 0 ||  strcmp(cmd,"cd") ==0 )
	{
		//无需回显信息
	}

	else if(strcmp(cmd,"pwd") ==0)
	{
		recv(sockfd,buff,MAX,0);
		puts(buff);
	}

// 创建/删除目录（lmkdir/lrmdir）、
	else if (strncmp(cmd, "lmdir", 5) == 0)
	{
		mkdir(parm, 755);
	}

	else if (strcmp(cmd, "lrmdir") == 0)
	{
		char tmp[MAX];
		strcpy(tmp, cmd);
		char *token = strtok(tmp, " ");
		while (token != NULL)
		{
			token = strtok(NULL, " ");
			if (token)
			{
				mkdir(token, 755);
			}
		}
	}

	//显示当前路径（lpwd)
	else if (strncmp(buff, "lpwd",4) == 0)
	{
		if (getcwd(cmd, MAX) != NULL)
		{
			
		}
		else
		{
			strncpy(cmd, "no such directory.", MAX);
		}
		puts(cmd);
	}
	//   切换目录（lcd）、
	else if (strcmp(cmd, "lcd") == 0)
	{
		if (chdir(parm) == 0)
		{
			sprintf(cmd, "change to %s", parm);
		}
		else
		{
			strncpy(cmd, "no such direction", MAX);
		}
		puts(cmd);
	}

	else if (strncmp(cmd, "dir",3) == 0)
	{
		while (1)
		{
			bzero(buff, MAX);
			int n = recv(sockfd, buff, MAX, 0);
			if (buff[0] != -100 || buff[n - 1] != -100)
			{
				printf("%s\t", buff);
			}
			else
				break;
		}
		puts("");
	}

	else if (strncmp(buff, "get", 3) == 0)
	{

		recv_bfile(sockfd, parm);
	}

	else if (strncmp(buff, "mget", 4) == 0)
	{
		char tmp[MAX];
		strcpy(tmp, buff);
		char *token = strtok(tmp, " ");
		//token 为parms
		while (token != NULL)
		{
			token = strtok(NULL, " ");
			if (token)
			{

				recv_bfile(sockfd, token);
			}
		}
	}

	else if (strncmp(buff, "put", 3) == 0)
	{

		send_bfile(sockfd, parm);
	}
	else if (strncmp(buff, "mput", 4) == 0)
	{
		char tmp[MAX];
		strcpy(tmp, buff);
		char *token = strtok(tmp, " ");
		//token 为parms
		while (token != NULL)
		{
			token = strtok(NULL, " ");
			if (token)
			{
				send_bfile(sockfd, token);
			}
		}
	}
	else
	{
		recv(sockfd, buff, MAX, 0);
		puts(buff);
	}
}

void func(int sockfd)
{
	char buff[MAX];
	bzero(buff, MAX);
	while (1)
	{
		putchar('>');
		fgets(buff, MAX, stdin);
		int n = send(sockfd, buff, MAX, 0);
		if (strncmp(buff, "quit", 4) == 0 || n <= 0)
			break;

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
	servaddr.sin_port = htons(atoi(info->port));
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

	if (login(sockfd, info) == ok)
		func(sockfd);

	// close the socket
	close(sockfd);
}
