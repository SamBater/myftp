#include "shaderd_function.c"

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
		return cmd;
	}
	printf("connecction error!\n");
	return NULL;
}

int main(int argc,char* argv[])
{
	struct sockaddr_in server_sockaddr;
	server_sockaddr.sin_family = AF_INET;

	if(argc == 2)
	{
		login_info* info = check_cmd(argv[1]);
		if(info != NULL)
		{
			server_sockaddr.sin_addr.s_addr = inet_addr(info->ip);
			server_sockaddr.sin_port = atoi(info->port);
		}
		{
			exit(0);
		}
	}

	int server_sockfd;
	if(server_sockfd =  socket(AF_INET,SOCK_STREAM,0) == -1)
	{
		fprintf(stderr,"Socket error:%s\n\a", strerror(errno));
		exit(1);
	}
	bzero(&server_sockaddr,sizeof(struct sockaddr_in));
}

