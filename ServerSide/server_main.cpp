#include "ServerSocket.cpp"
#include <thread>
#include "room.cpp"
#include <ctime>
#include <iostream>
#include <sstream>
#include "account.cpp"
#include <fstream>
#include <map>

#define RECEIVE_BUFFER_SIZE 64
#define MAX_CLIENT 100
#define CREATE_ROOM_CODE "CREATE"
#define START_GAME_CODE "START"
#define JOIN_ROOM_CODE "JOIN"
#define LEAVE_ROOM_CODE "LEAVE"
#define NEW_GUEST_CODE "NEWGUEST"
#define NEW_OWNER_CODE "NEWOWNER"
#define OPPONENT_LEAVE_ROOM_CODE "OPPOLEAVE"
#define MOVE_CODE "MOVE"
#define READY_CODE "READY"
#define RESIGN_CODE "RESIGN"
#define LOGIN_CODE "LOGIN"

void handle_client(int client_socket);
std::string gen_random(const int len);
std::map<std::string, Account> initializeAccountList();
void displayAccountList(const std::map<std::string, Account>& accountList);

Room roomList[MAX_CLIENT];
std::map<std::string, Account> accountList;

int main()
{
  std::thread threadArray[MAX_CLIENT];
  ServerSocket serverSock;
  struct sockaddr_in client;
  int client_length;
  SOCKET client_so;

  accountList = initializeAccountList();
  
  client_length = sizeof(client);
  while (client_so = accept(serverSock.GetServerSocket(), (struct sockaddr *)&client, &client_length))
  {
    std::cout << "New connection accepted " << inet_ntoa(client.sin_addr) << "\n";
    for (int i = 0; i < MAX_CLIENT; i++)
    {
      if (!threadArray[i].joinable())
      {
        threadArray[i] = std::thread(handle_client, client_so);
        threadArray[i].detach();
        break;
      }
    }
  }

  serverSock.close();
  return 0;
}

