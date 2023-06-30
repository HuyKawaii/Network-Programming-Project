#include "ServerSocket.h"

ServerSocket::ServerSocket(){
  WSADATA wsa;
  struct sockaddr_in server;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
      error("Failed WSAStartup.\n");

  std::cout << "Winsock Initialised.\n";

  if ((server_so = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
      error("Failed socket.\n");
      
  std::cout << "Socket created.\n";

  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_port = htons(5500);

  if (bind(server_so, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
      error("Bind failed.\n");

  if (listen(server_so, MAX_PENDING) < 0)
      error("Listen failed.\n");

  std::cout << "Waiting for incoming connections..\n";
};

void ServerSocket::close()
{
    closesocket(this->server_so);
    WSACleanup();
    return;
}

void ServerSocket::error(std::string message)
  {
      std::cerr << message << ": " << WSAGetLastError() << "\n";
      exit(1);
      return;
  }

void ServerSocket::error(std::string message, bool isThread)
  {
      std::cerr << message << ": " << WSAGetLastError() << "\n";
      if (!isThread)
        exit(1);
      return;
  }