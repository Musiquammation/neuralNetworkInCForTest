#ifndef GAMES_ORB_PROJECTILE_H_
#define GAMES_ORB_PROJECTILE_H_

#include <tools/tools.h>

structdef(Projectile);

enum {
	PROJECTILE_SIZE = 30,
	PROJECTILE_DAMAGES = 600
};

enum {
	PROJECTILE_GRAVITITY_EFFECT = 1,
	PROJECTILE_THROW_SPEED = 1400,
	PROJECTILE_MAX_SPEED = 1600,
};

#define PROJECTILE_THROW_DELAY .1f
#define PROJECTILE_ACCELERATION 1000.0f



struct Projectile {
	float x;
	float y;
	float sx;
	float sy;
	
	ushort owner;
};

int Projectile_frame(Projectile* prj, Player* playerList, float frameSpeed);
bool Projectile_isNull(const void* projectile);
#define Projectile_isNotAlive(p) ((p).owner == NULL_USHORT);
#define Projectile_teamA(owner) ((owner) < (PLAYER_COUNT/2))

#endif
