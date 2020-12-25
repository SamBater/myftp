#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#define MAX 255
#define SIZE 1500
typedef struct
{
  char *user;
  char *pwd;
  char *ip;
  char *port;
} login_info;

struct user;

typedef struct user *User_p;

typedef struct user
{
  int sockfd;
  char *userName;
  int uid;
  int gid;
  User_p next;
} User;

typedef enum
{
  ok,
  notexists,
  login_failed
} fileInformation;

typedef enum
{
  binary,
  ascii
} trans_mode;

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
  strcpy(buff, "put complete");
  send(sockfd, buff, MAX, 0);
}

size_t min(size_t a, size_t b)
{
  return a > b ? b : a;
}

void send_binaryfile(int sockfd, char *buff, char *fileName)
{
  FILE *f = fopen(fileName, "rb");

  char stat = 1;
  if (f == NULL)
  {
    stat = 0;
    send(sockfd, &stat, 1, 0);
    puts("No such file or permission dined.");
    return ;
  }
  send(sockfd, &stat, 1, 0);


  long long passed_bytes = 0;
  recv(sockfd, &passed_bytes, sizeof(long long), 0);

  fseek(f, 0, SEEK_END);
  long long fileSize = ftell(f) - passed_bytes;
  const long long remain_bytes = fileSize;
  fseek(f, passed_bytes, SEEK_SET);
  if (fileSize == EOF)
    return 0;

  char size[32];
  sprintf(size, "%lld", remain_bytes);
  //事先沟通大小.
  send(sockfd, size, 31, 0);

  long long c = 0;
  if (fileSize > 0)
  {
    char buffer[SIZE];
    do
    {
      size_t num = min(fileSize, sizeof(buffer));
      fread(buffer, num, 1, f);
      send(sockfd, buffer, num, 0);
      fileSize -= num;
      c += num;
      bzero(buffer, sizeof(buff));
      printf("\rprocess: (%lld/%lld)", c, remain_bytes);
    } while (fileSize > 0);
  }
  printf("\nsend total %lld bytes.\n", c);
  fclose(f);
  return 1;
}

void recive_binaryFile(int sockfd, char *fileName)
{
  char st = 1;
  recv(sockfd, &st, 1, 0);
  if (st == 0)
  {
    puts("No such file or permission dined.");
    return;
  }

  FILE *f = fopen(fileName, "ab+");

  //告知已经下载的bytes.
  fseek(f, 0, SEEK_END);
  long long passed_bytes = ftell(f);
  send(sockfd, &passed_bytes, sizeof(long long), 0);

  struct stat s;
  char buff[SIZE];
  char size[32];
  recv(sockfd, size, 31, 0);
  long long fileSize = atoll(size);
  const long long fs = fileSize;
  long long c = 0;
  do
  {
    if (fileSize <= 0)
      break;
    size_t num = recv(sockfd, buff, min(fileSize, sizeof(buff)), 0);
    fwrite(buff, num, 1, f);
    fileSize -= num;
    c += num;
    printf("\rprocess : (%lld/%lld)", c, fs);
  } while (fileSize > 0);
  printf("\nrecv total %lld bytes.\n", c);
  fclose(f);
}