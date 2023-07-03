/*
----------------------------------
	~Moosey Chess Engine~
	      display.h
----------------------------------
*/

#ifndef DISPLAY_H
#define DISPLAY_H

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include "button.h"
#include "ltexture.h"
#include "square.h"

#define NUMBER_OF_BUTTOMS 34
#define ON_HOVER_COLOR {100, 0, 0}
#define NUMBER_OF_INPUTFIELDS 1
class ClientSocket;

class Display {
	friend class Board;
	public:
    enum Menu { mainMenu, onlineMenu, offlineMenu, roomMenu, joinRoomMenu};
		Display(Board * b, ClientSocket * socket);
		~Display();
	
		void displayBoard(const int& mF = -1, const int& mT = -1);
		void setButtons();
		void setSpriteClips();
		void displayBotText();
    void displayRoomText();
		void updateText();
		void drawButtons();
		void drawSquares(const int& mF = -1, const int& mT = -1);
		void drawPieces(const int& mF = -1, const int& mT = -1);
		void drawBorder();
		void drawMoveTable();
		void drawTitleScreen();
		void handleButtons(SDL_Event* e);
    void setMenu(Menu menu) {this->menu = menu; };
    void displayDefaultButtom(Button * button);
    void displayInputField(Button * button);
		void setPlayerSideText();
		bool init_SDL();
		bool loadMedia();
		void close_SDL();
		SDL_Texture* loadTexture(std::string path);

		//ACCESSORS
		int getBoardXStart() { return BXSTART; };
		int getBoardYStart() { return BYSTART; };
		int getSqSize() { return SQ_SIZE; };
		int getBoardSize() { return B_SIZE; };
		Menu getMenu() { return menu; };
		Square squares[64];

	private:
		Board * boardPtr;
    ClientSocket * socketPtr;
    Menu menu;
		bool sideFlag;
		const static int B_SIZE = 600, SQ_SIZE = 75, BXSTART = 50, BYSTART = 25;
		const static int SCREEN_W = 1250;
		const static int SCREEN_H = 700;
		int frameCounter;
		const static int FRAME_REFRESH_RATE = 8;
		SDL_Window* window;
		SDL_Renderer* renderer;
		TTF_Font* Garamond26, *Garamond28, *Cicero22, *Cicero26;
		Mix_Chunk* mTSound, *mFSound;
		SDL_Rect spriteClips[12], buttonClips[6], titleTextClips[28];
		Button buttons[NUMBER_OF_BUTTOMS];
		LTexture spriteSheetTexture, buttonTexture, titleTexture,
			 titleTextTexture, turnText, checkText, moveText,
			 rankText, fileText, onlineModeText, playerNameText, opponentNameText, 
       roomCodeText, roomNotFoundText, waitForServerText, roomIsFullText, 
			 playerSideText, guestReadyText;
		SDL_Color textColor;
		std::string rankStr, fileStr;
};

//extern Mix_Chunk * mFSound, * mTSound;

#endif
