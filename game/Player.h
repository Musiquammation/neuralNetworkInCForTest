#ifndef GAMES_ORB_PLAYER_H_
#define GAMES_ORB_PLAYER_H_

#include <tools/tools.h>

structdef(Game);
structdef(SI_Handler);
structdef(Array);

enum {
	PLAYER_ACTION_MASK_JUMP = 1,
	PLAYER_ACTION_MASK_PARACHUTE = 2,
	PLAYER_ACTION_MASK_FIRST_PARACHUTE = 4,
};

enum {
	PLAYER_JUMP_COUNT = 3,
	PLAYER_FULL_LIFE = 2000,
	PLAYER_JUMP_COST = 375,
	PLAYER_JOYSTICK_FULL_THROW_BALL = 500,
	PLAYER_JOYSTICK_FULL_THROW_PROJECTILE = 500
};

#define PLAYER_SIZE 25
#define PLAYER_JUMP 900
#define PLAYER_GRAVITY 1100
#define PLAYER_GRAVITY_MAX 4000
#define PLAYER_SPEED 1400
#define PLAYER_DASH 1400
// #define PLAYER_RESCLD_OUT_OF_MAP 3.8f
// #define PLAYER_RESCLD_KILLED 4.7f
#define PLAYER_ATTACK_COULDOWN 1.0f
#define PLAYER_PARACHUTE_FACTOR .8f

/// TODO: set times
#define PLAYER_RESCLD_OUT_OF_MAP .9f
#define PLAYER_RESCLD_KILLED .9f



structdef(Player);


struct Player {
	float x;
	float y;
	float sx;
	float sy;
	float dirX;
	float dirY;
	
	int hp;

	float respawnCouldown;

	short jumps;
	bool actionMask;
	uchar privateScore;

	struct {
		float x;
		float y;
		float couldown;
		char attacking;
	} attack;
};


void Player_init(Player* player, bool teamA, ushort index);
bool Player_move(Player* player, bool canJump, float speed);
void Player_teleportToSpawn(Player* player, bool teamA);
void Player_hit(Player* player, Array* msgArr, int damages, uchar playerIndex);

void Player_sortResults(char results[], const Player playerList[], uchar startIndex);
void Player_collectFinalResults(const Game* game, SI_Handler* handler);



#endif
