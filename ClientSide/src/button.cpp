/*
----------------------------------
	~Moosey Chess Engine~
	      button.cpp
----------------------------------
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "button.h"
#include "board.h"
#include "ltexture.h"
#include "display.h"
#include "common.h"
#include "clientSocket.h"

Button::Button() {
	pos.x = pos.y = 0;
	w = h = 0;
	inside = clicking = isEditing = false;
	butt = 0;
	boardPtr = NULL;
}

void Button::setPos(int x, int y) {
	pos.x = x;
	pos.y = y;
}

void Button::setSize(int x, int y) {
	setW(x);
	setH(y);
}

bool Button::handleEvent(SDL_Event* e, int& sound) {
    //isTyping = false;

		int x, y;
		SDL_GetMouseState(&x, &y);
		inside = true;
		if (x < pos.x) 
			inside = false;
		else if (x > pos.x + w)
			inside = false;
		else if (y < pos.y)
			inside = false;
		else if (y > pos.y + h)
			inside = false;
		if (inside) {
			switch (e->type) { 
					case SDL_MOUSEBUTTONDOWN:
					clicking = true;
					if (!muted) {
						if (butt == 25 && !start)
							sound = 2;
						else {
							if ((butt > 1 && !start) || butt < 2)
								sound = 1;
						}
					}
					//Restart button
					if (butt == 0) {
						boardPtr->restart();
						return true;
					}
					//Undo button
					else if (butt == 1) {
						boardPtr->undoMove();
						return true;
					}
					//White, "Human"
					else if (butt == 2){
						if (!start && displayPtr->getMenu() == Display::Menu::offlineMenu)
							boardPtr->setWhiteIsBot(false);
					}
					//White, "Computer"
					else if (butt == 3){
						if (!start && displayPtr->getMenu() == Display::Menu::offlineMenu)
							boardPtr->setWhiteIsBot(true);
					}
					//Black, "Human"
					else if (butt == 4){
						if (!start && displayPtr->getMenu() == Display::Menu::offlineMenu)
							boardPtr->setBlackIsBot(false);
					}
					//Black, "Computer"
					else if (butt == 5){
						if (!start && displayPtr->getMenu() == Display::Menu::offlineMenu)
							boardPtr->setBlackIsBot(true);
					}
					//White computer levels 1-9
					else if (butt >= 6 && butt <= 14){
						if (boardPtr->getWhiteIsBot() && !start && displayPtr->getMenu() == Display::Menu::offlineMenu)
							boardPtr->whiteBot.setLevel(butt-5);
					}
					//Black computer levels 1-9
					else if (butt >= 15 && butt <= 23){
						if (boardPtr->getBlackIsBot() && !start && displayPtr->getMenu() == Display::Menu::offlineMenu)
							boardPtr->blackBot.setLevel(butt-14);
					}
					//Flip board
					else if (butt == 24){
						if (!start) 
							boardPtr->flipBoard();
					}
					//Start
					else if (butt == 25){
						if (boardPtr->getGamemode() == Board::gamemode::offline)
							start = true;
						else if (socketPtr->getIsOwner())
							socketPtr->sendStartGame();
					}
          else if (butt == 26){
						if (displayPtr->getMenu() == Display::Menu::mainMenu){
							displayPtr->setMenu(Display::Menu::onlineMenu);
							boardPtr->setGamemode(Board::gamemode::online);
						}
					}
          else if (butt == 27){
						if (displayPtr->getMenu() == Display::Menu::mainMenu){
							displayPtr->setMenu(Display::Menu::offlineMenu);
							boardPtr->setGamemode(Board::gamemode::offline);
						}
					}
          else if (butt == 28){
						if (displayPtr->getMenu() == Display::Menu::roomMenu)
							socketPtr->sendLeaveRoom();
						if (displayPtr->getMenu() != Display::Menu::mainMenu){
            	displayPtr->setMenu(Display::Menu::mainMenu);
							boardPtr->setGamemode(Board::gamemode::nomode);
						}
					}
          else if (butt == 29){
						if (displayPtr->getMenu() == Display::Menu::onlineMenu){
							displayPtr->setMenu(Display::Menu::roomMenu);
							socketPtr->sendCreateRoom();
						}
					}
          else if (butt == 30){
						if (displayPtr->getMenu() == Display::Menu::onlineMenu){
							displayPtr->setMenu(Display::Menu::joinRoomMenu);
							socketPtr->setRoomFoundState(false);
						}
					}
          else if (butt == 31){
						if (displayPtr->getMenu() == Display::Menu::joinRoomMenu){
							setButtonEditing();
						}
					}
					else if (butt == 32){
						if (displayPtr->getMenu() == Display::Menu::joinRoomMenu){
							socketPtr->sendJoinRoom();
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
            setButtonNotEditing();
          }
          break;
					case SDL_MOUSEBUTTONUP:
					clicking = false;
					break;
      }
    }
	  return false;
}

void Button::ShowButton(SDL_Renderer* renderer){
  buttonTexture.loadFromRenderedText(renderer, buttonText, buttonTextColor, buttonTextFont);
  buttonTexture.render(renderer, pos.x, pos.y);
}

void Button::setButton(std::string newText, TTF_Font* newFont, SDL_Color newColor){
  setButtonText(newText);
  setButtonTextFont(newFont);
  setButtonTextColor(newColor);
}

void Button::setButtonEditing(){
  isEditing = true;
  inputText.assign(buttonText);
}

void Button::setButtonNotEditing(){
  isEditing = false;
}