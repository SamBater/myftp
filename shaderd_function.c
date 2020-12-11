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

typedef enum
{
  binary,
  ascii
}trans_mode;

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

size_t min(size_t a,size_t b)
{
  return a>b?b : a;
}

int send_binaryfile(int sockfd,char *buff,char *fileName)
{
  FILE *f = fopen(fileName,"rb");
  fseek(f,0,SEEK_END);
  long long fileSize = ftell(f) ;
  const long long fs = fileSize;
  printf("filesize = %lld\n",fileSize);
  rewind(f);
  if(fileSize == EOF)
    return 0;
  
  char size[32];
  sprintf(size,"%lld",fileSize);

  //事先沟通大小.
  send(sockfd,size,sizeof(long long),0);
  recv(sockfd,size,2,0);
  long long c = 0;
  if(fileSize > 0)
  {
    char buffer[1024];
    do
    {
      size_t num = min(fileSize,sizeof(buffer));
      fread(buffer,1,num,f);
      send(sockfd,buffer,num,0);
      fileSize -= num;
      c += num;
      bzero(buffer,1024);
      printf("process: (%lld/%lld)\n",c,fs);
    } while (fileSize > 0);
    
  }
  printf("send total %lld bytes.\n",c);
  fclose(f);
  return 1;
}


int recive_binaryFile(int sockfd,char *fileName)
{
  FILE *f = fopen(fileName,"wb");
  char buff[1024];
  char size[32];
  recv(sockfd,size,sizeof(long long),0);
  long long fileSize = atoll(size) ;
  const long long fs = fileSize;
  send(sockfd,size,2,0);
  long long c = 0;
  do
  {
    size_t num = recv(sockfd,buff,min(fileSize,sizeof(buff)),0);
    fwrite(buff,1,num,f);
    fileSize -= num;
    c += num;
    printf("process : (%lld/%lld)\n",c,fs);
  } while (fileSize > 0);
  printf("recv total %lld bytes.\n",c);
  fclose(f);
}