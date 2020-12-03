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
    char * user;
	char * pwd;
	char * ip;
	char * port;
}login_info;

typedef enum
{
	ok,notexists,login_failed
}fileInformation;

char *readFile(char *fileName) {
    FILE *file = fopen(fileName, "r");
    char *code;
    size_t n = 0;
    int c;

    if (file == NULL) 
	{
		return NULL;
	}
    fseek(file, 0, SEEK_END);
    long f_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    code = malloc(f_size);

    while ((c = fgetc(file)) != EOF) {
        code[n++] = (char)c;
    }

    code[n] = '\0';        

	fclose(file);
    return code;
}

char *saveFile(char *fileName,char *content)
{
	FILE *file = fopen(fileName, "w+");
	char *code;
	size_t n = 0;

	if(file == NULL)
	{
		printf("open file %s error!",fileName);
		return NULL;
	}

	fputs(content,file);
	fclose(file);
}

void client_usage()
{
	printf("useage:ftpclient <user>:<passwd>@<host>:<port>\n");
}

void send_file(FILE *fp, int sockfd){
  int n;
  char data[SIZE] = {0};

  while(fgets(data, SIZE, fp) != NULL) {
    if (send(sockfd, data, sizeof(data), 0) == -1) {
      perror("[-]Error in sending file.");
      exit(1);
    }
    bzero(data, SIZE);
  }
}

void write_file(int sockfd){
  int n;
  FILE *fp;
  char *filename = "recv.txt";
  char buffer[SIZE];

  fp = fopen(filename, "w");
  while (1) {
    n = recv(sockfd, buffer, SIZE, 0);
	printf("recv:%d",n);
    if (n <= 0){
      break;
      return;
    }
    fprintf(fp, "%s", buffer);
    bzero(buffer, SIZE);
  }
  return;
}

void clean_stdin(void)
{
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}