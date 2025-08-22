#ifndef GAMES_ORB_GAME_H_
#define GAMES_ORB_GAME_H_

// clear && gcc -c -g -fPIC "src/"*.c -ltools && gcc -shared -o "bin/game.so" *.o && rm *.o

#include "/home/musiquammation/Documents/Prog/OnlineGames/api/src/ServInterface.h"
#include "Player.h"
#include "Ball.h"
#include "rooms.h"


enum {
	GAME_SCREEN_WIDTH = 1600,
	GAME_SCREEN_HEIGHT = 900,

	GAME_FULL_WIDTH = 7 * GAME_SCREEN_WIDTH,
	GAME_FULL_HEIGHT = 3 * GAME_SCREEN_HEIGHT,
};

enum {
	PLAYER_COUNT = 4,
	GAME_DURATION = 5*60
};


structdef(Game);

struct Game {
	Player players[PLAYER_COUNT];
	Array projectiles; // type: Projectile

	float chrono;
	RoomList roomList;

	Ball ball;
};

extern SI_Interface SERV_INTERFACE;


bool Game_isHorizontalOutOfMap(float x);
bool Game_isVerticalOutOfMap(float y);



SI_StartMethod game_start;
SI_RunMethod game_run;
SI_CleanupMethod game_cleanUp;
SI_ReadMethod game_read;
SI_WriteMethod game_write;
SI_FirstReadMethod game_readFirst;
SI_FirstWriteMethod game_writeFirst;



#endif