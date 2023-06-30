#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "button.h"

class InputField : public Button{
public:
  InputField();
  //ACCESSOR
  bool getSelected() { return isSelected; };

  //MUTATOR
  void unselectField() { isSelected = false; };
  void selectField() { isSelected = true; };
private:
  bool isSelected;
};