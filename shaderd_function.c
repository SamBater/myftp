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

size_t min(size_t a, size_t b)
{
  return a > b ? b : a;
}

void client_usage()
{
  printf("useage:ftpclient <user>:<passwd>@<host>:<port>\n");
}

void send_bfile(int sockfd, char *fileName)
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
  size_t fileSize = ftell(f) - passed_bytes;
  size_t remain_bytes = fileSize;
  fseek(f, passed_bytes, SEEK_SET);
  if (fileSize == EOF)
    return ;

  //告知剩余字节
  send(sockfd, &remain_bytes, sizeof(size_t), 0);

  size_t c = 0;
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
      bzero(buffer, sizeof(buffer));
      printf("\rprocess: (%ld/%ld)", c, remain_bytes);
    } while (fileSize > 0);
  }
  printf("\nsend total %ld bytes.\n", c);
  fclose(f);
  return ;
}

void recv_bfile(int sockfd, char *fileName)
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
  size_t passed_bytes = ftell(f);
  send(sockfd, &passed_bytes, sizeof(size_t), 0);

  struct stat s;
  char buff[SIZE];
  size_t fileSize;
  recv(sockfd, &fileSize, sizeof(size_t), 0);
  size_t fs = fileSize;
  size_t c = 0;
  do
  {
    if (fileSize <= 0)
      break;
    size_t num = recv(sockfd, buff, min(fileSize, sizeof(buff)), 0);
    fwrite(buff, num, 1, f);
    fileSize -= num;
    c += num;
    printf("\rprocess : (%ld/%ld)", c, fs);
  } while (fileSize > 0);
  printf("\nrecv total %ld bytes.\n", c);
  fclose(f);
}