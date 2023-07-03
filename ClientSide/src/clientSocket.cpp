#include "clientSocket.h"
#include "display.h"
#include <sstream>
#include "board.h"

std::string gen_random(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);
    
    srand(time(0));
    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return tmp_s;
}

void ClientSocket::error(std::string message)
{
  std::cerr << message << ": " << WSAGetLastError() << "\n";
  exit(1);
  return;
}

void ClientSocket::close()
{
  closesocket(this->so);
  WSACleanup();
  return;
}

void ClientSocket::setDisplayPtr(Display *display)
{
  displayPtr = display;
}

void ClientSocket::setBoardPtr(Board *board)
{
  boardPtr = board;
}

ClientSocket::ClientSocket(std::string server_ip, unsigned short server_port)
{
  struct sockaddr_in server;
  WSADATA wsa;

  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    error("Failed WSAStartup");

  std::cout << "Winsock Initialised.\n";

  if ((so = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    error("Error on created the socket");

  std::cout << "Socket created.\n";

  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(server_ip.c_str());
  server.sin_port = htons(server_port);

  if (connect(so, (struct sockaddr *)&server, sizeof server) < 0)
    error("Failed to connect");

  std::cout << "Connected to " << server_ip << ":" << server_port << "\n";

  u_long mode = 1; // Enable non-blocking mode
  if (ioctlsocket(so, FIONBIO, &mode) != 0)
    error("Failed to set the socket to non-blocking mode");

  name.assign(gen_random(3)); // assign name here
  roomNotFound = false;
  roomFull = false;
  isOwner = false;
  isGuestReady = false;
}

void ClientSocket::sendJoinRoom()
{
  int bytes_received;
  std::string message;
  std::string code;
  char buffer[RECEIVE_BUFFER_SIZE];

  code.assign(JOIN_ROOM_CODE);
  message = code + '\n' + roomCode + '\n' + name + '\n';
  if (send(so, message.c_str(), RECEIVE_BUFFER_SIZE, 0) == SOCKET_ERROR)
    error("Error sending message to server. Exiting\n");
}

void ClientSocket::handleJoinRoom(std::string reply, std::string opponentName)
{
  roomNotFound = false;
  roomFull = false;
  if (!reply.compare("1"))
  {
    std::cout << "Room found\n";
    displayPtr->setMenu(Display::Menu::roomMenu);
    this->opponentName.assign(opponentName);
  }
  else if (!reply.compare("0"))
  {
    std::cout << "Room not found\n";
    roomNotFound = true;
  }
  else if (!reply.compare("2"))
  {
    std::cout << "Room is full\n";
    roomFull = true;
  }
  else
  {
    std::cout << "Message not recognized. Exiting\n";
    exit(0);
  }
}

void ClientSocket::sendCreateRoom()
{
  int bytes_received;
  std::string message;
  std::string code;
  char buffer[RECEIVE_BUFFER_SIZE];

  std::cout << "Sending create room message\n";
  roomCode.assign("");
  code.assign(CREATE_ROOM_CODE);
  message = code + '\n' + name + '\n';
  if (send(so, message.c_str(), RECEIVE_BUFFER_SIZE, 0) == SOCKET_ERROR)
    error("Error sending message to server. Exiting\n");
}

void ClientSocket::handleCreateRoom(std::string roomCode)
{
  this->roomCode.assign(roomCode);
  isOwner = true;
}

void ClientSocket::sendLeaveRoom()
{
  int bytes_received;
  std::string code;
  std::string message;
  char buffer[RECEIVE_BUFFER_SIZE];

  std::cout << "Sending leaving message\n";
  code.assign(LEAVE_ROOM_CODE);
  message = code + '\n';
  if (send(so, message.c_str(), RECEIVE_BUFFER_SIZE, 0) == SOCKET_ERROR)
    error("Error sending message to server. Exiting\n");
}

void ClientSocket::handleLeaveRoom()
{
  std::cout << "Leaved room\n";
  opponentName = "";
  roomCode = "";
  isOwner = false;
  isGuestReady = false;
}

void ClientSocket::sendStartGame()
{
  int bytes_received;
  std::string code;
  std::string message;
  char buffer[RECEIVE_BUFFER_SIZE];

  std::cout << "Sending start game signal\n";
  code.assign(START_GAME_CODE);
  message = code + '\n';
  if (send(so, message.c_str(), RECEIVE_BUFFER_SIZE, 0) == SOCKET_ERROR)
    error("Error sending message to server. Exiting\n");
}

void ClientSocket::handleStartGame(std::string side)
{
  if (side == "white")
    boardPtr->setPlayerSide(WHITE);
  else
    boardPtr->setPlayerSide(BLACK);
    
  boardPtr->startGame();
}

void ClientSocket::handleNewGuest(std::string newGuestName)
{
  opponentName.assign(newGuestName);
  if (newGuestName == ""){
    isGuestReady = false;
  }
}

void ClientSocket::handleNewOwner(std::string newOwnerName)
{
  if (newOwnerName == name){
    opponentName.assign("");
    isOwner = true;
  }else{
    opponentName.assign(newOwnerName);
    isOwner = false;
  }
}

void ClientSocket::sendMoveSignal(int moveFrom, int moveTo){
  std::string code;
  std::string message;
  char buffer[RECEIVE_BUFFER_SIZE];

  std::cout << "Sending move signal\n";
  code.assign(MOVE_CODE);
  message = code + '\n' + std::to_string(moveFrom) + '\n' + std::to_string(moveTo) + '\n';
  if (send(so, message.c_str(), RECEIVE_BUFFER_SIZE, 0) == SOCKET_ERROR)
    error("Error sending message to server. Exiting\n");
}

void ClientSocket::handleMoveSignal(int moveFrom, int moveTo){
  boardPtr->handleInput(moveFrom, moveTo, nullptr);
}

void ClientSocket::sendReadySignal(){
  std::string code;
  std::string message;
  char buffer[RECEIVE_BUFFER_SIZE];

  std::cout << "Sending ready signal\n";
  code.assign(READY_CODE);
  message = code + '\n' + (isGuestReady ? std::to_string(0) : std::to_string(1)) + '\n';
  if (send(so, message.c_str(), RECEIVE_BUFFER_SIZE, 0) == SOCKET_ERROR)
    error("Error sending message to server. Exiting\n");
}

void ClientSocket::handleReadySignal(int readyStatus){
  isGuestReady = readyStatus;
}

void ClientSocket::handleBufferRead()
{
  int bytes_received = 0;
  char buffer[RECEIVE_BUFFER_SIZE];
  std::string token;

  do
  {
    bytes_received = recv(so, buffer, RECEIVE_BUFFER_SIZE, 0);
    if (bytes_received > 0)
    {
      // Process the received data
      std::stringstream ss(buffer);
      std::getline(ss, token, '\n');
      std::cout << "Token: " << token << '\n';
      if (!token.compare(CREATE_ROOM_CODE))
      {
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        handleCreateRoom(token);
      }
      else if (!token.compare(LEAVE_ROOM_CODE))
      {
        handleLeaveRoom();
      }
      else if (!token.compare(JOIN_ROOM_CODE))
      {
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        std::string token2;
        std::getline(ss, token2, '\n');
        handleJoinRoom(token, token2);
      }
      else if (!token.compare(NEW_GUEST_CODE))
      {
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        handleNewGuest(token);
      }
      else if (!token.compare(NEW_OWNER_CODE))
      {
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        handleNewOwner(token);
      }
      else if (!token.compare(START_GAME_CODE))
      {
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        handleStartGame(token);
      }
      else if (!token.compare(MOVE_CODE))
      {
        int moveFrom, moveTo;
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        moveFrom = std::stoi(token);
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        moveTo = std::stoi(token);
        handleMoveSignal(moveFrom, moveTo);
      }
      else if (!token.compare(READY_CODE))
      {
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        handleReadySignal(std::stoi(token));
      }
    }
    else if (bytes_received == 0)
    {
      ClientSocket::error("Connection closed by the server");
      break;
    }
    else
    {
      int error = WSAGetLastError();
      if (error != WSAEWOULDBLOCK)
      {
        ClientSocket::error("Failed to receive data. Error code");
        break;
      }
      // No data available in the receive buffer
      // Do other work or sleep for a while before calling recv() again
    }
  } while (bytes_received > 0);
}