#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#define MAX 255
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

void putFile()
{

}

void getFile()
{
	
}
