/*
Client-server application
Author: Flakey Roman
Type: Client
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

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

#include <ctime>
#include <cstdlib>

#define DEFAULT_PORT 1043
#define DEFAULT_BUFLEN 255


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
        printf("Server not found/refused the connection.\n");
      }
      else if(WSAGetLastError() == WSAECONNRESET){
        printf("Server closed the connection.\n");
      }
      else{
        printf("Stop by error [%d] on %s\n", WSAGetLastError(), actionAttempted);
      };

      closesocket(socketToClose); WSACleanup(); exit(1);
    };
};


std::string getTimeNow()
{
    std::string result;

    std::time_t unixtime = std::time(nullptr);
    result = std::asctime(std::localtime(&unixtime));
    result.pop_back();

    return result;
};


bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
};


int sendMessage(SOCKET& s, std::string sendbuf, int& codeResult, std::ofstream& logfile)
{
    codeResult = send(s, sendbuf.c_str(), sendbuf.size(), 0);
    socketErrorCheck(codeResult, s, "send");
    logfile << getTimeNow() << " | CLIENT -> SERVER" << ":" << sendbuf << std::endl;
    logfile.flush();
    
    return codeResult;
};


int recvMessage(SOCKET& s, char* recvbuf, int& recvbuflen, int& bytesRecv, std::ofstream& logfile)
{
  bytesRecv = recv(s, recvbuf, recvbuflen, 0);
  if (bytesRecv == 0 || bytesRecv == WSAECONNRESET)
  {
     printf("Connection Closed.\n");
      return -1;
  };
  logfile << getTimeNow() << " | SERVER -> CLIENT" << ":" << recvbuf << std::endl;
  logfile.flush();

  std::string recvString(recvbuf);

  if (recvString == "gn-s") printf("<- SERVER: [GAME-STARTED] Code generated success.\n");
  else if (recvString == "gn-r") printf("<- SERVER: [GAME-RESTARTED] Code generated success.\n");
  else if (recvString == "f") printf("<- SERVER: [FAILED] Check your input data/state.\n");
  else if (recvString == "fn-s") printf("<- SERVER: [FINISH] Game break by command.\n");
  else if (recvString.rfind("gu-s_", 0) == 0) printf("<- SERVER: [GIVE-UP] Right code was: %s\n", recvString.substr(5, 4).c_str());
  else if (recvString == "ws-fn") printf("<- SERVER: [WIN] Game win.\nInput: Start - to start a new game.\n");
  else if (recvString.rfind("tf_", 0) == 0) printf("<- SERVER: [TRY-FAIL] Right number - %c, In Right Place - %c\n", recvString[3], recvString[4]);
  else if (recvString == "f") printf("<- SERVER: [ERROR] Unexpected commmand.\n");
  else printf("<- SERVER: %s\n", recvString.c_str());

  return bytesRecv;
};



int main()
{
  WSADATA wsaData;

  std::ofstream logfile;
  logfile.open("client-log.txt", std::ios_base::app);

  int bytesRecv;
  int codeResult;

  char sendbuf[DEFAULT_BUFLEN] = "";
  char recvbuf[DEFAULT_BUFLEN] = "";
  int recvbuflen = DEFAULT_BUFLEN;

  std::string inputbuf;

  SOCKET ConnectSocket;

  srand(time(NULL));

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
  printf("[Connected to server.]\n");

  logfile << getTimeNow() << " | CLIENT-CONNECT]" << std::endl;
  logfile.flush();

  while (true)
  {
    std::string().swap(inputbuf);
  
    std::cout << "CLIENT ->: ";
    std::cin >> inputbuf;

    if (inputbuf == "Start")
    {
      sendMessage(ConnectSocket, "s", codeResult, logfile);
      bytesRecv = recvMessage(ConnectSocket, recvbuf, recvbuflen, bytesRecv, logfile);
      if (bytesRecv == -1) break;
    }

    else if (inputbuf == "Who")
    {
      sendMessage(ConnectSocket, "Who", codeResult, logfile);
      bytesRecv = recvMessage(ConnectSocket, recvbuf, recvbuflen, bytesRecv, logfile);
      if (bytesRecv == -1) break;
    }

    else if (inputbuf == "Restart")
    {
      sendMessage(ConnectSocket, "s", codeResult, logfile);
      bytesRecv = recvMessage(ConnectSocket, recvbuf, recvbuflen, bytesRecv, logfile);
      if (bytesRecv == -1) break;
    }

    else if (inputbuf == "GiveUp")
    {
      sendMessage(ConnectSocket, "gup", codeResult, logfile);
      bytesRecv = recvMessage(ConnectSocket, recvbuf, recvbuflen, bytesRecv, logfile);
      if (bytesRecv == -1) break;
    }

    else if (inputbuf == "Finish")
    {
      sendMessage(ConnectSocket, "fn", codeResult, logfile);
      bytesRecv = recvMessage(ConnectSocket, recvbuf, recvbuflen, bytesRecv, logfile);
      if (bytesRecv == -1) break;
    }

    else if (inputbuf.rfind("TryCode_", 0) == 0 && inputbuf.length() == 12 
    && is_number(inputbuf.substr(8, 4)))
    {
      sendMessage(ConnectSocket, "t_" + inputbuf.substr(8, 4), codeResult, logfile);
      bytesRecv = recvMessage(ConnectSocket, recvbuf, recvbuflen, bytesRecv, logfile);
      if (bytesRecv == -1) break;
    }

    else if (inputbuf.rfind("TryRand_", 0) == 0
    && is_number(inputbuf.substr(8, 5)))
    {
      unsigned int tryCount = stoi(inputbuf.substr(8, 5));

      for (int i = 0; i < tryCount; i++)
      {
        int randomNumber = (rand() % 9000) + 1000;

        std::string tempMsg("t_" + std::to_string(randomNumber));
        sendMessage(ConnectSocket, tempMsg.c_str(), codeResult, logfile);
        std::string().swap(tempMsg);

        bytesRecv = recvMessage(ConnectSocket, recvbuf, recvbuflen, bytesRecv, logfile);
        if (bytesRecv == -1) break;

        if (recvbuf == "ws-fn")
        {
          printf("[RandomCode-check *SUCCESS*] Code was: %d\n", randomNumber);
          break;
        }
        else
        {
          printf("[Check-code] %d - FAIL\n", randomNumber);
        };

      };
    }
    
    else if (inputbuf == "Exit")
    {
      printf("Closing the connection...\n");
      break;
    }

    else
    {
      printf("  Availible command:\n"
      "    Start/Restart - start game-session\n"
      "    Finish - stop game session\n"
      "    GiveUp - stop game and tell hidden number\n"
      "    TryCode_#### - try check own code | # - [0-9]\n"
      "    TryRand_# - try check random code | # - count of random code\n"
      "    Exit - close connection.\n");
    };
    std::string().swap(inputbuf);
  };
  closesocket(ConnectSocket);
  WSACleanup();
  printf("[Connection closed.]\n");
  logfile << getTimeNow() << " | CLIENT-DISCONNECT]" << std::endl;
  logfile.flush();
  return 0;
}