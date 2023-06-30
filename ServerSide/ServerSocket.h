#include <iostream>
#include <string>
#include <stdlib.h>
#include <winsock.h>


#define MAX_PENDING 5

class ServerSocket{
public:
  ServerSocket();
  void close();
  static void error(std::string message);
  static void error(std::string message, bool isThread);
  SOCKET GetServerSocket() {return server_so;};
private:
  SOCKET server_so;
  
};