/*
Client-server application
Author: Flakey Roman
Var: 18
Short name: Гра "вгадування 4-значного числа".
Task: Користувач на клієнті вгадує 4-значне ціле, яке зберігається на сервері.
    З клієнта передаються 4 цифри, на які сервер дає відповідь із двох цифр:
    кількість правильних цифр та кількість цифр на своїх місцях.
    Клієнт в ході гри може її завершити, почати нову гру, завершити сеанс.
    Клієнт може здатися і тоді сервер розкриває загадане число.
    Користувач на клієнті може вибирати спосіб задання спроб:
    в діалозі вводити самому чи автоматична генерація випадкових
    4-х значних чисел в заданій кількості.
*/

#define _WIN32_WINNT 0x501 // fix some error

#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>

#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdlib>

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


void socketErrorCheck(int returnValue, SOCKET socketToClose, const char *action)
{
    const char *actionAttempted = action;
    if (returnValue == SOCKET_ERROR)
    {
        printf("Stop by error [%d] on %s", WSAGetLastError(), actionAttempted);
        closesocket(socketToClose); // WSACleanup(); exit(1);
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


void checkTryCode(short tryCode, short rightCode, char* outStr)
{
    int rightNumber = 0, rightPlace = 0;
    unsigned short rightNumberCode[4];

    for (short k = 0; k<4; rightCode=rightCode/10, k++){
        rightNumberCode[k] = rightCode%10;
    };

    for (short k = 0; k<4; tryCode=tryCode/10, k++){
        short currNumb = tryCode%10;

        for (short i=0; i<4; i++){
            if (rightNumberCode[i] == currNumb){
                rightNumber++;
                if (k == i){
                  rightPlace++;
                };
                break;
            };
        };
    };

    std::sprintf(outStr, "tf_%d%d", rightNumber, rightPlace);
};


int sendMessage(SOCKET &s, const char *sendbuf, std::ofstream &logfile)
{
    int sendResult;

    sendResult = send(s, sendbuf, DEFAULT_BUFLEN, 0);
    socketErrorCheck(sendResult, s, "send");
    logfile << getTimeNow() << " | SERVER >> socket-" << s << ":" << sendbuf << std::endl;
    logfile.flush();

    return sendResult;
};


void commandController(SOCKET &s, char* clientText, std::unordered_map<SOCKET, short[2]>& socketToUserMap, std::ofstream& logfile)
{
    /*
    STATUS CODE:
    0 - Disconnected.
    1 - Connect/Finish.
    2 - Start/Restart.

    Send:
        get - get status code
        s - start/restart
        fn - finish
        gup - give up
        t_#### - try 4-NUMB

    Return:
        gn-s - success generate start
        gn-r - success generate restart
        f - failed
        fn-s - finish success
        gu-s_#### - give up success _ 4-NUMB
        ws-fn - win success / finished game
        tf_## - try fail / # - number of right / # - in right place
        tf_l - try fail / limit
    */

    int sendbufsize = DEFAULT_BUFLEN;
    char sendbuf[sendbufsize] = "";

    std::string recvString(clientText);
    std::cout << recvString << std::endl;
    if (recvString == "Who")
    {
        strncpy(sendbuf, "Flakey Roman k-23, var 18. \"Guess the 4-number\"", sendbufsize);
        sendMessage(s, sendbuf, logfile);
    }

    else if (recvString == "s")
    {
            if (socketToUserMap[s][0] > 0)
            {
                int randomNumber = (rand() % 9000) + 1000;
                socketToUserMap[s][1] = randomNumber;

                if (socketToUserMap[s][0] == 1){
                    socketToUserMap[s][0] = 2;
                    strncpy(sendbuf, "gn-s", sendbufsize);
                }
                else{
                    strncpy(sendbuf, "gn-r", sendbufsize);
                };

            }
            else
            {
                strncpy(sendbuf, "f", sendbufsize);
            };
            sendMessage(s, sendbuf, logfile);
    }
    else if (recvString == "get"){
        std::string tempMsg("Cur_code_" + std::to_string(socketToUserMap[s][0]));
        strncpy(sendbuf, tempMsg.c_str(), sendbufsize);
        sendMessage(s, sendbuf, logfile);
    }

    else if(recvString == "fn")
    {
        if (socketToUserMap[s][0] == 2)
        {
            socketToUserMap[s][0] = 1;
            socketToUserMap[s][1] = 0;
            strncpy(sendbuf, "fn-s", sendbufsize);
        }
        else {strncpy(sendbuf, "f", sendbufsize);
        };
        sendMessage(s, sendbuf, logfile);
    }

    else if (recvString == "gup")
    {
        if (socketToUserMap[s][0] == 2)
        {
            std::string tempMsg("gu-s_" + std::to_string(socketToUserMap[s][1]));
            strncpy(sendbuf, tempMsg.c_str(), sendbufsize);
            socketToUserMap[s][0] = 1;
            socketToUserMap[s][1] = 0;
        }
        else {strncpy(sendbuf, "f", sendbufsize);
        };
        sendMessage(s, sendbuf, logfile);
    }

    else if (recvString.rfind("t_", 0) == 0)
    {

        if (socketToUserMap[s][0] == 2 && recvString.length() == 6)
        {
            int tryCode = stoi(recvString.substr(2, 4));
            if (tryCode >= 1000 && tryCode <= 9999){
                if (tryCode == socketToUserMap[s][1])
                {
                    strncpy(sendbuf, "ws-fn", sendbufsize);
                    sendMessage(s, sendbuf, logfile);

                    socketToUserMap[s][0] = 1;
                    socketToUserMap[s][1] = 0;
                }
                else
                {
                    char resultCheck[5];
                    checkTryCode(tryCode, socketToUserMap[s][1], resultCheck);

                    strncpy(sendbuf, resultCheck, sendbufsize);
                    sendMessage(s, sendbuf, logfile);
                };
            }
            else {
                strncpy(sendbuf, "tf_l", sendbufsize);
                sendMessage(s, sendbuf, logfile);
            };
        }
        else{
            strncpy(sendbuf, "f", sendbufsize);
            sendMessage(s, sendbuf, logfile);
        };
    }

    else
    {
        strncpy(sendbuf, "Unexpected command.", sendbufsize);
        sendMessage(s, sendbuf, logfile);
    };

    std::string().swap(recvString);
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

    SOCKET serverSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;

    struct sockaddr_in servAddr;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen;

    fd_set activeFdSet;
    fd_set readFdSet;

    std::vector<SOCKET> socketsArray;
    std::unordered_map<SOCKET, short[2]> socketToUserMap;

    srand(time(NULL));

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

    while (true)
    {
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
                        closesocket(serverSocket);
                        WSACleanup();
                        exit(1);
                    }
                    else
                    {
                        printf("Got a connection socket-%d!\n", clientSocket);
                        logfile << getTimeNow() << " | SERVER-CONNECT [socket-" << clientSocket << "]" << std::endl;
                        logfile.flush();
                    };
                    socketToUserMap[clientSocket][0] = 1;

                    FD_SET(clientSocket, &activeFdSet);
                    newSocketArray.push_back(clientSocket);
                }
                else
                {
                    ZeroMemory(recvbuf, recvbuflen);
                    codeResult = recv(currSocketFd, recvbuf, recvbuflen, 0);

                    if (codeResult > 0)
                    {
                        logfile << getTimeNow() << " | SERVER << socket-" << currSocketFd << ":" << recvbuf << std::endl;
                        logfile.flush();

                        commandController(currSocketFd, recvbuf, socketToUserMap, logfile);
                    }

                    else if (codeResult == 0 || codeResult == -1)
                    {
                        printf("Closed connection with socket-%d\n", currSocketFd);

                        logfile << getTimeNow() << " | SERVER-DISCONNECT [socket-" << currSocketFd << "]" << std::endl;
                        logfile.flush();

                        shutdown(currSocketFd, SD_SEND);
                        closesocket(currSocketFd);
                        FD_CLR(currSocketFd, &activeFdSet);

                        socketToUserMap.erase(currSocketFd);
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