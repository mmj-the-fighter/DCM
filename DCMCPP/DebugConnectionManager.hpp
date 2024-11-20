#ifndef _DEBUG_CONNECTION_MANAGER_
#define _DEBUG_CONNECTION_MANAGER_

#include <memory>
#include <atomic>
#include <thread>
#include <sstream>
#include <functional>
#include <chrono>

#include "CommandDispatcher.hpp"

#if defined(_WIN32)
#define BUILD_FOR_WINDOWS
#endif

#ifdef BUILD_FOR_WINDOWS
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
typedef SOCKET SocketType;
typedef int dcm_socket_addr_len_t;
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
typedef int SocketType;
typedef socklen_t dcm_socket_addr_len_t;
#define INVALID_SOCKET -1
#endif

#define BUFFERSIZE 4096
#define SERVERPORT 64000
#define MAXCLIENTS 2

class DebugConnectionManager
{
	std::atomic<int> clientCount{0};
	int maxClients;
	std::shared_ptr<CommandDispatcher> cmdDispatcher = nullptr;
	std::string ackStr = "Awaiting commands.\nCommand Examples:\nhealth -20\nrotation 24.5\naxis 1 1 1\ndone\n";
	std::string nackStr = "Server Capacity exceeded\n";
	std::string doneCommand = "done\n";
	std::string doneCommand2 = "done";
#ifdef BUILD_FOR_WINDOWS
	WSADATA wsaData;
#endif
	SocketType serverSocket;
	int numberOfBytesReceived;
	struct sockaddr_in serverAddress;
	char buffer[BUFFERSIZE + 1];
public:
	DebugConnectionManager() {
		maxClients = MAXCLIENTS;
		Init();
	}
	void SetCommandDispatcher(std::shared_ptr<CommandDispatcher>& aCmdDispatcher) {
		cmdDispatcher = aCmdDispatcher;
	}

	void SetAcknowledgementString(const std::string& str) {
		ackStr = str;
	}

