#define _WIN32_WINNT 0x501 // fix some error

#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>

#include <vector>
#include <ctime>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT 1043
#define DEFAULT_BUFLEN 1024


SOCKET constructSocket()
{
	SOCKET sockRtnVal = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockRtnVal == INVALID_SOCKET)
	{
		printf("socket() failed\n");
		WSACleanup();
		exit(1);
	}
	return sockRtnVal;
};



void socketErrorCheck(int returnValue, SOCKET socketToClose, const char* action)
{
	const char *actionAttempted = action;
	if(returnValue == SOCKET_ERROR){
        printf("Stop by error [%d] on %s", WSAGetLastError(), actionAttempted);
        closesocket(socketToClose);// WSACleanup(); exit(1);
    };
}


std::string getTimeNow()
{   
    std::string result;

    std::time_t unixtime = std::time(nullptr);
    result = std::asctime(std::localtime(&unixtime));
    result.pop_back();
    
    return result;
};



void commandController(SOCKET s, std::string clientText, std::ofstream& logfile)
{
    int sendResult = 0;
    std::stringstream msgToSend;
    std::string line;

    if (clientText == "Who")
    {
        msgToSend << "Flakey Roman k-23, var 18. Контексний пошук у файлі(ах).";
        msgToSend << "\ntest line";

    }else{
        msgToSend << "Unexpected command.";
        msgToSend << "\ntest line";
    };

    while(std::getline(msgToSend,line,'\n')){
            sendResult = send(s, (line + "\n").c_str(), (line + "\n").size(), 0);

            logfile << getTimeNow() << " | SERVER >> socket-" << s << ":" << line << std::endl;
            logfile.flush();

            socketErrorCheck(sendResult, s, "send");

            std::string().swap(line);
    };
};
 


int main(void)
{
    WSADATA wsaData;

    std::ofstream logfile;
    logfile.open("server-log.txt", std::ios_base::app);

	int codeResult;
	int sendResult;
	char recvbuf[DEFAULT_BUFLEN] = "";
	int recvbuflen = DEFAULT_BUFLEN;
    std::string recvString;

	SOCKET serverSocket = INVALID_SOCKET;
	SOCKET clientSocket = INVALID_SOCKET;

	struct sockaddr_in servAddr;
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen;

    fd_set activeFdSet;
	fd_set readFdSet;

    std::vector<SOCKET> socketsArray;

    codeResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (codeResult != NO_ERROR)
	{
		printf("WSAStartup failed with code %d\n", codeResult);
		return 1;
	}

    serverSocket = constructSocket();

    ZeroMemory(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	servAddr.sin_port = htons(DEFAULT_PORT);

    codeResult = bind(serverSocket, (struct sockaddr *)&servAddr, sizeof(servAddr));
    socketErrorCheck(codeResult, serverSocket, "bind");

    codeResult = listen(serverSocket, SOMAXCONN);
    socketErrorCheck(codeResult, serverSocket, "listen");

    FD_ZERO(&activeFdSet);
	FD_SET(serverSocket, &activeFdSet);
    socketsArray.push_back(serverSocket);

    std::vector<SOCKET> newSocketArray(socketsArray);

    printf("waiting for connections...\n");

    while(true) {
        readFdSet = activeFdSet;

        codeResult = select(FD_SETSIZE, &readFdSet, NULL, NULL, NULL);
        socketErrorCheck(codeResult, serverSocket, "select");

        for (int i = 0; i < (int)socketsArray.size(); i++)
        {
            SOCKET currSocketFd = socketsArray.at(i);
            if (FD_ISSET(currSocketFd, &readFdSet))
			{
                if (currSocketFd == serverSocket)
                {
                    clientAddrLen = sizeof(clientAddr);
					clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
                    if (clientSocket == INVALID_SOCKET)
					{
						printf("Stop by error [%d] on accept", WSAGetLastError());
                        closesocket(serverSocket); WSACleanup(); exit(1);
					}
					else
					{
						printf("Got a connection socket-%d!\n", clientSocket);
                        logfile << getTimeNow() << " | SERVER-CONNECT [socket-"<< clientSocket << "]" << std::endl;
                        logfile.flush();
					}
                    FD_SET(clientSocket, &activeFdSet);
                    newSocketArray.push_back(clientSocket);
                }
                else
                {
                    ZeroMemory(recvbuf, recvbuflen);
                    codeResult = recv(currSocketFd, recvbuf, recvbuflen, 0);

                    if (codeResult > 0)
                    {
                        recvString.append(recvbuf);

                        logfile << getTimeNow() << " | SERVER << socket-" << currSocketFd << ":" << recvString << std::endl;
                        logfile.flush();

                        commandController(currSocketFd, recvString, logfile);
                        

                        std::string().swap(recvString);
                    }

                    else if (codeResult == 0 || codeResult == -1) 
                    {  
                        printf("Closed connection with socket-%d\n", currSocketFd);

                        logfile << getTimeNow() << " | SERVER-DISCONNECT [socket-"<< currSocketFd << "]" << std::endl;
                        logfile.flush();

                        shutdown(currSocketFd, SD_SEND);
                        closesocket(currSocketFd);
                        FD_CLR(currSocketFd, &activeFdSet);

                        std::vector<SOCKET> tempArr;
                        tempArr.push_back(serverSocket);
                        newSocketArray.assign(tempArr.begin(), tempArr.end());
                    }

                    else
                    {
                        printf("hmmm.. idk this result code %d\n", codeResult);
                    };
                    ZeroMemory(recvbuf, recvbuflen);
                };
            }
            
        };
        socketsArray.assign(newSocketArray.begin(), newSocketArray.end());

    };
    WSACleanup();
    logfile.close();
	return 0;
};