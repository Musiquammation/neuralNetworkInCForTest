#include "Projectile.h"

#include "Game.h"

#include <stdio.h>

int Projectile_frame(Projectile* prj, Player* playerList, float frameSpeed) {
	const ushort owner = prj->owner;
	if (owner == NULL_USHORT)
		return 0x7fffffff;

	const float x = (prj->x += prj->sx * frameSpeed);
	const float y = (prj->y += (prj->sy += PROJECTILE_GRAVITITY_EFFECT * frameSpeed) * frameSpeed);

	// Check out of bounds
	if (Game_isHorizontalOutOfMap(x) || Game_isVerticalOutOfMap(y)) {
		prj->owner = NULL_USHORT;
		return 0x7fffffff;
	}


	// Search the nearest player alive to get closer to him
	{
		const Player* best = NULL;
		float best_dist = 3e38f;
		float best_dx;
		float best_dy;
		for (
			const Player* p = owner < PLAYER_COUNT/2 ? &playerList[PLAYER_COUNT/2] : &playerList[0],
			*const end = p + PLAYER_COUNT/2;
			p < end;
			p++
		) {
			if (p->respawnCouldown <= 0) {
				float dx = p->x - x;
				float dy = p->y - y;
				float dist = dx*dx + dy*dy;
				if (dist < best_dist) {
					best_dist = dist;
					best = p;
					best_dx = dx;
					best_dy = dy;
				}
			}
		}

		// Get closer to player
		if (best) {
			best_dist = PROJECTILE_ACCELERATION * frameSpeed * Q_rsqrt(best_dist);
			float sx = (prj->sx += best_dx * best_dist);
			float sy = (prj->sy += best_dy * best_dist);

			float norm = sx*sx + sy*sy;
			if (norm > PROJECTILE_MAX_SPEED) {
				norm = PROJECTILE_MAX_SPEED * Q_rsqrt(norm);
				prj->sx = sx*norm;
				prj->sy = sy*norm;
			}
		}
	}

	// Check for players to hit
	{
		Player* player;

		if (Projectile_teamA(owner)) {
			player = &playerList[PLAYER_COUNT/2]; // hit teamB
		} else {
			player = &playerList[0]; // hit teamA
		}
		

		for (const Player* const endPlayer = player + (PLAYER_COUNT/2); player < endPlayer; player++) {
			if (player->respawnCouldown > 0)
				continue;

			const float dx = player->x - x;
			const float dy = player->y - y;

			if (dx*dx+dy*dy < (PROJECTILE_SIZE*PROJECTILE_SIZE)) {
				prj->owner = NULL_USHORT;
				return player - playerList;
			}
		}
	}


	return 0x7fffffff;
}

bool Projectile_isNull(const void* projectile) {
	return Projectile_isNotAlive(*((Projectile*)projectile));
}