	void SetMaxClients(int n) {
		maxClients = n;
	}

	

private:
	void Init() {
#ifdef BUILD_FOR_WINDOWS
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			std::cout << "WSAStartup() failed; WSAGetLastError() returns: " << WSAGetLastError() << '\n';
			return;
		}
		std::cout << "init success\n";
#endif
		std::thread serverThread(
			std::bind(&DebugConnectionManager::ListenForClients,
				this,
				SERVERPORT)
		);
		serverThread.detach();
	}

	void ListenForClients(int port) {
		//Create a server socket
		if ((serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		{
#ifdef BUILD_FOR_WINDOWS
			std::cout<<"socket() failed; WSAGetLastError() returns: "<< WSAGetLastError() << '\n';
			WSACleanup();
#else
			std::cout<<"socket() failed; errno: "<< errno << '\n';
#endif
			return;
		}
		std::cout << "socket created\n";

		//bind
		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = INADDR_ANY;
		serverAddress.sin_port = htons(port);
		if (::bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
#ifdef BUILD_FOR_WINDOWS
			std::cout<<"bind failed; WSAGetLastError() returns: "<<WSAGetLastError() <<'\n';
			closesocket(serverSocket);
			WSACleanup();
#else
			std::cout << "bind() failed; errno: " << errno << '\n';
			close(serverSocket);
#endif
			return;
		}
		std::cout<<"socket bound\n";


		//listen 
		if (listen(serverSocket, 1) < 0) {
#ifdef BUILD_FOR_WINDOWS
			std::cout << "listen failed; WSAGetLastError() returns: "<< WSAGetLastError()<<'\n';
			closesocket(serverSocket);
			WSACleanup();
#else
			std::cout << "listen() failed; errno: " << errno << '\n';
			close(serverSocket);
#endif
			return;
		}
		std::cout << "listening on port: " << port << '\n';


		while (true) {
			SocketType clientSocket = accept(serverSocket, NULL, NULL);
			if (clientSocket == INVALID_SOCKET) {
#ifdef BUILD_FOR_WINDOWS
				std::cout << "Accept failed; WSAGetLastError() returns" << WSAGetLastError() << '\n';
				closesocket(serverSocket);
				WSACleanup();
#else
				std::cout << "accept() failed; errno: " << errno << '\n';
				close(serverSocket);
#endif	
				return;
			}
			std::cout << "new client joined\n";
			++clientCount;
			std::thread clientThread (
				std::bind(&DebugConnectionManager::ReceiveCommandsFromClientAndDispatch, 
				this, 
				clientSocket)
			);
			clientThread.detach(); 
		}
		std::cout << "stopping server thread\n";
#ifdef BUILD_FOR_WINDOWS
		closesocket(serverSocket);
		WSACleanup();
#else
		close(serverSocket);
#endif
		return;

	}

	void ReceiveCommandsFromClientAndDispatch(SocketType clientSocket) {
		std::string clientIp = "";
		struct sockaddr_in clientAddr;
		dcm_socket_addr_len_t clientAddrLen = sizeof(clientAddr);
		if (getpeername(clientSocket, (struct sockaddr*)&clientAddr, &clientAddrLen) != -1) {
			char ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));
			clientIp = ip;
		}
		bool continueInLoop = true;
		int len;
		if (clientCount > maxClients) {
			len = static_cast<int>(nackStr.size()) + 1;
			if (send(clientSocket, nackStr.c_str(), len, 0) < 0) {
#ifdef BUILD_FOR_WINDOWS
				std::cout << "send() failed; WSAGetLastError() returns: " << WSAGetLastError() << '\n';
				closesocket(clientSocket);
#else
				std::cout << "send() failed; errno: " << errno << '\n';
				close(clientSocket);
#endif
				return;
			}
			std::cout << "closing connection of client: "<<clientIp<<"\n";
			continueInLoop = false;
		}
		else {
			len = static_cast<int>(ackStr.size()) + 1;
			if (send(clientSocket, ackStr.c_str(), len, 0) < 0) {
#ifdef BUILD_FOR_WINDOWS
				std::cout << "send() failed; WSAGetLastError() returns: " << WSAGetLastError() << '\n';
				closesocket(clientSocket);
#else
				std::cout << "send() failed; errno: " << errno << '\n';
				close(clientSocket);
#endif
				return;
			}
		}
		
		std::cout << "entering loop of client: "<<clientIp<<"\n";
		while (continueInLoop) {
			if ((numberOfBytesReceived = recv(clientSocket, (char*)&buffer, BUFFERSIZE, 0)) < 0) {
#ifdef BUILD_FOR_WINDOWS
				std::cout << "recv() failed; WSAGetLastError() returns: " << WSAGetLastError() << '\n';
				closesocket(clientSocket);
#else
				std::cout << "send() failed; errno: " << errno << '\n';
				close(clientSocket);
#endif
				return;
			}
			if (numberOfBytesReceived == 0) {
				std::cout << "Client failure..\n";
				break;
			}
			buffer[numberOfBytesReceived] = '\0';
			std::string receivedData(buffer);
			if (receivedData.compare(doneCommand) == 0 || receivedData.compare(doneCommand2) == 0) {
				continueInLoop = false;
				std::cout<<"done command came\n";
			}
			else {
				std::vector<std::string> cmdWithArgs;
				std::stringstream ss(receivedData);
				std::string token;
				while (ss >> token) {
					cmdWithArgs.push_back(token);
				}

				bool isExecutedOk = false;
				if (cmdDispatcher != nullptr) {
					isExecutedOk = cmdDispatcher->ExecuteCommand(cmdWithArgs);
				}
				if(isExecutedOk) {
					buffer[0] = 'O';
					buffer[1] = 'K';
					buffer[2] = '\n';
					buffer[3] = '\0';
				}else{
					buffer[0] = 'N';
					buffer[1] = 'O';
					buffer[2] = 'T';
					buffer[3] = ' ';
					buffer[4] = 'O';
					buffer[5] = 'K';
					buffer[6] = '\n';
					buffer[7] = '\0';
				}
				if (send(clientSocket, buffer, static_cast<int>(strlen(buffer)), 0) < 0)
				{
#ifdef BUILD_FOR_WINDOWS
					std::cout << "send() failed; WSAGetLastError() returns: " << WSAGetLastError() << '\n';
					closesocket(clientSocket);
#else
					std::cout << "send() failed; errno: " << errno << '\n';
					close(clientSocket);
#endif
					return;
				}
			}
		}
		std::cout << "exiting loop of client: "<<clientIp<<"\n";
#ifdef BUILD_FOR_WINDOWS
		closesocket(clientSocket);
#else
		close(clientSocket);
#endif
		--clientCount;
	}
	
};

#endif
