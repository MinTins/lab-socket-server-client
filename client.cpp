#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>

#include "winsock2.h"

#define DEFAULT_PORT 1043
#define DEFAULT_BUFLEN 1024


SOCKET constructSocket()
{
  SOCKET sock;
  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock == INVALID_SOCKET)
  {
    printf("Error at socket() in constructSocket: %ld\n", WSAGetLastError());
    WSACleanup();
    return 0;
  };
  return sock;
}


void socketErrorCheck(int returnValue, SOCKET socketToClose, const char* action)
{
	const char *actionAttempted = action;
  if(returnValue == SOCKET_ERROR){
      if(WSAGetLastError() == WSAECONNREFUSED){
        printf("Server not found/refused the connection.");
      }
      else if(WSAGetLastError() == WSAECONNRESET){
        printf("Server closed the connection.");
      }
      else{
        printf("Stop by error [%d] on %s", WSAGetLastError(), actionAttempted);
      };

      closesocket(socketToClose); WSACleanup(); exit(1);
    };
};


int main()
{
  WSADATA wsaData;

  int bytesRecv;
  int codeResult;

  char sendbuf[DEFAULT_BUFLEN] = "";
  char recvbuf[DEFAULT_BUFLEN] = "";
  int recvbuflen = DEFAULT_BUFLEN;

  SOCKET ConnectSocket;

  sockaddr_in clientService;
  clientService.sin_family = AF_INET;
  clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
  clientService.sin_port = htons(DEFAULT_PORT);

  codeResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (codeResult != NO_ERROR)
  {
    printf("WSAStartup failed with code %d\n", codeResult);
    return 1;
  };

  ConnectSocket = constructSocket();

  codeResult = connect(ConnectSocket, (SOCKADDR *)&clientService, sizeof(clientService));
  socketErrorCheck(codeResult, ConnectSocket, "connect");

  while (true)
  {
    std::cout << "Input:";
    std::cin >> sendbuf;

    codeResult = send(ConnectSocket, sendbuf, strlen(sendbuf), 0);
    socketErrorCheck(codeResult, ConnectSocket, "send");

    printf("Send>>: %s\n", sendbuf);

    bytesRecv = recv(ConnectSocket, recvbuf, recvbuflen, 0);
    if (bytesRecv == 0 || bytesRecv == WSAECONNRESET)
    {
      printf("Connection Closed.\n");
      break;
    };

    printf("Got<<: %s\n", recvbuf);

  }
  WSACleanup();
  return 0;
}