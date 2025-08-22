#include "Game.h"

#include "rooms.h"
#include "net.h"

void Player_init(Player* player, bool teamA, ushort index) {
	Player_teleportToSpawn(player,  teamA);
	player->hp = PLAYER_FULL_LIFE;

	player->dirX = 0;
	player->dirY = 0;

	player->jumps = 3;
	player->respawnCouldown = -1;
	player->actionMask = 0;
	player->privateScore = 0;

	player->attack.x = 0;
	player->attack.y = 0;
	player->attack.couldown = 0;
	player->attack.attacking = false;
}



bool Player_move(Player* player, bool canJump, float speed) {
	const uchar mask = player->actionMask;


	const Vector_short lastRoom = RoomList_getCenterCoords(player->x, player->y);

	bool returnValue = false;


	// Lateral behavior
	player->x += player->dirX * PLAYER_SPEED * speed;

	// Jump
	if (canJump && (mask & PLAYER_ACTION_MASK_JUMP)) {
		player->actionMask &= ~PLAYER_ACTION_MASK_JUMP;
		player->sy = -PLAYER_JUMP;

		if ((--player->jumps) < 0) {
			// hit player
			returnValue = true;
		}
	}

	// Parachute
	if ((mask & PLAYER_ACTION_MASK_FIRST_PARACHUTE) && (player->sy >= 0)) {
		player->sy *= .5f;
	}
	
	float gravitiyEffect;
	if (mask & PLAYER_ACTION_MASK_PARACHUTE) {
		gravitiyEffect = PLAYER_GRAVITY * PLAYER_PARACHUTE_FACTOR;
	} else {
		gravitiyEffect = PLAYER_GRAVITY;
	}

	// Dash
	player->y += player->dirY * PLAYER_DASH * speed;


	// Move and gravity
	float sy = (player->sy += gravitiyEffect * speed);
	if (sy > PLAYER_GRAVITY_MAX) {
		sy = PLAYER_GRAVITY_MAX;
		player->sy = PLAYER_GRAVITY_MAX;
	}


	// Check room changing
	const Vector_short finalRoom = RoomList_getCenterCoords(
		(player->x += player->sx * speed),
		(player->y += sy * speed)
	);

	if (lastRoom.x != finalRoom.x || lastRoom.y != finalRoom.y) {
		if (returnValue) {
			player->jumps = PLAYER_JUMP_COUNT - 1;
			returnValue = false;
		} else {
			player->jumps = PLAYER_JUMP_COUNT;
		}
	}


	return returnValue;

}


void Player_teleportToSpawn(Player* player, bool teamA) {
	player->x = teamA ? -GAME_SCREEN_WIDTH*2 : GAME_SCREEN_WIDTH*2;
	player->y = 0;
	player->sx = 0;
	player->sy = 0;
	player->hp = PLAYER_FULL_LIFE;
}



void Player_hit(Player* player, Array* msgArr, int damages, uchar playerIndex) {
	int hp = (player->hp -= damages);
	if (hp <= 0) {
		hp = 0;
		player->hp = PLAYER_FULL_LIFE;
		player->jumps = PLAYER_JUMP_COUNT;
	}


	// Send message : player hit
	{
		enum {
			SIZE = sizeof(ushort)  // NET_MSG_PLAYER_HIT
				   + sizeof(uchar) // playerIndex
				   + sizeof(short) // damages
				   + sizeof(int)   // hp
				   + sizeof(float)*2 // x, y
		};
	
	
		char* const globalMsg = malloc(sizeof(ushort) + SIZE);
	
		char* msg = globalMsg;
		pushMsg(ushort, SIZE);
		pushMsg(ushort, NET_MSG_PLAYER_HIT);
		pushMsg(uchar, playerIndex);
		pushMsg(short, (short)damages);
		pushMsg(int, hp);
		pushMsg(float, player->x);
		pushMsg(float, player->y);
	
		*Array_push(char*, msgArr) = globalMsg;
	}

	if (hp > 0)
		return;

	// If here, player died
	player->respawnCouldown = PLAYER_RESCLD_KILLED;
	Player_teleportToSpawn(player, playerIndex < PLAYER_COUNT/2);

	// Send message : player dead
	enum {
		SIZE = sizeof(ushort)  // NET_MSG_PLAYER_DIE
			   + sizeof(uchar) // playerIndex
	};


	char* const globalMsg = malloc(sizeof(ushort) + SIZE);

	char* msg = globalMsg;
	pushMsg(ushort, SIZE);
	pushMsg(ushort, NET_MSG_PLAYER_DIE);
	pushMsg(uchar, playerIndex);
	*Array_push(char*, msgArr) = globalMsg;
}



// Sort players. In case of equality, the player with the lower y wins
void Player_sortResults(char results[], const Player playerList[], uchar startIndex) {
	int indices[PLAYER_COUNT/2];
    for (int i = 0; i < PLAYER_COUNT/2; i++) {
        indices[i] = startIndex + i;
    }
    
    // Tri par privateScore (descendant), puis par y (ascendant) en cas d'égalité
    for (int i = 0; i < (PLAYER_COUNT/2-1); i++) {
        for (int j = i + 1; j < PLAYER_COUNT/2; j++) {
            if (playerList[indices[i]].privateScore < playerList[indices[j]].privateScore || 
                (playerList[indices[i]].privateScore == playerList[indices[j]].privateScore && playerList[indices[i]].y > playerList[indices[j]].y)) {
                // Échanger les indices
                int temp = indices[i];
                indices[i] = indices[j];
                indices[j] = temp;
            }
        }
    }
    
    // Stocker le résultat
    for (int i = 0; i < PLAYER_COUNT/2; i++) {
        results[i] = indices[i];
    }
}



void Player_collectFinalResults(const Game* game, SI_Handler* handler) {
	const char scoreA = game->roomList.scoreA;
	const char scoreB = game->roomList.scoreB;
	if (scoreA > scoreB) {
		// Team A won
		teamWin_A:
		Player_sortResults(
			&handler->results.rankings[0],
			game->players,
			0
		);

		Player_sortResults(
			&handler->results.rankings[PLAYER_COUNT/2],
			game->players,
			PLAYER_COUNT/2
		);

		return;
	}
		
	
	if (scoreA < scoreB) {
		// Team B won
		teamWin_B:
		Player_sortResults(
			&handler->results.rankings[0],
			game->players,
			PLAYER_COUNT/2
		);

		Player_sortResults(
			&handler->results.rankings[PLAYER_COUNT/2],
			game->players,
			0
		);

		return;
	}

	
	// Get y sum
	float yA = 0;
	float yB = 0;

	Array_for(const Player, &game->players[0], PLAYER_COUNT/2, p) {
		yA += p->y;
	}

	Array_for(const Player, &game->players[PLAYER_COUNT/2], PLAYER_COUNT/2, p) {
		yB += p->y;
	}

	if (yA > yB) {
		goto teamWin_A;
	}

	goto teamWin_B;
}