std::string gen_random(const int len)
{
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::string tmp_s;
  tmp_s.reserve(len);

  srand(time(0));
  for (int i = 0; i < len; ++i)
  {
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return tmp_s;
}

int coin_flip(){
  srand(time(0));
  return rand() % 2;
}

void handle_client(int client_socket)
{
  char buffer[RECEIVE_BUFFER_SIZE];
  std::string message;
  std::string code;
  int roomIndex = -1;
  int bytes_received;
  std::thread::id threadId = std::this_thread::get_id();

  u_long mode = 1; // Enable non-blocking mode
  if (ioctlsocket(client_socket, FIONBIO, &mode) != 0)
    ServerSocket::error("Failed to set the socket to non-blocking mode", true);

  while (1)
  {
    bytes_received = recv(client_socket, buffer, RECEIVE_BUFFER_SIZE, 0);

    if (bytes_received > 0)
    {

      std::stringstream ss(buffer);
      std::string token;
      std::getline(ss, token, '\n');
      std::cout << "Token: " << token << '\n';

      if (!token.compare(CREATE_ROOM_CODE))
      {
        std::getline(ss, token, '\n');
        std::cout << "Create room with owner : " << token << '\n';
        std::cout << "Creating room \n";

        std::string roomCode = gen_random(6);
        for (int i = 0; i < MAX_CLIENT; i++)
        {
          if (!roomList[i].isRoomOccupied())
          {
            roomIndex = i;
            roomList[i].setRoomCode(roomCode);
            roomList[i].setRoomOccupied(true);
            roomList[i].setOwner(threadId, token);
            code.assign(CREATE_ROOM_CODE);
            message = code + '\n' + roomCode + '\n';
            send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
            std::cout << "Room " << i << " is occupied with code " << roomCode << '\n';
            break;
          }
        }
      }
      else if (!token.compare(LEAVE_ROOM_CODE))
      {
        if (roomIndex < 0){
          std::cout << "Player not in a room but send leave room request. Ignoring...\n";
          continue;
        }

        if (roomList[roomIndex].isOwner(threadId))
        {
          roomList[roomIndex].removeOwner();
          roomIndex = -1;
          code.assign(LEAVE_ROOM_CODE);
          message = code + '\n';
          send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
          std::cout << "Room " << roomIndex << " with code " << roomList[roomIndex].getRoomCode() << " has lost its owner\n";
        }
        else if (roomList[roomIndex].isGuest(std::this_thread::get_id()))
        {
          roomList[roomIndex].removeGuest();
          roomIndex = -1;
          code.assign(LEAVE_ROOM_CODE);
          message = code + '\n';
          send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
          std::cout << "Room " << roomIndex << " with code " << roomList[roomIndex].getRoomCode() << "has lost its guest\n";
        }
        else
        {
          std::cout << "Something is wrong cant find room to leave";
        }
      }
      else if (!token.compare(JOIN_ROOM_CODE))
      {
        bool foundRoom = false;
        bool roomFull = false;
        std::getline(ss, token, '\n');
        std::cout << "Find room with code : " << token << '\n';

        for (int i = 0; i < MAX_CLIENT; i++)
        {
          if (roomList[i].isRoomOccupied())
          {
            if (roomList[i].getRoomCode() == token)
            {
              foundRoom = true;
              if (roomList[i].isRoomFull())
              {
                roomFull = true;
                break;
              }
              std::getline(ss, token, '\n');
              std::cout << "Player : " << token << " is joing room\n";
              roomIndex = i;
              roomList[i].setGuest(threadId, token);
              code.assign(JOIN_ROOM_CODE);
              message = code + '\n' + "1\n" + roomList[i].getOwnerName() + '\n';
              send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
              std::cout << "Room " << i << " with code " << roomList[i].getRoomCode() << " has guest joined\n";
              break;
            }
          }
        }
        if (!foundRoom)
        {
          code.assign(JOIN_ROOM_CODE);
          message = code + '\n' + "0\n";
          send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
          std::cout << "Room with code " << token << " can not be found\n";
        }
        else if (roomFull)
        {
          code.assign(JOIN_ROOM_CODE);
          message = code + '\n' + "2\n";
          send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
          std::cout << "Room with code " << token << " is full\n";
        }
      }
      else if (!token.compare(START_GAME_CODE)){
        if (roomIndex < 0){
          std::cout << "Player not in a room but send start game request. Ignoring...\n";
          continue;
        }
        code.assign(START_GAME_CODE);
        int newInt = coin_flip();
        std::cout << "coin: " << newInt << '\n';
        std::string playerSide = newInt ? "white" : "black";
        message = code + '\n' + playerSide + '\n';
        roomList[roomIndex].setGameStarting(true);
        roomList[roomIndex].setOwnerSide(newInt);
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
      }
      else if (!token.compare(MOVE_CODE)){
        int moveFrom, moveTo;
        std::getline(ss, token, '\n');
        std::cout << token << '\n';
        moveFrom = std::stoi(token);
        std::getline(ss, token, '\n');
        std::cout << token << '\n';
        moveTo = std::stoi(token);

        if(roomList[roomIndex].isOwner(threadId))
          roomList[roomIndex].setOwnerMove(moveFrom, moveTo);
        else
          roomList[roomIndex].setGuestMove(moveFrom, moveTo);
      }
      else if (!token.compare(READY_CODE)){
        std::getline(ss, token, '\n');
        std::cout << token << '\n';
        roomList[roomIndex].setGuestReady(std::stoi(token));
        roomList[roomIndex].setNewGuestReady(true);

        code.assign(READY_CODE);
        message = code + '\n' + token + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
      }
      else if (!token.compare(RESIGN_CODE)){
        if (roomList[roomIndex].isGuest(threadId))
          roomList[roomIndex].setGuestResign(true);
        else
          roomList[roomIndex].setOwnerResign(true);
      }
      else if (!token.compare(LOGIN_CODE)){
        std::string username, password;
        std::getline(ss, token, '\n');
        std::cout << token << '\n';
        username = token;
        std::getline(ss, token, '\n');
        std::cout << token << '\n';
        password = token;
        //Find account logic here
        Account::Status status;
        std::string statusMessage;
        for (auto& account : accountList) {
          status = account.second.attempLogin(username, password);
          if (status != Account::Status::logged_out)
            break;
        }
        code.assign(LOGIN_CODE);
        message = code + "\n" + Account::Stringify(status) + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
      }  
    }
    else if (bytes_received == 0)
    {
      std::cout << "Connection closed by the client." << std::endl;
      break;
    }
    else
    {
      int error = WSAGetLastError();
      if (error != WSAEWOULDBLOCK)
      {
        std::cerr << "Failed to receive data. Error code: " << error << std::endl;
        break;
      }
      // No data available in the receive buffer
      // Do other work or sleep for a while before calling recv() again
    }

    if (roomIndex >= 0)
    {
      if (roomList[roomIndex].getNewOwnerState())
      {
        roomList[roomIndex].setNewOwnerState(false);
        // Send onwer state to client
        code.assign(NEW_OWNER_CODE);
        message = code + '\n' + roomList[roomIndex].getOwnerName() + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
      }
      if (roomList[roomIndex].getNewGuestState())
      {
        roomList[roomIndex].setNewGuestState(false);
        // Send guest state to client
        code.assign(NEW_GUEST_CODE);
        message = code + '\n' + roomList[roomIndex].getGuestName() + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
      }
      if (roomList[roomIndex].getGameStarting() && roomList[roomIndex].isGuest(threadId)){
        roomList[roomIndex].setGameStarting(false);
        code.assign(START_GAME_CODE);
        message = code + '\n' + (roomList[roomIndex].getOwnerSide() ? "black" : "white") + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
      }

      if (roomList[roomIndex].availableOwnerMove() && roomList[roomIndex].isGuest(threadId)){
        code.assign(MOVE_CODE);
        message = code + '\n' + std::to_string(roomList[roomIndex].getOwnerMoveFrom()) + '\n' + std::to_string(roomList[roomIndex].getOwnerMoveTo()) + '\n';
        roomList[roomIndex].setOwnerMove(-1, -1);
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
      }
      if (roomList[roomIndex].availableGuestMove() && roomList[roomIndex].isOwner(threadId)){
        code.assign(MOVE_CODE);
        message = code + '\n' + std::to_string(roomList[roomIndex].getGuestMoveFrom()) + '\n' + std::to_string(roomList[roomIndex].getGuestMoveTo()) + '\n';
        roomList[roomIndex].setGuestMove(-1, -1);
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
      }
      if (roomList[roomIndex].getNewGuestReady() && roomList[roomIndex].isOwner(threadId)){
        code.assign(READY_CODE);
        message = code + '\n' + (roomList[roomIndex].getGuestReady() ? std::to_string(1) : std::to_string(0)) + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
        roomList[roomIndex].setNewGuestReady(false);
      }
      if (roomList[roomIndex].getGuestResign() && roomList[roomIndex].isOwner(threadId)){
        code.assign(RESIGN_CODE);
        message = code + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
        roomList[roomIndex].setGuestResign(false);
      }
      if (roomList[roomIndex].getOwnerResign() && roomList[roomIndex].isGuest(threadId)){
        code.assign(RESIGN_CODE);
        message = code + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
        roomList[roomIndex].setOwnerResign(false);
      }
    }
  }

  std::cout << "Closing client socket..";
  closesocket(client_socket);
}

std::map<std::string, Account> initializeAccountList(){
  std::map<std::string, Account> accountList;
  std::ifstream inputFile("data.txt");

  if (!inputFile) {
    std::cout << "Error opening the file." << std::endl;
    exit(1);
  }

  std::string line;
  std::string username;
  std::string password;
  std::string status;
  int lineCount = 0;
  while (std::getline(inputFile, line)) {
    switch (lineCount % 3) {
      case 0:
          username = line;
          break;
      case 1:
          password = line;
          break;
      case 2:
          status = line;
          accountList.emplace(username, Account(username, password, status));
          break;
    }
    lineCount++;
  }

  return accountList;
}

// void displayAccountList(const std::map<std::string, Account>& accountList) {
//     for (const auto& account : accountList) {
//         std::cout << "Username: " << account.second.username << std::endl;
//         std::cout << "Password: " << account.second.password << std::endl;
//         std::cout << "Status: " << account.second.status << std::endl;
//         std::cout << std::endl;
//     }
// }
