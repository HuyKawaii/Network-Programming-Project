#include <iostream>
#include <string>
#include <winsock.h>

#define RECEIVE_BUFFER_SIZE 32

#define JOIN_ROOM_CODE "JOIN"
#define CREATE_ROOM_CODE "CREATE"
#define LEAVE_ROOM_CODE "LEAVE"
#define OPPONENT_LEAVE_ROOM_CODE "OPPOLEAVE"
#define START_GAME_CODE "START"
#define NEW_GUEST_CODE "NEWGUEST"
#define NEW_OWNER_CODE "NEWOWNER"
#define MOVE_CODE "MOVE"
#define READY_CODE "READY"

class Display;
class Board;

class ClientSocket
{
public:
  ClientSocket(std::string server_ip, unsigned short server_port);
  SOCKET GetClientSocket() { return so; };

  void error(std::string message);
  void close();
  void handleBufferRead();

  //ACCESSOR
  std::string getRoomCode() { return roomCode; };
  std::string getPlayerName() { return name; };
  std::string getOpponentName() { return opponentName; };
  bool getRoomFoundState() { return roomNotFound; };
  bool getRoomFull() { return roomFull; };
  bool getIsOwner() { return isOwner; };
  bool getIsGuestReady() { return isGuestReady; };
  //MUTATOR
  void setRoomCode(std::string newRoomCode) { roomCode = newRoomCode; };
  void setDisplayPtr(Display * display);
  void setBoardPtr(Board * board);
  void setRoomFoundState(bool b) { roomNotFound = b; };
  void setGuestReady(bool isGuestReady) { this->isGuestReady = isGuestReady; };

  void sendJoinRoom();
  void handleJoinRoom(std::string reply, std::string opponentName);
  void sendCreateRoom();
  void handleCreateRoom(std::string roomCode);
  void sendLeaveRoom();
  void handleLeaveRoom();
  void sendStartGame();
  void handleStartGame(std::string side);
  void sendMoveSignal(int moveFrom, int moveTo);
  void handleMoveSignal(int moveFrom, int moveTo);
  void sendReadySignal();
  void handleReadySignal(int readyStatus);
  void handleNewGuest(std::string newGuestName);
  void handleNewOwner(std::string newGuestName);
  bool isRoomFull() { return opponentName != ""; };

private:
  SOCKET so;
  std::string roomCode = "";
  std::string name = "";
  std::string opponentName = "";
  bool roomNotFound;
  bool roomFull;
  bool isOwner;
  bool isGuestReady;
  Display* displayPtr;
  Board* boardPtr;
};
