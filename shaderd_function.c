#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#define MAX 255
#define SIZE 1500
typedef struct
{
  char *user;
  char *pwd;
  char *ip;
  char *port;
} login_info;

typedef enum
{
  ok,
  notexists,
  login_failed
} fileInformation;

void client_usage()
{
  printf("useage:ftpclient <user>:<passwd>@<host>:<port>\n");
}

void send_file(int sockfd, char *buff, char *fileName)
{
  FILE *f = fopen(fileName, "r+");
  bzero(buff, MAX);
  while (fgets(buff, MAX, f) != NULL)
  {
    send(sockfd, buff, MAX, 0);
    bzero(buff, MAX);
  }
  //添加EOF 作为接受完的标志.
  char c = EOF;
  send(sockfd, &c, 1, 0);
  fclose(f);

  puts("send done");

  //等待对方发送完成信息
  recv(sockfd, buff, MAX, 0);
  puts(buff);
}

void receive_file(int sockfd, char *buff, char *fileName)
{
  FILE *f = fopen(fileName, "w+");
  while (1)
  {
    int n = recv(sockfd, buff, MAX, 0);
    printf("recv %d bytes.\n", n);
    if (n <= 0 || buff[n - 1] == EOF)
      break;
    fprintf(f, "%s", buff);
    bzero(buff, MAX);
  }
  fclose(f);
  puts("get done");
  strcpy(buff,"put complete");
  send(sockfd,buff,MAX,0);
}
