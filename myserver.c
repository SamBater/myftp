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
#define MAX 255
int PORT = 8080;
#define SA struct sockaddr

#include "shaderd_function.c"

static int client_count_so_far = 0; //系统访客总数
static int client_current = 0;		//活动用户总数
trans_mode mode = binary;
User* user_list;

void addUser(User* root,User* next)
{
  User* tmp = root;
  while(tmp)
  {
    if(tmp->next == 0)
    {
      tmp->next = next;
      break;
    }
    tmp = tmp->next;
  }
}

void deleteUser(User* root,int sockfd)
{
  for(User* tmp=root;tmp;tmp=tmp->next)
  {
    User *next = tmp->next;
    if(next && next->sockfd == sockfd)
    {
      tmp->next = next->next;
	  close(next->sockfd);
      free(next);
      break;
    }
  }
}

void printAllUser(User* root)
{
  for(User* tmp = root->next;tmp;tmp=tmp->next)
  {
    printf("%s\t",tmp->userName);
  }
  puts("");
}

void quit(User_p root)
{
  for(User_p tmp=root->next;tmp;tmp=tmp->next)
  {
    close(tmp->sockfd);
  }
  exit(0);
}

/// @return 0 - password is correct, otherwise no need root permision
int CheckPassword(const char *user, const char *password)
{
	return 0;
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

void reaction(User *user, char *recved_command)
{
	int sockfd = user->sockfd;
	char cmd[MAX];
	char parm[MAX];
	char stat;
	int n = sscanf(recved_command, "%s %s", cmd, parm);

	if (strcmp(cmd, "binary"))
	{
		mode = binary;
	}

	else if (strcmp(cmd, "ascii"))
	{
		mode = ascii;
	}

	// 创建/删除目录（lmkdir/lrmdir）、
	if (strncmp(cmd, "lmdir", 5) == 0)
	{
		mkdir(parm, 777);
	}

	else if (strcmp(cmd, "lrmdir") == 0)
	{
		char tmp[MAX];
		strcpy(tmp, recved_command);
		char *token = strtok(tmp, " ");
		while (token != NULL)
		{
			token = strtok(NULL, " ");
			if (token)
			{
				mkdir(token, 777);
			}
		}
	}

	//显示当前路径（lpwd)
	else if (strcmp(cmd, "lpwd") == 0)
	{
		if (getcwd(recved_command, MAX) != NULL)
		{

		}
		else
		{
			strncpy(recved_command, "no such direction.", MAX);
		}

		send(sockfd, recved_command, MAX, 0);
	}
	//   切换目录（lcd）、
	else if (strcmp(cmd, "lcd") == 0)
	{
		if (chdir(parm) == 0)
		{
			sprintf(recved_command,"change to %s",parm);
		}
		else
		{
			strncpy(recved_command, "no such direction", MAX);
		}
		send(sockfd, recved_command, MAX, 0);
	}
	//   查看当前目录下的所有文件（dir）、
	else if (strcmp(cmd, "dir") == 0)
	{
		struct dirent *myfile;
		DIR *mydir = opendir(".");
		while ((myfile = readdir(mydir)) != NULL)
		{
			char buf[255];
			sprintf(buf, "%s", myfile->d_name);
			send(sockfd, buf, MAX, 0);
			puts(buf);
		}
		char c = -100;
		send(sockfd, &c, 1, 0);
	}
	// 	   上传单个/多个文件（put/mput）、

	else if (strcmp(cmd, "put") == 0)
	{
		char stat;
		recv(sockfd, &stat, 1, 0);
		if (stat < 0)
		{
			return;
		}
		char fileName[MAX];
		sprintf(fileName, "%s", parm);
		recive_binaryFile(sockfd, fileName);
	}

	else if (strcmp(cmd, "mput") == 0)
	{
		char tmp[MAX];
		strcpy(tmp, recved_command);
		char *token = strtok(tmp, " ");
		while (token != NULL)
		{
			token = strtok(NULL, " ");
			if (token)
			{
				char fileName[MAX];
				sprintf(fileName, "%s", token);
				char stat;
				recv(sockfd, &stat, 1, 0);
				if (stat < 0)
				{
					return;
				}
				if (mode == binary)
					recive_binaryFile(sockfd, fileName);
				else
					receive_file(sockfd, recved_command, fileName);
			}
		}
	}

	//    下载单个/多个文件（get/mget）。
	else if (strcmp(cmd, "get") == 0)
	{
		//检测权限
		mode_t v = vaild_acess(parm, user->uid, user->gid);
		v = readAble(v);
		if (v <= 0)
		{
			char stat = -100;
			send(sockfd, &stat, 1, 0);
			return;
		}
		else
		{
			char stat = 100;
			send(sockfd, &stat, 1, 0);
		}

		//发送
		if (mode == binary)
			send_binaryfile(sockfd, recved_command, parm);
		else
			send_file(sockfd, recved_command, parm);
	}
	else if (strcmp(cmd, "mget") == 0)
	{
		char tmp[MAX];
		strcpy(tmp, recved_command);
		char *token = strtok(tmp, " ");
		//token 为parms
		while (token != NULL)
		{
			token = strtok(NULL, " ");
			if (token)
			{
				mode_t v = vaild_acess(token, user->uid, user->gid);
				v = readAble(v);
				if (v <= 0)
				{
					char stat = -100;
					send(sockfd, &stat, 1, 0);
					return;
				}
				else
				{
					char stat = 100;
					send(sockfd, &stat, 1, 0);
				}
				if (mode == binary)
					send_binaryfile(sockfd, recved_command, token);
				else
					send_file(sockfd, recved_command, token);
			}
		}
	}
	else
	{
		strncpy(recved_command, "cmd doesn't exists.", MAX);
		send(sockfd, recved_command, MAX, 0);
	}
}

User *detectUser_Pwd(int sockfd)
{
	char buff[MAX];
	char user[MAX];
	char pwd[MAX];
	char stat;
	recv(sockfd, buff, MAX, 0);
	int n = sscanf(buff, "%s %s", user, pwd);

	User* newUser = 0;
	//参数检测
	if (n < 2)
	{
		client_usage();
		close(sockfd);
		stat = login_failed;
	}

	//验证密码
	else if (CheckPassword(user, pwd) == 0)
	{
		stat = ok;
		sprintf(buff, "Welcome,you are the cilent %d\n", ++client_count_so_far);
		++client_current;

		//加入到用户列表.
		newUser = (User *)malloc(sizeof(User));
		newUser->sockfd = sockfd;
		newUser->userName = user;
		newUser->uid = getpwnam(user)->pw_uid;
		newUser->gid = getpwnam(user)->pw_gid;
		printf("addUser\nuid = %d\tpid = %d\t name = %s\n", newUser->uid, newUser->gid, newUser->userName);
		addUser(user_list, newUser);
	}
	else
	{
		stat = login_failed;
		strncpy(buff, "username doesn't exist or password is error.\n", sizeof(buff));
	}
	//回显信息

	write(sockfd, &stat, 1);
	send(sockfd, buff, strlen(buff), 0);
	return newUser;
}

// Function designed for chat between client and server.
void func(User *user)
{
	char buff[MAX];
	bzero(buff, MAX);

	int sockfd = user->sockfd;
	while (1)
	{
		recv(sockfd, buff, MAX, 0);
		if (strncmp(buff, "quit", 4) == 0)
			break;
		reaction(user, buff);
		bzero(buff, sizeof(buff));
	}
	deleteUser(user_list, sockfd);
	close(sockfd);
	client_current--;
}
/*
count current 当前活动用户数 退出时减1
count all 显示系统访客总数 ==目前的so_far
list 显示当前在线的所有用户的用户名 连接后缓存下来 退出时删除
kill username 强制删除某个用户 删除用户命令
quit 关闭ftp
*/
void server_cmd()
{
	while (1)
	{
		char cmd[MAX];
		printf("> ");
		//fgets(cmd,MAX,stdin);
		gets(cmd);
		char c[MAX];
		char parm[MAX];
		sscanf(cmd,"%s %s",c,parm);
		if (strcmp(cmd, "count current") == 0)
		{
			printf("count current = %d \n", client_current);
		}
		else if (strcmp(cmd, "count all") == 0)
		{
			printf("count all = %d \n", client_count_so_far);
		}
		else if (strcmp(cmd, "list") == 0)
		{
			printAllUser(user_list);
		}
		else if (strcmp(c, "kill") == 0)
		{
			for(User* tmp = user_list;tmp;tmp = tmp->next)
			{
				if(tmp->next && strcmp(parm,tmp->next->userName) == 0)
				{
					close(tmp->next->sockfd);
					tmp->next = tmp->next->next;
					free(tmp->next);
					return;
				}
			}
		}
		else if (strcmp(cmd, "quit") == 0)
		{
			quit(user_list);
		}
	}
}

int main(int args,char** argv)
{
	if(args > 1) PORT = atoi(argv[1]);
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	user_list = (User_p)malloc(sizeof(User));

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

	pthread_t server;
	pthread_create(&server, NULL, server_cmd, NULL);

	while (1)
	{
		connfd = accept(sockfd, (SA *)&cli, &len);
		User *user = NULL;
		if (connfd > 0 && (user = detectUser_Pwd(connfd)))
		{
			pthread_t pid;
			pthread_create(&pid, NULL, func, (void *)user);
			pthread_detach(pid);
		}
		else
			printf("server acccept failed...\n");
	}
}
