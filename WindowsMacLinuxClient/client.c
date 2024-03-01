#if defined(_WIN32)
#define BUILD_FOR_WINDOWS
#endif

#ifdef BUILD_FOR_WINDOWS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef BUILD_FOR_WINDOWS
#include <WinSock2.h>
#pragma comment (lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <errno.h>
#endif

#define BUFFERSIZE 1024
#define CMDMAXLEN 255
#define DCMSERVERPORT 64000

#ifdef BUILD_FOR_WINDOWS
WSADATA wsaData;
#endif


int getLine(char* s, int maxchars)
{
	int c, i;
	for (i = 0; i < maxchars - 1 && (c = getchar()) != '\n'; ++i)
	{
		s[i] = c;
	}
	s[i] = '\0';
	return i;
}


int main(int argc, char* argv[])
{
	int soc, n;
	struct sockaddr_in serveraddr;
	char buffer[BUFFERSIZE+1];
	
	if(argc != 3) {
		printf("\nUsage dcmclient ipaddress port");
		exit(1);
	}
	
	//wsainit for windows only
#ifdef BUILD_FOR_WINDOWS
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("\nWSAStartup() failed; WSAGetLastError() returns %d", WSAGetLastError());
		exit(2);
	}
#endif

	//create socket
	if ((soc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
#ifdef BUILD_FOR_WINDOWS
		printf("\nsocket() failed; WSAGetLastError() returns %d", WSAGetLastError());
		WSACleanup();
#else
		printf("\nsocket() failed; errno: %d", errno);
#endif
		exit(3);
	}

	//fill in server address structure
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port        = htons( atoi(argv[2]) );
	//serveraddr.sin_port = htons(DCMSERVERPORT);
	//serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	//connect to server
	if (connect(soc, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
	{
#ifdef BUILD_FOR_WINDOWS
		printf("\nconnect failed; WSAGetLastError() returns %d\n", WSAGetLastError());
		closesocket(soc);
		WSACleanup();
#else
		printf("\nconnect failed; errno is  %d\n", errno);
		close(soc);
#endif
		exit(4);
	}

	//recieve acknowledement from server
	printf("\nWaiting for server.\n");
	if ((n = recv(soc, (char*)&buffer, BUFFERSIZE, 0)) < 0)
	{
#ifdef BUILD_FOR_WINDOWS
		printf("\nrecv failed; WSAGetLastError() returns %d\n", WSAGetLastError());
		closesocket(soc);
		WSACleanup();
#else
		printf("\nconnect failed; errno is  %d\n", errno);
		close(soc);
#endif
		exit(5);
	}
	buffer[n] = '\0';
	printf("\n\n%s", buffer);

	for (; ; )
	{
		printf("\n# ");
		fflush(stdout);
		//read command with arguments from user
		do
		{
			getLine(buffer, CMDMAXLEN);
		} while (buffer[0] == '\0');

		//process quit command and breakout
		if (!strcmp(buffer,"done"))
		{
			if (send(soc, buffer, strlen(buffer), 0) < 0)
			{
#ifdef BUILD_FOR_WINDOWS
				printf("\nsend() failed; WSAGetLastError() returns %d\n", WSAGetLastError());
				closesocket(soc);
				WSACleanup();
#else
				printf("\nsend failed; errno is  %d\n", errno);
				close(soc);
#endif
				exit(6);
			}
			printf("\nclosing connection\n");
			break;
		}
	
		
		//send commmand with arguments to server
		if (send(soc, buffer, strlen(buffer), 0) < 0)
		{
#ifdef BUILD_FOR_WINDOWS
			printf("\nsend() failed; WSAGetLastError() returns %d\n", WSAGetLastError());
			closesocket(soc);
			WSACleanup();
#else
			printf("\nsend failed; errno is  %d\n", errno);
			close(soc);
#endif
			exit(7);
		}

		//recieve result from server
		if ((n = recv(soc, (char*)&buffer, BUFFERSIZE, 0)) < 0)
		{
#ifdef BUILD_FOR_WINDOWS
			printf("\nrecv() failed; WSAGetLastError() returns %d\n", WSAGetLastError());
			closesocket(soc);
			WSACleanup();
#else
			printf("\nrecv failed; errno is  %d\n", errno);
			close(soc);
#endif
			exit(8);
		}
		buffer[n] = '\0';
		printf("%s", buffer);
	}//end of for loop

	//cleanup
#ifdef BUILD_FOR_WINDOWS
	closesocket(soc);
	WSACleanup();
#else
	close(soc);
#endif

	return 0;
}