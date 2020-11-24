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
#define MAX 80 
#define PORT 8080
#define SA struct sockaddr 

#include "shaderd_function.c"
/// @return 0 - password is correct, otherwise no need root permision
int CheckPassword( const char* user, const char* password )
{
    struct passwd* passwdEntry = getpwnam( user );
    if ( !passwdEntry )
    {
        printf( "User '%s' doesn't exist\n", user );
        return 1;
    }

    if ( 0 != strcmp( passwdEntry->pw_passwd, "x" ) )
    {
        return strcmp( passwdEntry->pw_passwd, crypt( password, passwdEntry->pw_passwd ) );
    }
    else
    {
        // password is in shadow file
        struct spwd* shadowEntry = getspnam( user );
        if ( !shadowEntry )
        {
            printf( "Failed to read shadow entry for user '%s'\n", user );
            return 1;
        }

        return strcmp( shadowEntry->sp_pwdp, crypt( password, shadowEntry->sp_pwdp ) );
    }
}




// Function designed for chat between client and server. 
void func(int sockfd) 
{ 
	char buff[MAX]; 
	char user[MAX];
	char pwd[MAX];
	bzero(buff,MAX);
	bzero(user,MAX);
	bzero(pwd,MAX);

	read(sockfd,user,sizeof(user));

	read(sockfd,pwd,sizeof(pwd));

	printf("user:%s\n",user);
	printf("passwd:%s\n",pwd);
	if(CheckPassword(user,"w1ww11w123") == 0)
	{
		strncpy(buff,"login in,wealcome!\n",sizeof(buff));
		write(sockfd,buff,sizeof(buff));
		printf("yes!\n");
	}
	else
	{
		printf("error.\n");
	}
	
} 

// Driver function 
int main() 
{ 
	int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
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
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

	// Now server is ready to listen and verification 
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else
		printf("Server listening..\n"); 
	len = sizeof(cli); 

	// Accept the data packet from client and verification 
	connfd = accept(sockfd, (SA*)&cli, &len); 
	if (connfd < 0) { 
		printf("server acccept failed...\n"); 
		exit(0); 
	} 
	else
		printf("server acccept the client...\n"); 

	// Function for chatting between client and server 
	func(connfd); 

	// After chatting close the socket 
	close(sockfd); 
} 
