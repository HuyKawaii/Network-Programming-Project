#include "inputField.h"
#include "display.h"
#include "clientSocket.h"

InputField::InputField(){
  Button();
  isEditing = false;
}

bool InputField::handleEvent(SDL_Event* e, int& sound){
  if (Button::handleEvent(e, sound))
    return true;
  
  if (inside) {
			switch (e->type) { 
					case SDL_MOUSEBUTTONDOWN:
					clicking = true;
					
          if (butt == 31){
						if (displayPtr->getMenu() == Display::Menu::joinRoomMenu){
							setInputFieldEditing(ROOM_CODE_PLACEHOLDER);
						}
					}
					else if (butt == 34){
						if (displayPtr->getMenu() == Display::Menu::loginMenu){
							setInputFieldEditing(USERNAME_PLACEHOLDER);
						}
					}
					else if (butt == 35){
						if (displayPtr->getMenu() == Display::Menu::loginMenu){
							setInputFieldEditing(PASSWORD_PLACEHOLDER);
						}
					}
					break;
					case SDL_MOUSEBUTTONUP:
					clicking = false;
					break;
			}
  }else{
    switch (e->type) { 
        case SDL_MOUSEBUTTONDOWN:
        clicking = true;

        if(butt == 31){
          setInputFieldNotEditing();
        }
        if(butt == 34){
          setInputFieldNotEditing();
        }
        if(butt == 35){
          setInputFieldNotEditing();
        }
        break;
        case SDL_MOUSEBUTTONUP:
        clicking = false;
        break;
    }
  }

  return false;
}

void InputField::setInputFieldEditing(std::string defaultValue){
	if (defaultValue == buttonText)
		buttonText = "";
  isEditing = true;
  inputText.assign(buttonText);
}

void InputField::setInputFieldNotEditing(){
  isEditing = false;
}
