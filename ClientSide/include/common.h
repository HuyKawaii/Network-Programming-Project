#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <string>
#include <cstdlib>

#define ENDGAME_VAL 11309 
#define defaultFEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef unsigned long long U64;

//GAME CONTROL
extern bool quit, muted, start, isTyping;
extern std::string inputText;

//INLINE FUNCTIONS
inline int to64(int x) { //returns 1 to 64
	if (x < 99 && x > 20 && !(x%10 == 0 || x%10 == 9))
			return x-20-2*((x-x%10)/10-2);
	return 0;
}

inline int FR2SQ64(int f, int r) { //returns 1 to 64
	return (f)+((r-1)*8);
}

inline int from64(int x) { //returns 21 to 98
	return x+20+(((x-1)/8))*2;
}

inline std::string intToSquare(int square) { //Board120 to A1, etc
	std::string squareName;
	if (square==0) return "";
	squareName = char(int('a') + ((square)%10)-1);
	squareName += char((square)/10 + 49 - 2);
	return squareName;
}

//ENUMERATIONS
enum side_t { BLACK = 0, WHITE = 1 
};

enum square2_t { none = -1, null = 0, invalid = -99,
		 A1 = 21, B1, C1, D1, E1, F1, G1, H1,
                 A2 = 31, B2, C2, D2, E2, F2, G2, H2,
                 A3 = 41, B3, C3, D3, E3, F3, G3, H3,
                 A4 = 51, B4, C4, D4, E4, F4, G4, H4,
                 A5 = 61, B5, C5, D5, E5, F5, G5, H5,
                 A6 = 71, B6, C6, D6, E6, F6, G6, H6,
                 A7 = 81, B7, C7, D7, E7, F7, G7, H7,
                 A8 = 91, B8, C8, D8, E8, F8, G8, H8 
};

enum dir_t { L = -1, R = 1, U = 10, D = -10, 
	     UL = 9, UR = 11, DL = -11, DR = -9, 
	     K1 = 8, K2 = -8, K3 = 12, K4 = -12, 
	     K5 = 19, K6 = -19, K7 = 21, K8 = -21
};

enum piece_t {wqR = 0, wqN, wqB, wQ, wK, wkB, wkN, wkR,
               wPa, wPb, wPc, wPd, wPe, wPf, wPg, wPh,
               bqR, bqN, bqB, bQ, bK, bkB, bkN, bkR,
               bPa, bPb, bPc, bPd, bPe, bPf, bPg, bPh 
};

enum pieceValue_t { P_VAL = 100, N_VAL = 300, B_VAL = 310,
                     R_VAL = 500, Q_VAL = 1000, K_VAL = 9999 
};

enum castleside_t { QUEENSIDE = 2, KINGSIDE = 1
};

enum scores_t { CHECKMATE_VAL = 9999, STALEMATE_VAL = 8000, MATING_VAL = 9000
};

enum type_t { ROOK = 0, KNIGHT, BISHOP, QUEEN, KING, PAWN
};

enum dir8_t { NORTH = 0, NORTHEAST, EAST, SOUTHEAST, 
			  SOUTH, SOUTHWEST, WEST, NORTHWEST 
};

#endif
