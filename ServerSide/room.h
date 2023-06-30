#include <string>
#include <thread>

class Room{
public:
  Room();
  ~Room();

  //ACCESSOR
  std::string getRoomCode() { return roomCode; };
  bool isRoomOccupied() { return isOccupied; };
  bool isOwner(std::thread::id player) { return player == owner; };
  bool isGuest(std::thread::id player) { return player == guest; };
  bool isRoomFull() { return guest != std::thread::id(); };
  bool getNewGuestState() { return newGuest; };
  bool getNewOwnerState() { return newOwner; };
  std::string getOwnerName() { return ownerName; };
  std::string getGuestName() { return guestName; };

  //MUTATOR
  void setRoomOccupied(bool b) { isOccupied = b; };
  void setRoomCode(std::string roomCode) { this->roomCode = roomCode; };
  void setOwner(std::thread::id owner, std::string ownerName);
  void setGuest(std::thread::id guest, std::string guestName);
  void setNewGuestState(bool state) { newGuest = state; };
  void setNewOwnerState(bool state) { newOwner = state; };
  void removeOwner();
  void removeGuest();

private:
  std::string roomCode;
  std::thread::id owner;
  std::string ownerName = "";
  std::thread::id guest;
  std::string guestName = "";
  bool isOccupied;
  bool newGuest, newOwner;
};