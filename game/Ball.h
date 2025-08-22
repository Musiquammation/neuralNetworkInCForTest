#ifndef GAMES_ORB_BALL_H_
#define GAMES_ORB_BALL_H_

#include <tools/tools.h>

#include "coords.h"

structdef(Ball);
structdef(Game);

enum {BALL_NOT_HOLD = 255};


enum {
	BALL_GRAVITITY_EFFECT = 300,
	BALL_THROW_SPEED = 600,
	BALL_SPAWN_JUMP = 300,
	BALL_SIZE = 75,
	BALL_HOOK_RADIUS = BALL_SIZE+20,
};

#define BALL_THROW_DELAY  .2f
// #define BALL_COULDOWN_OUT 4
// #define BALL_COULDOWN_WIN 6
// #define BALL_COULDOWN_FIRST 6
/// TODO: set times
#define BALL_COULDOWN_OUT 2.0f
#define BALL_COULDOWN_WIN 2.0f
#define BALL_COULDOWN_FIRST 2.0f


struct Ball {
	float x;
	float y;
	float sx;
	float sy;
	
	uchar holdBy;
	uchar lastHolder;
	float couldown;
};

char  Ball_move(Game* game, float speed);
void  Ball_exit(Ball* ball, float couldown);
void  Ball_bonceAround(float x0, float y0, Ball* ball);
uchar Ball_checkHold(Game* game);
Vector_short Ball_checkHook(Game* game);


#define Ball_isAlive(ball) ((ball).couldown <= 0)




#endif