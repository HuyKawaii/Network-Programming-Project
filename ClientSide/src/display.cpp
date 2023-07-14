/*
----------------------------------
	~Moosey Chess Engine~
				display.cpp
----------------------------------
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>
#include <vector>
#include "board.h"
#include "common.h"
#include "inputField.h"
#include "clientSocket.h"
#include "display.h"

Display::Display(Board *b, ClientSocket *socket) : boardPtr(b), socketPtr(socket)
{
	socket->setDisplayPtr(this);
	menu = loginMenu;
	textColor = {0, 0, 0};
	sideFlag = !boardPtr->getSide();

	window = NULL;
	renderer = NULL;
	Garamond26 = Garamond28 = Cicero22 = Cicero26 = NULL;
	mTSound = mFSound = NULL;
	sideFlag = !boardPtr->getSide();
	init_SDL();
	loadMedia();
	frameCounter = FRAME_REFRESH_RATE;
}

bool Display::init_SDL()
{
	bool success = true;
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		// Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
			printf("Warning: Linear texture filtering not enabled!");

		// Create window
		window = SDL_CreateWindow("Moosey Chess Engine", SDL_WINDOWPOS_UNDEFINED,
															SDL_WINDOWPOS_UNDEFINED, SCREEN_W, SCREEN_H,
															SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

		if (window == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			// Create vsynced renderer for window
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (renderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				// Initialize renderer color
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

				// Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}

				// Initialize SDL_ttf
				if (TTF_Init() == -1)
				{
					printf("SDL_ttf could not be initialized.\n");
					success = false;
				}

				if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
				{
					printf("SDL_mixer could not be initialized.\n");
					success = false;
				}
			}
		}
	}

	return success;
}

bool Display::loadMedia()
{
	bool success = true;
	Garamond28 = TTF_OpenFont("../res/Garamond.ttf", 28);
	Cicero22 = TTF_OpenFont("../res/Cicero.ttf", 22);
	Cicero26 = TTF_OpenFont("../res/Cicero.ttf", 26);
	Garamond26 = TTF_OpenFont("../res/Garamond.ttf", 26);

	if (Garamond28 == NULL || Garamond26 == NULL)
		success = false;
	if (Cicero26 == NULL || Cicero22 == NULL)
		success = false;

	mFSound = Mix_LoadWAV("../res/moveFrom.wav");
	mTSound = Mix_LoadWAV("../res/moveTo.wav");
	if (mFSound == NULL || mTSound == NULL)
		success = false;

	spriteSheetTexture.loadFromFile(renderer, "../res/spritesheet2.bmp");
	buttonTexture.loadFromFile(renderer, "../res/buttons.bmp");
	titleTexture.loadFromFile(renderer, "../res/mooseytitle.bmp");
	titleTextTexture.loadFromFile(renderer, "../res/titletext.bmp");
	if (spriteSheetTexture.texture == NULL || buttonTexture.texture == NULL)
		success = false;
	if (titleTexture.texture == NULL || titleTextTexture.texture == NULL)
		success = false;

	SDL_Surface *icon = IMG_Load("../res/icon.png");
	SDL_SetWindowIcon(window, icon);

	return success;
}

void Display::close_SDL()
{
	// Free textures
	spriteSheetTexture.free();
	buttonTexture.free();
	titleTexture.free();
	titleTextTexture.free();

	// Free sounds
	Mix_FreeChunk(mFSound);
	Mix_FreeChunk(mTSound);
	mFSound = NULL;
	mTSound = NULL;

	// Free global font
	TTF_CloseFont(Garamond26);
	TTF_CloseFont(Garamond28);
	TTF_CloseFont(Cicero22);
	TTF_CloseFont(Cicero26);
	Garamond26 = NULL;
	Garamond28 = NULL;
	Cicero22 = NULL;
	Cicero26 = NULL;

	// Destroy window
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;

	// Quit SDL subsystems
	Mix_Quit();
	TTF_Quit();
}

Display::~Display()
{
	close_SDL();
}

void Display::displayBoard(const int &mF, const int &mT)
{
	using std::string;

	if (!boardPtr->getFlipped())
		fileStr = "a         b        c         d         e         f         g         h";
	else
		fileStr = "h         g        f         e         d         c         b         a";

	// Clear screen
	SDL_SetRenderDrawColor(renderer, 209, 224, 255, 255);
	SDL_RenderClear(renderer);

	boardPtr->setPiecesOnSquares(); // Update piece positions
	drawSquares(mF, mT);						// Draw the squares
	drawPieces(mF, mT);							// Draw pieces on squares
	drawBorder();										// Draw border around board

	drawButtons(); // Draw buttons (undo, restart)
	updateText();	 // Updates text if someone moved
	if (start)
		drawMoveTable(); // Draw movetable (with text)
	else
		drawTitleScreen(); // Draw title screen

	// Draw rank numbers
	if (!boardPtr->getFlipped())
		for (char i = '8'; i >= '1'; i--)
		{
			rankStr = i;
			rankText.loadFromRenderedText(renderer, rankStr, textColor, Cicero26);
			rankText.render(renderer, BXSTART - 35, BYSTART + 30 + 75 * ('8' - i));
		}
	else
		for (char i = '1'; i <= '8'; i++)
		{
			rankStr = i;
			rankText.loadFromRenderedText(renderer, rankStr, textColor, Cicero26);
			rankText.render(renderer, BXSTART - 35, BYSTART + 30 + 75 * (i - '1'));
		}

	fileText.loadFromRenderedText(renderer, fileStr, textColor, Cicero26); // Load file text

	// Render all the rest of the text
	checkText.render(renderer, BXSTART + B_SIZE - 200, BYSTART + B_SIZE + 40);
	fileText.render(renderer, BXSTART + 33, BYSTART + B_SIZE + 10);
	playerSideText.render(renderer, BXSTART + 100, BYSTART + B_SIZE + 40);
	loginStatusText.render(renderer, BXSTART + B_SIZE + 200, BYSTART + B_SIZE + 40);
	// Update screen
	if (frameCounter > FRAME_REFRESH_RATE)
	{
		SDL_RenderPresent(renderer);
		frameCounter = 0;
	}
	frameCounter++;
}

void Display::setButtons()
{
	for (int i = 0; i < NUMBER_OF_BUTTOMS; i++)
		buttons[i] = new Button();

	// Restart and undo
	buttons[0]->setPos(1185, 25);
	buttons[0]->setSize(51, 31);
	
	buttons[1]->setPos(buttons[0]->getX(), buttons[0]->getY() + 50);
	buttons[1]->setSize(51, 31);

	// Human, computer (white and black)
	for (int i = 2; i < 6; i++)
	{
		buttons[i]->setPos(BXSTART + B_SIZE + 115 + ((i - 2) % 2) * 200,
											BYSTART + 250 + (i / 4) * 150);
		buttons[i]->setSize(titleTextClips[i].w, titleTextClips[i].h);
	}
	buttons[2]->setButton("Human", Garamond28, textColor);
	buttons[3]->setButton("Computer", Garamond28, textColor);
	buttons[4]->setButton("Human", Garamond28, textColor);
	buttons[5]->setButton("Computer", Garamond28, textColor);

	// 1-9 (white and black)
	for (int i = 6; i < 24; i++)
	{
		buttons[i]->setPos(BXSTART + B_SIZE + 275 + (i - 6) % 9 * titleTextClips[i].w,
											BYSTART + 300 + (i / 15) * 150);
		buttons[i]->setSize(titleTextClips[i].w, titleTextClips[i].h);
		buttons[i]->setButton(std::to_string((i + 5) % 10), Garamond28, textColor);
	}

	// Flip board
	buttons[24]->setPos(BXSTART + B_SIZE + 199, BYSTART + 520);
	buttons[24]->setSize(titleTextClips[24].w, titleTextClips[24].h);
	buttons[24]->setButton("Flip board", Garamond28, textColor);
	// Start, and light version
	buttons[25]->setPos(BXSTART + B_SIZE + 229, BYSTART + 590);
	buttons[25]->setSize(titleTextClips[25].w, titleTextClips[25].h);
	buttons[25]->setButton("Start", Garamond28, textColor);

	// Online
	buttons[26]->setPos(BXSTART + B_SIZE + 200, BYSTART + 400);
	buttons[26]->setSize(150, 50);
	buttons[26]->setButton("Online Mode", Garamond28, textColor);
	// Offline
	buttons[27]->setPos(BXSTART + B_SIZE + 200, BYSTART + 300);
	buttons[27]->setSize(150, 50);
	buttons[27]->setButton("Offline Mode", Garamond28, textColor);

	// BackToMain
	buttons[28]->setPos(BXSTART + B_SIZE + 380, BYSTART + 180);
	buttons[28]->setSize(150, 50);
	buttons[28]->setButton("Mainmenu", Garamond28, textColor);

	// CreateRoom
	buttons[29]->setPos(BXSTART + B_SIZE + 110, BYSTART + 350);
	buttons[29]->setSize(150, 50);
	buttons[29]->setButton("Create room", Garamond28, textColor);
	// Joinroom
	buttons[30]->setPos(BXSTART + B_SIZE + 330, BYSTART + 350);
	buttons[30]->setSize(150, 50);
	buttons[30]->setButton("Join room", Garamond28, textColor);

	// Roomcode input field
	buttons[31]	= new InputField();
	buttons[31]->setPos(BXSTART + B_SIZE + 180, BYSTART + 400);
	buttons[31]->setSize(150, 50);
	buttons[31]->setButton(ROOM_CODE_PLACEHOLDER, Garamond28, textColor);

	buttons[32]->setPos(BXSTART + B_SIZE + 230, BYSTART + 450);
	buttons[32]->setSize(150, 50);
	buttons[32]->setButton("Join", Garamond28, textColor);

	buttons[33]->setPos(BXSTART + B_SIZE + 229, BYSTART + 590);
	buttons[33]->setSize(titleTextClips[25].w, titleTextClips[25].h);
	buttons[33]->setButton("Ready", Garamond28, textColor);

	buttons[34]	= new InputField();
	buttons[34]->setPos(BXSTART + B_SIZE + 229, BYSTART + 400);
	buttons[34]->setSize(titleTextClips[25].w, titleTextClips[25].h);
	buttons[34]->setButton(USERNAME_PLACEHOLDER, Garamond28, textColor);

	buttons[35]	= new InputField();
	buttons[35]->setPos(BXSTART + B_SIZE + 229, BYSTART + 500);
	buttons[35]->setSize(titleTextClips[25].w, titleTextClips[25].h);
	buttons[35]->setButton(PASSWORD_PLACEHOLDER, Garamond28, textColor);

	buttons[36]->setPos(BXSTART + B_SIZE + 240, BYSTART + 590);
	buttons[36]->setSize(titleTextClips[25].w, titleTextClips[25].h);
	buttons[36]->setButton("Sign in", Garamond28, textColor);

	for (int i = 0; i < NUMBER_OF_BUTTOMS; i++){
		buttons[i]->setButt(i);
		buttons[i]->setBoardPtr(boardPtr);
		buttons[i]->setDisplayPtr(this);
		buttons[i]->setSocketPtr(socketPtr);
	}
}

void Display::setSpriteClips()
{
	for (int i = 0; i < 12; i++)
	{
		spriteClips[i].x = i % 6 * SQ_SIZE;
		spriteClips[i].y = i / 6 * SQ_SIZE;
		spriteClips[i].w = SQ_SIZE;
		spriteClips[i].h = SQ_SIZE;
	}
	for (int i = 0; i < 6; i++)
	{
		buttonClips[i].x = i % 2 * 51;
		buttonClips[i].y = i / 2 * 31;
		buttonClips[i].w = 51;
		buttonClips[i].h = 31;
	}
	//"White:", "Black:"
	for (int i = 0; i < 2; i++)
	{
		titleTextClips[i].x = 0 + i * 100;
		titleTextClips[i].y = 0;
		titleTextClips[i].w = 100;
		titleTextClips[i].h = 40;
	}
	//"Human", "Computer", and light versions
	for (int i = 2; i < 6; i++)
	{
		titleTextClips[i].x = 0 + (i % 2) * 100;
		titleTextClips[i].y = 45 + (i / 4) * 40;
		titleTextClips[i].w = 100 + (i % 2) * 40;
		titleTextClips[i].h = 40;
	}
	// 1-9, and light versions
	for (int i = 6; i < 24; i++)
	{
		titleTextClips[i].x = 0 + 25 * ((i - 6) % 9);
		titleTextClips[i].y = 130 + 33 * (i / 15);
		titleTextClips[i].w = 25;
		titleTextClips[i].h = 30;
	}
	//"Flip board"
	titleTextClips[24].x = 0;
	titleTextClips[24].y = 200;
	titleTextClips[24].w = 150;
	titleTextClips[24].h = 50;
	//"Flip board" light
	titleTextClips[27].x = 0;
	titleTextClips[27].y = 294;
	titleTextClips[27].w = 150;
	titleTextClips[27].h = 50;

	//"Start"
	titleTextClips[25].x = 10;
	titleTextClips[25].y = 250;
	titleTextClips[25].w = 90;
	titleTextClips[25].h = 40;
	//"Start" light
	titleTextClips[26].x = 125;
	titleTextClips[26].y = 250;
	titleTextClips[26].w = 90;
	titleTextClips[26].h = 40;
}

void Display::displayBotText()
{
	std::string botStr;

	botStr = boardPtr->getSide() ? "White" : "Black";
	botStr += " is thinking..";
	turnText.loadFromRenderedText(renderer, botStr, textColor, Garamond26);
	turnText.render(renderer, BXSTART, BYSTART + B_SIZE + 40);
	SDL_RenderPresent(renderer);
}

void Display::displayRoomText()
{
}

void Display::updateText()
{
	std::string checkStr = "";
	std::string playerSideStr = "";
	std::string loginStatusStr = "";

	if (boardPtr->getWinner() == 2)
		checkStr = "Black wins!";
	else if (boardPtr->getWinner() == 1)
		checkStr = "White wins!";
	else if (boardPtr->getNumMovesMade() == 0 || sideFlag != boardPtr->getSide())
	{
		sideFlag = boardPtr->getSide();
		if (boardPtr->getSideInCheck())
		{ // Load check text
			if (boardPtr->getSideInCheck() == 1)
				checkStr = "White is in check";
			else if (boardPtr->getSideInCheck() == 2)
				checkStr = "Black is in check";
		}
	}

	if (start && boardPtr->getGamemode() == Board::gamemode::online)
	{
		playerSideStr = "You play as ";
		playerSideStr += (boardPtr->getPlayerSide() ? "white" : "black");
	}

	std::string status = socketPtr->getLoginStatus();
	if (status == "logged_in")
		loginStatusStr = "";
	else if (status == "logged_out")
		loginStatusStr = "Wrong account info";
	else if (status == "wrong_password")
		loginStatusStr = "Wrong password";
	else if (status == "blocked")
		loginStatusStr = "Account is blocked";

	playerSideText.loadFromRenderedText(renderer, playerSideStr, textColor, Garamond26);
	checkText.loadFromRenderedText(renderer, checkStr, textColor, Garamond26);
	loginStatusText.loadFromRenderedText(renderer, loginStatusStr, textColor, Garamond26);
}


void Display::drawButtons()
{
	SDL_Rect clipSq;
	for (int i = 0; i < 2; i++)
	{
		if (buttons[i]->getInside())
			clipSq = buttons[i]->getClicking() ? buttonClips[i + 4] : buttonClips[i + 2];
		else
			clipSq = buttonClips[i];
		buttonTexture.render(renderer, buttons[i]->getX(), buttons[i]->getY(), &clipSq);
	}
}

void Display::drawBorder()
{
	SDL_Rect borderRect = {BXSTART, BYSTART, B_SIZE, B_SIZE};
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(renderer, &borderRect);
	borderRect = {BXSTART - 1, BYSTART - 1, B_SIZE + 2, B_SIZE + 2};
	SDL_RenderDrawRect(renderer, &borderRect);
}

void Display::drawSquares(const int &mF, const int &mT)
{
	int sq;
	SDL_Rect sqPos;
	for (int r = 1; r <= 8; r++)
	{
		for (int f = 1; f <= 8; f++)
		{
			sq = FR2SQ64(f, r) - 1;
			// If this square is clicked, or has had a move made on it (moveFrom)
			if (mF == sq + 1 || to64(boardPtr->getLastMove() / 100) == sq + 1)
				SDL_SetRenderDrawColor(renderer, 248, 195, 248, 255);
			// If this square is clicked, or has had a move made on it (moveTo)
			else if (mT == sq + 1 || to64(boardPtr->getLastMove() % 100) == sq + 1)
				SDL_SetRenderDrawColor(renderer, 238, 157, 242, 255);
			// Otherwise, color light
			else if ((r + f) % 2 == 1)
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			// or color dark
			else
				SDL_SetRenderDrawColor(renderer, 0, 153, 153, 255);
			sqPos = {boardPtr->display.squares[sq].getX(), // X start
							 boardPtr->display.squares[sq].getY(), // Y start
							 SQ_SIZE, SQ_SIZE};										 // Width, height of square
			SDL_RenderFillRect(renderer, &sqPos);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		}
	}
}

void Display::drawPieces(const int &mF, const int &mT)
{
	boardPtr->setSquarePositions();

	int p, sq, x, y, putOnTop = -1;
	SDL_Rect sqPos;
	SDL_Rect clipSq, pOTClipSq;
	for (int r = 1; r <= 8; r++)
	{
		for (int f = 1; f <= 8; f++)
		{
			sq = FR2SQ64(f, r) - 1;
			sqPos = {boardPtr->display.squares[sq].getX(), // X start
							 boardPtr->display.squares[sq].getY(), // Y start
							 SQ_SIZE, SQ_SIZE};										 // Width, height of square

			p = boardPtr->display.squares[sq].getPiece();
			if (p == wqR || p == wkR)
				clipSq = spriteClips[wRook];
			else if (p == wqN || p == wkN)
				clipSq = spriteClips[wKnight];
			else if (p == wqB || p == wkB)
				clipSq = spriteClips[wBishop];
			else if (p == wQ)
				clipSq = spriteClips[wQueen];
			else if (p == wK)
				clipSq = spriteClips[wKing];
			else if (p >= wPa && p <= wPh)
			{
				if (boardPtr->piece[(*boardPtr)[from64(sq + 1)]].getValue() == P_VAL)
					clipSq = spriteClips[wPawn];
				else
					clipSq = spriteClips[wQueen];
			}
			if (p == bqR || p == bkR)
				clipSq = spriteClips[bRook];
			else if (p == bqN || p == bkN)
				clipSq = spriteClips[bKnight];
			else if (p == bqB || p == bkB)
				clipSq = spriteClips[bBishop];
			else if (p == bQ)
				clipSq = spriteClips[bQueen];
			else if (p == bK)
				clipSq = spriteClips[bKing];
			else if (p >= bPa && p <= bPh)
			{
				if (boardPtr->piece[(*boardPtr)[from64(sq + 1)]].getValue() == P_VAL)
					clipSq = spriteClips[bPawn];
				else
					clipSq = spriteClips[bQueen];
			}

			if (p != none)
			{
				// Save piece being dragged, to rerender on top
				if (boardPtr->display.squares[sq].getDragging())
				{
					putOnTop = sq;
					pOTClipSq = clipSq;
				}
				else // Every other piece
					spriteSheetTexture.render(renderer, sqPos.x, sqPos.y, &clipSq);
			}
		}
	}
	if (putOnTop != -1)
	{ // Rerender piece being dragged
		SDL_GetMouseState(&x, &y);
		if (x < BXSTART + SQ_SIZE / 2)
			x = BXSTART + SQ_SIZE / 2;
		if (x > BXSTART + B_SIZE - SQ_SIZE / 2)
			x = BXSTART + B_SIZE - SQ_SIZE / 2;
		if (y < BYSTART + SQ_SIZE / 2)
			y = BYSTART + SQ_SIZE / 2;
		if (y > BYSTART + B_SIZE - SQ_SIZE / 2)
			y = BYSTART + B_SIZE - SQ_SIZE / 2;
		spriteSheetTexture.render(renderer, x - SQ_SIZE / 2, y - SQ_SIZE / 2, &pOTClipSq);
	}
}

void Display::drawMoveTable()
{
	using std::string;
	using std::vector;
	static vector<string> plyStrings;
	vector<int> allOtherAliveQueens;
	int startPawn, notSameFile = 0, notSameRank = 0;
	int e = 0, smallE, big;
	string plyStr = "";
	int mF2, mT2, p, otherPiece;
	bool castling = false, dupMove = false;

	// Draw the border and background
	SDL_Rect borderRect = {BXSTART + B_SIZE + 25, BYSTART, 500, 650};
	SDL_SetRenderDrawColor(renderer, 236, 247, 235, 255);
	SDL_RenderFillRect(renderer, &borderRect);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(renderer, &borderRect);
	borderRect = {BXSTART + B_SIZE + 24, BYSTART - 1, 500, 650};
	SDL_RenderDrawRect(renderer, &borderRect);

	static int extra = 0;
	// Add "#. ... " if loaded from a FEN and black to move
	if (boardPtr->getNumMovesMade() == 0)
	{
		if (boardPtr->getSide() == BLACK)
		{
			plyStr = std::to_string((boardPtr->getPly() - 1) / 2 + 1) + ". ";
			plyStr += " ... ";
			plyStrings.push_back(plyStr);
			extra = 1;
		}
		else
			extra = 0;
	}

	while ((int)plyStrings.size() > boardPtr->getNumMovesMade() + extra)
		plyStrings.pop_back();

	int lastMove = boardPtr->getNumMovesMade() - 1 + extra;
	int oPos = 0;

	// If a new move has been made
	if ((int)plyStrings.size() < boardPtr->getNumMovesMade() + extra)
	{
		mF2 = boardPtr->getLastMove() / 100;
		mT2 = boardPtr->getLastMove() % 100;
		if ((boardPtr->getPly() - 1) % 2 == 0) // Add number in front for white's moves
			plyStr = std::to_string((boardPtr->getPly() - 1) / 2 + 1) + ". ";
		p = boardPtr->getPieceMoved(lastMove);

		if (boardPtr->piece[p].getValue() == Q_VAL &&
				boardPtr->getPmSq(lastMove) != mT2)
		{
			plyStr += "Q";

			// Check same side pawns, for ambiguous moves
			startPawn = !boardPtr->getSide() ? wPa : bPa;

			for (int i = startPawn; i <= startPawn + 8; i++)
			{
				if (!boardPtr->piece[i].getAlive())
					continue;
				if (i == p)
					continue;
				if (boardPtr->piece[i].getValue() == Q_VAL)
				{
					allOtherAliveQueens.push_back(i);
				}
			}
			// Add the real queen if necessary
			if (!boardPtr->getSide())
			{
				if (p > wQ && boardPtr->piece[wQ].getAlive())
				{
					allOtherAliveQueens.push_back(wQ);
				}
			}
			else
			{
				if (p > bQ && boardPtr->piece[bQ].getAlive())
				{
					allOtherAliveQueens.push_back(bQ);
				}
			}
			// Loop through other queens, checking for ambiguity
			for (int i = 0; i < (int)allOtherAliveQueens.size(); i++)
			{
				oPos = boardPtr->piece[allOtherAliveQueens[i]].getPos();
				if (!(boardPtr->validateHozMove(oPos, mT2) |
							boardPtr->validateDiagMove(oPos, mT2)))
					continue;

				dupMove = true;

				// Make sure we're not needlessly disambiguating
				if (oPos % 10 == mT2 % 10) // Same file
					e = 10;
				else if (oPos / 10 == mT2 / 10) // Same rank
					e = 1;
				else if ((oPos - mT2) % 11 == 0)
				{ // Positive slope diagonal
					e = 11;
				}
				else if ((oPos - mT2) % 9 == 0)
				{ // Negative slope diagonal
					e = 9;
				}

				// Loop through the squares between otherPiece
				// and moveTo, and if any of them are moveFrom,
				// it's not really an ambiguous move
				if (e)
				{
					big = oPos > mT2 ? oPos : mT2;
					smallE = oPos < mT2 ? oPos : mT2;
					for (int j = smallE + e; j < big; j += e)
						if (j == mF2)
						{
							dupMove = false;
							// Erase the queen from the list
							allOtherAliveQueens.erase(allOtherAliveQueens.begin() + i);
							i--;
						}
				}
				if (dupMove)
				{
					if (mF2 % 10 != oPos % 10) // Not same file
						notSameFile++;
					else // Same file
						notSameRank++;
				}
			}
			if (notSameFile)
			{ // need file to differentiate
				plyStr += mF2 % 10 + 'a' - 1;
			}
			if (notSameRank)
			{ // need rank to differentiate
				plyStr += mF2 / 10 + '1' - 2;
			}
		}
		else if (boardPtr->piece[p].getValue() == K_VAL)
		{
			if (mF2 == E1 && (mT2 == B1 || mT2 == G1))
			{ // White castled
				castling = true;
				plyStr += (mT2 == G1) ? "0-0" : "0-0-0";
			}
			else if (mF2 == E8 && (mT2 == B8 || mT2 == G8))
			{ // Black castled
				castling = true;
				plyStr += (mT2 == G8) ? "0-0" : "0-0-0";
			}
			else
				plyStr += "K";
		}
		else if (boardPtr->piece[p].getValue() == R_VAL)
		{
			plyStr += "R";
			// Check same side and same piecetype, for ambiguous moves
			otherPiece = !boardPtr->getSide() ? (p == 0 ? 7 : 0) : (p == 16 ? 23 : 16);
			oPos = boardPtr->piece[otherPiece].getPos();

			if (boardPtr->piece[otherPiece].getAlive())
				if (boardPtr->validateHozMove(oPos, mT2))
				{
					dupMove = true;
					// Make sure we're not needlessly disambiguating
					if (oPos % 10 == mT2 % 10) // Same file
						e = 10;
					else if (oPos / 10 == mT2 / 10) // Same rank
						e = 1;
					// Loop through the squares between otherPiece
					// and moveTo, and if any of them are moveFrom,
					// it's not really an ambiguous move
					if (e)
					{
						big = oPos > mT2 ? oPos : mT2;
						smallE = oPos < mT2 ? oPos : mT2;
						for (int i = smallE + e; i < big; i += e)
							if (i == mF2)
								dupMove = false;
					}
				}
		}
		else if (boardPtr->piece[p].getValue() == B_VAL)
			plyStr += "B";
		else if (boardPtr->piece[p].getValue() == N_VAL)
		{
			plyStr += "N";
			// Check same side and same piecetype, for ambiguous moves
			otherPiece = !boardPtr->getSide() ? (p == wqN ? wkN : wqN) : (p == bqN ? bkN : bqN);
			if (boardPtr->piece[otherPiece].getAlive())
				if (boardPtr->validateKnightMove(oPos, mT2))
					dupMove = true;
		}
		if (dupMove)
		{ // If the move was ambiguous, de-ambiguate
			// Queen deambiguating is done in the Queen's check above
			if (boardPtr->piece[p].getValue() != Q_VAL)
			{
				if (mF2 % 10 != oPos % 10)			// Not same file
					plyStr += mF2 % 10 + 'a' - 1; // so, file is sufficient
				else														// Same file
					plyStr += mF2 / 10 + '1' - 2; // so, rank is sufficient
			}
		}
		if (boardPtr->getPrevOnMoveTo(lastMove) != none)
		{ // If move was a capture
			// Special case for pawns, display the file of departure
			if (boardPtr->piece[p].getValue() == P_VAL ||
					boardPtr->getPmSq(boardPtr->getNumMovesMade() - 1 + extra) == mT2)
				plyStr += mF2 % 10 + 'a' - 1;
			plyStr += "x";
		}
		else if (boardPtr->piece[p].getValue() == P_VAL)
			if (abs(mF2 - mT2) == 9 || abs(mF2 - mT2) == 11)
			{ // En passant
				plyStr += mF2 % 10 + 'a' - 1;
				plyStr += 'x';
			}

		if (!castling) // Add moveTo
			plyStr += intToSquare(mT2);

		if (boardPtr->getPmSq(lastMove) == mT2) // If it was a promotion
			plyStr += "=Q";

		if (boardPtr->getSideInCheck())
			plyStr += boardPtr->getSideInCheckmate() ? '#' : '+';

		plyStrings.push_back(plyStr); // Add it to the list of ply-moves
	}
	for (size_t i = 0; i < plyStrings.size(); i += 2)
	{																			 // Loop through moves
		plyStr = plyStrings[i];							 // Load white move
		if (i + 1 < plyStrings.size())			 // If black has moved,
			plyStr += " " + plyStrings[i + 1]; // load their move, too
		moveText.loadFromRenderedText(renderer, plyStr, textColor, Cicero22);
		moveText.render(renderer, BXSTART + (i / 42 * 180) + B_SIZE + 40,
										BYSTART + 10 + ((i / 2) % 21) * 30);
	}
}

void Display::drawTitleScreen()
{
	SDL_Rect clipSq;

	// Draw the border and background
	SDL_Rect borderRect = {BXSTART + B_SIZE + 25, BYSTART, 500, 650};
	SDL_SetRenderDrawColor(renderer, 236, 247, 235, 255);
	SDL_RenderFillRect(renderer, &borderRect);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(renderer, &borderRect);
	borderRect = {BXSTART + B_SIZE + 24, BYSTART - 1, 500, 650};
	SDL_RenderDrawRect(renderer, &borderRect);

	titleTexture.render(renderer, BXSTART + B_SIZE + 74, BYSTART);

	if (menu == offlineMenu)
	{
		// Draw white options
		//"White:"
		clipSq = titleTextClips[0];
		titleTextTexture.render(renderer, BXSTART + B_SIZE + 224, BYSTART + 200, &clipSq);
		//"Human"
		if (buttons[2]->getInside() || !boardPtr->getWhiteIsBot())
			buttons[2]->setButtonTextColor({100, 0, 0});
		else
			buttons[2]->setButtonTextColor({0, 0, 0});
		buttons[2]->ShowButton(renderer);

		//"Computer"
		if (buttons[3]->getInside() || boardPtr->getWhiteIsBot())
			buttons[3]->setButtonTextColor({100, 0, 0});
		else
			buttons[3]->setButtonTextColor({0, 0, 0});
		buttons[3]->ShowButton(renderer);

		// Numbers 1-9
		if (boardPtr->getWhiteIsBot())
		{
			for (int i = 1; i < 10; i++)
			{
				if (buttons[i + 5]->getInside() ||
						boardPtr->whiteBot.getLevel() == i)
					buttons[i + 5]->setButtonTextColor({100, 0, 0});
				else
					buttons[i + 5]->setButtonTextColor({0, 0, 0});
				buttons[i + 5]->ShowButton(renderer);
			}
		}

		// Draw black options
		//"Black:"
		clipSq = titleTextClips[1];
		titleTextTexture.render(renderer, BXSTART + B_SIZE + 224, BYSTART + 350, &clipSq);
		//"Human"
		if (buttons[4]->getInside() || !boardPtr->getBlackIsBot())
			buttons[4]->setButtonTextColor({100, 0, 0});
		else
			buttons[4]->setButtonTextColor({0, 0, 0});
		buttons[4]->ShowButton(renderer);
		//"Computer"
		if (buttons[5]->getInside() || boardPtr->getBlackIsBot())
			buttons[5]->setButtonTextColor({100, 0, 0});
		else
			buttons[5]->setButtonTextColor({0, 0, 0});
		buttons[5]->ShowButton(renderer);
		// Numbers 1-9
		if (boardPtr->getBlackIsBot())
		{
			for (int i = 1; i < 10; i++)
			{
				if (buttons[i + 14]->getInside() ||
						boardPtr->blackBot.getLevel() == i)
					buttons[i + 14]->setButtonTextColor({100, 0, 0});
				else
					buttons[i + 14]->setButtonTextColor({0, 0, 0});
				buttons[i + 14]->ShowButton(renderer);
			}
		}
	}
	//"Flip board"
	if (menu != loginMenu)
		displayDefaultButtom(buttons[24]);

	//"Start"
	if (boardPtr->getGamemode() == Board::gamemode::offline || socketPtr->getIsOwner())
		displayDefaultButtom(buttons[25]);

	//"Online" "Offline"
	if (menu == mainMenu)
	{
		for (int i = 26; i < 28; i++)
		{
			displayDefaultButtom(buttons[i]);
		}
	}
	else if (menu != loginMenu)
	{ // Mainmenu
		displayDefaultButtom(buttons[28]);
	}

	if (menu == onlineMenu)
	{
		displayDefaultButtom(buttons[29]);
		displayDefaultButtom(buttons[30]);
	}

	if (menu == roomMenu)
	{
		// Player name
		playerNameText.loadFromRenderedText(renderer, socketPtr->getPlayerName(), textColor, Garamond28);
		playerNameText.render(renderer, BXSTART + B_SIZE + 100, BYSTART + 300);

		opponentNameText.loadFromRenderedText(renderer, socketPtr->getOpponentName(), textColor, Garamond28);
		opponentNameText.render(renderer, BXSTART + B_SIZE + 300, BYSTART + 300);

		roomCodeText.loadFromRenderedText(renderer, "Room: " + socketPtr->getRoomCode(), textColor, Garamond28);
		roomCodeText.render(renderer, BXSTART + B_SIZE + 300, BYSTART + 220);

		if (!socketPtr->getIsOwner())
		{
			displayDefaultButtom(buttons[33]);
			std::string readyText = "You are ";
			guestReadyText.loadFromRenderedText(renderer, readyText + (socketPtr->getIsGuestReady() ? "ready" : "not ready"), textColor, Garamond28);
			guestReadyText.render(renderer, BXSTART + B_SIZE + 150, BYSTART + 450);
		}

		if (socketPtr->getIsOwner() && socketPtr->isRoomFull() && !socketPtr->getIsGuestReady())
		{
			guestReadyText.loadFromRenderedText(renderer, "Guest is not ready", textColor, Garamond28);
			guestReadyText.render(renderer, BXSTART + B_SIZE + 150, BYSTART + 450);
		}
	}

	if (menu == joinRoomMenu)
	{
		displayInputField(dynamic_cast<InputField*>(buttons[31]));
		displayDefaultButtom(buttons[32]);

		if (socketPtr->getRoomFoundState())
		{
			roomNotFoundText.loadFromRenderedText(renderer, "Room not found", textColor, Garamond28);
			roomNotFoundText.render(renderer, BXSTART + B_SIZE + 200, BYSTART + 350);
		}
		if (socketPtr->getRoomFull())
		{
			roomNotFoundText.loadFromRenderedText(renderer, "Room is full", textColor, Garamond28);
			roomNotFoundText.render(renderer, BXSTART + B_SIZE + 200, BYSTART + 350);
		}
	}

	if (menu == loginMenu)
	{
		displayInputField(dynamic_cast<InputField*>(buttons[34]));
		displayInputField(dynamic_cast<InputField*>(buttons[35]));
		displayDefaultButtom(buttons[36]);
	}
}

void Display::handleButtons(SDL_Event *e)
{
	int sound = 0;
	isTyping = false;
	for (int i = 0; i < NUMBER_OF_BUTTOMS; i++)
	{
		buttons[i]->handleEvent(e, sound);
		if (sound == 1)
			Mix_PlayChannel(-1, mFSound, 0);
		else if (sound == 2)
			Mix_PlayChannel(-1, mTSound, 0);

		InputField* inputField = dynamic_cast<InputField*>(buttons[i]);
		if (inputField != nullptr && inputField->getIsEditing())
			isTyping = true;
	}
}

void Display::displayDefaultButtom(Button *button)
{
	if (button->getInside())
		button->setButtonTextColor(ON_HOVER_COLOR);
	else
		button->setButtonTextColor(textColor);
	button->ShowButton(renderer);
}

void Display::displayInputField(InputField *inputField)
{
	if (inputField->getIsEditing())
	{
		inputField->setButtonText(inputText);
		if (inputField->getButt() == 31)
			socketPtr->setRoomCode(inputText);
		if (inputField->getButt() == 34)
			socketPtr->setUsername(inputText);
		if (inputField->getButt() == 35)
			socketPtr->setPassword(inputText);
	}

	inputField->ShowButton(renderer);
}

void Display::setPlayerSideText()
{
	std::string playerText = "You play as ";
	playerText += (boardPtr->getPlayerSide() ? "white" : "black");
	playerSideText.loadFromRenderedText(renderer, playerText, textColor, Garamond26);
}