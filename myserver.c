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
#include <sys/prctl.h>
#include <signal.h>
#include <sys/mman.h>
#include "shaderd_function.c"
#define MAX 255
int PORT = 8080;
#define SA struct sockaddr

static int client_count_so_far = 0; //系统访客总数
static int client_current = 0;		//活动用户总数
trans_mode mode = binary;

void addUser(User *root, User *next)
{
	for (User *tmp = root; tmp; tmp = tmp->next)
	{
		if (tmp->next == NULL)
		{
			tmp->next = next;
			break;
		}
	}
}

void deleteUser(User *root, int sockfd)
{
	for (User *tmp = root; tmp; tmp = tmp->next)
	{
		User *next = tmp->next;
		if (next && next->sockfd == sockfd)
		{
			tmp->next = next->next;
			close(next->sockfd);
			free(next);
			break;
		}
	}
}

void printAllUser(User *root)
{
	for (User *tmp = root->next; tmp; tmp = tmp->next)
	{
		printf("%s\t", tmp->userName);
	}
	puts("");
}

void quit(User_p root)
{
	for (User_p tmp = root->next; tmp; tmp = tmp->next)
	{
		close(tmp->sockfd);
	}
	exit(0);
}

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

void reaction(User *user, char *recved_command)
{
	int sockfd = user->sockfd;
	char cmd[MAX];
	char parm[MAX];
	char stat;
	int n = sscanf(recved_command, "%s %s", cmd, parm);

	if(recved_command[0] == 'l')
	{
		//已在客户端处理
	}

	else if (strcmp(cmd, "binary") == 0)
	{
		mode = binary;
	}

	else if (strcmp(cmd, "ascii") == 0)
	{
		mode = ascii;
	}

	// 创建/删除目录（lmkdir/lrmdir）、
	else if (strncmp(cmd, "mkdir", 5) == 0)
	{
		mkdir(parm, 755);
	}

	else if (strcmp(cmd, "rmkdir") == 0)
	{
		char tmp[MAX];
		strcpy(tmp, recved_command);
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
	else if (strcmp(cmd, "pwd") == 0)
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
	else if (strcmp(cmd, "cd") == 0)
	{
		if (chdir(parm) == 0)
		{
			sprintf(recved_command, "change to %s", parm);
		}
		else
		{
			strncpy(recved_command, "no such direction", MAX);
		}
		//send(sockfd, recved_command, MAX, 0);
	}
	//   查看当前目录下的所有文件（dir）、
	else if (strcmp(cmd, "dir") == 0)
	{
		struct dirent *myfile;
		DIR *mydir = opendir(".");
		while((myfile = readdir(mydir)) != NULL)
		{
			char fileName[MAX];
			sprintf(fileName,"%s",myfile->d_name);
			send(sockfd,fileName,MAX,0);
		}
		char c = -100;
		send(sockfd,&c,1,0);
	}
	// 	   上传单个/多个文件（put/mput）、

	else if (strcmp(cmd, "put") == 0)
	{

		recv_bfile(sockfd, parm);
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

				recv_bfile(sockfd, token);
			}
		}
	}

	//    下载单个/多个文件（get/mget）。
	else if (strcmp(cmd, "get") == 0)
	{
		//发送

		send_bfile(sockfd, parm);
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

				send_bfile(sockfd, token);
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

	User *newUser = 0;
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
		printf("\naddUser\nuid = %d\tpid = %d\t name = %s\n", newUser->uid, newUser->gid, newUser->userName);
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
	setuid(user->uid);
	setgid(user->gid);
	int sockfd = user->sockfd;
	while (1)
	{
		recv(sockfd, buff, MAX, 0);
		if (strncmp(buff, "quit", 4) == 0)
			break;
		reaction(user, buff);
		bzero(buff, sizeof(buff));
	}
}
/*
count current 当前活动用户数 退出时减1
count all 显示系统访客总数 ==目前的so_far
list 显示当前在线的所有用户的用户名 连接后缓存下来 退出时删除
kill username 强制删除某个用户 删除用户命令
quit 关闭ftp
*/
void server_cmd(User *user_list)
{
	while (1)
	{
		char cmd[MAX];
		printf("> ");
		fgets(cmd,MAX,stdin);
		char c[MAX];
		char parm[MAX];
		sscanf(cmd, "%s %s", c, parm);
		if (strncmp(cmd, "count current",13) == 0)
		{
			printf("count current = %d \n", client_current);
		}
		else if (strncmp(cmd, "count all",9) == 0)
		{
			printf("count all = %d \n", client_count_so_far);
		}
		else if (strncmp(c, "list",4) == 0)
		{
			printAllUser(user_list);
		}
		else if (strcmp(c, "kill") == 0)
		{
			// TODO : 删除用户
			// for(User* tmp = user_list;tmp;tmp = tmp->next)
			// {
			// 	User* next = tmp->next;
			// 	if(next && strcmp(parm,next->userName) == 0)
			// 	{
			// 		tmp->next = next->next;
			// 		close(next->sockfd);
			// 		free(next);
			// 		break;
			// 	}
			// }
		}
		else if (strcmp(c, "quit") == 0)
		{
			exit(0);
			quit(user_list);
		}
	}
}

int main(int args, char **argv)
{
	if (args > 1)
		PORT = atoi(argv[1]);
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	User *user_list = (User_p)malloc(sizeof(User));

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
	pthread_create(&server, NULL, server_cmd, user_list);

	while (1)
	{
		connfd = accept(sockfd, (SA *)&cli, &len);
		printf("parent uid = %d \n", getuid());
		User *newUser = NULL;
		if (connfd > 0 && (newUser = detectUser_Pwd(connfd)))
		{
			addUser(user_list, newUser);
			int child_quit = 0;
			if (fork() == 0)
			{
				prctl(PR_SET_PDEATHSIG, SIGKILL);
				func(newUser);
				deleteUser(user_list, sockfd);
				close(sockfd);
				child_quit = 1;
			}
			if (child_quit)
			{
				client_current--;
				exit(0);
			}
		}
		else
			printf("server acccept failed...\n");
	}
}
