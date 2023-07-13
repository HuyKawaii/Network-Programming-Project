#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "button.h"

class InputField : public Button{
public:
  InputField();
  ~InputField(){};

  bool handleEvent (SDL_Event* e, int& sound) override;
  //ACCESSOR
  bool getIsEditing() const { return isEditing; };

private:
  void setInputFieldEditing(std::string defaultValue);
  void setInputFieldNotEditing();
  bool isEditing;
};