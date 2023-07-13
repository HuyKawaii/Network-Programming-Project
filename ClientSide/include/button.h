/*
----------------------------------
	~Moosey Chess Engine~
	      button.h
----------------------------------
*/

#ifndef BUTTON_H
#define BUTTON_H

#include <SDL2/SDL.h>
#include <ltexture.h>
#include <string>
#include "common.h"

#define ROOM_CODE_PLACEHOLDER "Enter code here"
#define USERNAME_PLACEHOLDER "Username"
#define PASSWORD_PLACEHOLDER "Password"

class Board;
class Display;
class ClientSocket;

class Button {
	public:
		Button();

		bool handleEvent(SDL_Event* e, int& sound);
    void ShowButton(SDL_Renderer* renderer);
		//ACCESSORS
		int getX() const { return pos.x; };
		int getY() const { return pos.y; };
		int getW() const { return w; };
		int getH() const { return h; };
		int getButt() { return butt; };
		bool getInside() const { return inside; };
		bool getClicking() const { return clicking; };
    bool getIsEditing() const { return isEditing; };
    std::string getButtonText() const { return buttonText; };
		//MUTATORS
		void setInside(bool b) { inside = b; };
		void setClicking(bool b) { clicking = b; };
		void setButt(int b) { butt = b; };
		void setPos(int x, int y);
		void setW(int x) { w = x > 0 ? x : 0; };
		void setH(int x) { h = x > 0 ? x : 0; };
		void setSize(int x, int y);
		void setBoardPtr(Board* b) { boardPtr = b; };
    void setDisplayPtr(Display* p) { displayPtr = p; };
    void setSocketPtr(ClientSocket* socket) { socketPtr = socket; };
    void setButtonText(std::string newText) { buttonText.assign(newText); };
    void setButtonTextColor(SDL_Color newColor) { buttonTextColor = newColor; };
    void setButtonTextFont(TTF_Font* newFont) {buttonTextFont = newFont; };
    void setButton(std::string newText, TTF_Font* newFont, SDL_Color newColor);
    void setButtonEditing(std::string defaultValue);
    void setButtonNotEditing();
	private:
		Board * boardPtr;
    Display * displayPtr;
    ClientSocket * socketPtr;
		int butt;	//Which button it is
		int w, h;
		bool inside, clicking;
		SDL_Point pos;
    std::string buttonText;
    SDL_Color buttonTextColor;
    LTexture buttonTexture;
    TTF_Font* buttonTextFont;
    bool isEditing;
};

#endif
