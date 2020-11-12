#include <arpa/inet.h>
#include <regex.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef struct 
{
    char * user;
	char * pwd;
	char * ip;
	char * port;
}login_info;


int match(const char *string, const char *pattern)
{
	regex_t re;
	if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) return 0;
	int status = regexec(&re, string, 0, 0, 0);
	regfree(&re);
	if (status != 0) return 0;
	return 1;
    // 	const char* s1 = "abc2";
	// const char* s2 = "b123";
	// const char* re = "b[1-9]+";
	// printf("%s Given string matches %s? %s\n", s1, re, match(s1, re) ? "true" : "false");
	// printf("%s Given string matches %s? %s\n", s2, re, match(s2, re) ? "true" : "false");
}

const char* split(const char* str,char step)
{
    char *t = str;
    while(t)
    {
        if(*t == step)
        {

        }
    }
}