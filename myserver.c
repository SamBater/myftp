#include <sys/types.h>
#include <pwd.h>
#include <shadow.h>
#include <crypt.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 8080
#define SA struct sockaddr

#include "shaderd_function.c"

static int client_count_so_far = 0;
trans_mode mode = binary;

/// @return 0 - password is correct, otherwise no need root permision
int CheckPassword(const char *user, const char *password)
{
	struct passwd *passwdEntry = getpwnam(user);
	if (!passwdEntry)
	{
		printf("User '%s' doesn't exist\n", user);
		return 1;
	}

	if (0 != strcmp(passwdEntry->pw_passwd, "x"))
	{
		return strcmp(passwdEntry->pw_passwd, crypt(password, passwdEntry->pw_passwd));
	}
	else
	{
		// password is in shadow file
		struct spwd *shadowEntry = getspnam(user);
		if (!shadowEntry)
		{
			printf("Failed to read shadow entry for user '%s'\n", user);
			return 1;
		}

		return strcmp(shadowEntry->sp_pwdp, crypt(password, shadowEntry->sp_pwdp));
	}
}

void reaction(int socofd, char *buff)
{
	char cmd[MAX];
	char parm[MAX];
	char stat;
	bzero(parm,MAX);
	bzero(cmd,MAX);
	int n = sscanf(buff,"%s %s",cmd,parm);
	if(n <= 0) strcpy(buff,"no such cmd.");
	// 创建/删除目录（lmkdir/lrmdir）、
	if (strncmp(cmd, "lmdir", sizeof(buff)) == 0)
	{
		//printf
	}
   	

	else if (strcmp(buff, "lrmdir") == 0)
	{

	}

	//显示当前路径（lpwd)
	else if (strcmp(buff, "lpwd") == 0)
	{
		if(getcwd(buff,MAX)!=NULL)
		{

		}
		else
		{
			strncpy(buff,"no such direction.",MAX);
		}
		
		send(socofd, buff, MAX, 0);
	}
	//   切换目录（lcd）、
	else if (strcmp(cmd, "lcd") == 0)
	{
		//strncpy(buff,"TODO:lcd",MAX);
		if(chdir(parm) == 0)
		{
			bzero(buff,MAX);
		}
		else
		{
			strncpy(buff,"no such direction",MAX);
			send(socofd, buff, MAX, 0);
		}
	}
	//   查看当前目录下的所有文件（dir）、
	else if (strcmp(cmd, "dir") == 0)
	{
		struct dirent *myfile;
		DIR * mydir = opendir(".");
		while((myfile = readdir(mydir)) != NULL)
		{
			char buf[255];
			bzero(buf,255);
			sprintf(buf,"%s",myfile->d_name);
			send(socofd,buf,MAX,0);
			puts(buf);
		}
	}
	// 	   上传单个/多个文件（put/mput）、

	else if (strcmp(cmd, "put") == 0)
	{
		char fileName[MAX];
		sprintf(fileName,"(put)%s",parm);
		receive_file(socofd,buff,fileName);
	}

	else if (strcmp(cmd, "mput") == 0)
	{
		char tmp[MAX];
		strcpy(tmp,buff);
		char *token = strtok(tmp," ");
		while(token != NULL)
		{
			token = strtok(NULL," ");
			char fileName[MAX];
			sprintf(fileName,"(mput)%s",token);
			if(token)
			{
				if(mode == binary)
					recive_binaryFile(socofd,fileName);
				else
					receive_file(socofd,buff,fileName);
			}
		}
	}

	//    下载单个/多个文件（get/mget）。
	else if (strcmp(cmd, "get") == 0)
	{
		if(mode == binary)
			send_binaryfile(socofd,buff,parm);
		else
			send_file(socofd,buff,parm);
	}
	else if (strcmp(cmd, "mget") == 0)
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
					send_binaryfile(socofd,buff,token);
				else
					send_file(socofd,buff,token);
			}
		}
	}
	else
	{
		strncpy(buff,"cmd doesn't exists.",MAX);
		send(socofd, buff, MAX, 0);
	}
	
}

int detectUser_Pwd(int sockfd)
{
	char buff[MAX];
	char user[MAX];
	char pwd[MAX];
	char stat;
	bzero(buff,MAX);
	bzero(user, MAX);
	bzero(pwd, MAX);
	recv(sockfd, buff, MAX, 0);
	int n = sscanf(buff, "%s %s", user, pwd);
	
	//参数检测
	if(n < 2)
	{
		client_usage();
		close(sockfd);
		stat = login_failed;
	}

	//验证密码
	if (CheckPassword(user, pwd) == 0)
	{
		stat = ok;
		sprintf(buff, "Welcome,you are the cilent #%d\n", ++client_count_so_far);
	}
	else
	{
		stat = login_failed;
		strncpy(buff, "username doesn't exist or password is error.\n", sizeof(buff));
	}
	//回显信息

	write(sockfd,&stat,1);
	send(sockfd, buff, sizeof(buff), 0);
	return stat;
}

// Function designed for chat between client and server.
void func(int sockfd)
{
	char buff[MAX];
	bzero(buff, MAX);
	while (1)
	{
		recv(sockfd, buff, MAX, 0);
		reaction(sockfd, buff);
		bzero(buff, sizeof(buff));
	}
}

int main()
{
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	// socket create and verification
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
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
	{
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0)
	{
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);

	while(1)
	{
		connfd = accept(sockfd, (SA *)&cli, &len);

		//如果coonfd 成功 并且 通过了密码检测，就开一个新子进程去处理.
		if(connfd > 0 && detectUser_Pwd(connfd) == ok)
		{
			if(fork() == 0)
			{
				printf("server acccept the client...\n");
				func(connfd);
				close(sockfd);
			}
		}
		else
			printf("server acccept failed...\n");
	}
}
