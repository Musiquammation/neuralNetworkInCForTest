#include "Game.h"

#include "bots.h"
#include "net.h"

#include "Projectile.h"
#include "coords.h"

#include <tools/Stream.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>


bool Game_isHorizontalOutOfMap(float x) {
	return x < -GAME_FULL_WIDTH/2 || x > GAME_FULL_WIDTH/2;
}

bool Game_isVerticalOutOfMap(float y) {
	return y < -GAME_FULL_HEIGHT/2 || y > GAME_FULL_HEIGHT/2;
}




SI_Interface SERV_INTERFACE = {
	game_start,
	game_run,
	game_cleanUp,
	game_read,
	game_write,
	game_readFirst,
	game_writeFirst,

	bots_init,
	bots_cleanup,
	bots_run,
	bots_learn,

	NULL,
	PLAYER_COUNT
};


void* game_start() {
	Game* game = malloc(sizeof(Game));
	game->chrono = 0;
	game->ball.x = 0;
	game->ball.y = 0;
	game->ball.sx = 0;
	game->ball.sy = 0;
	game->ball.couldown = BALL_COULDOWN_FIRST;
	game->ball.lastHolder = BALL_NOT_HOLD;
	memset(&game->roomList, 0, sizeof(game->roomList));


	for (ushort i = 0; i < PLAYER_COUNT/2; i++) {
		Player_init(&game->players[i], true, i);
	}

	for (ushort i = PLAYER_COUNT/2; i < PLAYER_COUNT; i++) {
		Player_init(&game->players[i], false, i+(PLAYER_COUNT/2));
	}


	Array_create(&game->projectiles, sizeof(Projectile));

	return game;
}



void game_run(void* __game__, SI_Handler* handler) {
	Game* const game = __game__;
	const float frameSpeed = handler->speed;
	
	game->chrono += frameSpeed;

	
	// Update ball
	{
		char result = Ball_move(game, frameSpeed);

		if (result > 0) {
			Ball_checkHold(game);
		} else if (result < 0) {
			// Ball out of the map
			enum {SIZE = sizeof(ushort)};
			char* const globalMsg = malloc(sizeof(ushort) + SIZE);

			char* msg = globalMsg;
			pushMsg(ushort, SIZE);
			pushMsg(ushort, NET_MSG_BALL_OUT);


			*Array_push(char*, &handler->commonMessages) = globalMsg;
		}
	}


	// Check hook
	{
		const Vector_short room = Ball_checkHook(game);

		if (room.x == 0x7fff)
			goto exitCheckHook;


		// Send message : new score
		enum {
			SIZE = sizeof(ushort)   // NET_MSG_SCORE
			     + sizeof(uchar)*2  // scoreA and scoreB
			     + sizeof(ushort)*2 // roomX and roomY
		};
	
		char* const globalMsg = malloc(sizeof(ushort) + SIZE);
		
		char* msg = globalMsg;
		pushMsg(ushort, SIZE);
		pushMsg(ushort, NET_MSG_SCORE);
		pushMsg(uchar, game->roomList.scoreA);
		pushMsg(uchar, game->roomList.scoreB);
		pushMsg(ushort, room.x);
		pushMsg(ushort, room.y);

	
		*Array_push(char*, &handler->commonMessages) = globalMsg;
	}


	// If here, let's check if game is not finished
	if (game->roomList.scoreA + game->roomList.scoreB >= FINAL_ROOM_COUNT) {
		handler->results.gameFinished = true;
		Player_collectFinalResults(game, handler);
		return;
	}


	exitCheckHook:




	// Move players
	{
		const int holdBy = game->ball.holdBy;
		for (int i = 0; i < PLAYER_COUNT; i++) {
			Player* p = &game->players[i];
			if (p->respawnCouldown > 0 && (p->respawnCouldown -= frameSpeed) > 0) {
				continue;
			}

			if (holdBy == i) {
				if (Player_move(p, false, frameSpeed)) {
					Player_hit(p, &handler->commonMessages, PLAYER_JUMP_COST, p - game->players);
				}

				Ball_bonceAround(p->x, p->y, &game->ball);
			} else {
				if (Player_move(&game->players[i], true, frameSpeed)) {
					Player_hit(p, &handler->commonMessages, PLAYER_JUMP_COST, p - game->players);
				}
			}
		}
	}
	
	// Check bounds and attack
	{
		const int holdBy = game->ball.holdBy;
		for (int i = 0; i < PLAYER_COUNT; i++) {
			Player* const p = &game->players[i];

			if (p->respawnCouldown > 0)
				continue;

			if ((Game_isHorizontalOutOfMap(p->x) || Game_isVerticalOutOfMap(p->y))) {
				// Player out of bounds
				p->respawnCouldown = PLAYER_RESCLD_OUT_OF_MAP;
				Player_teleportToSpawn(p, p - game->players < PLAYER_COUNT/2);

				// Send message
				enum {
					SIZE = sizeof(ushort)  // NET_MSG_PLAYER_DIE
						+ sizeof(uchar) // playerIndex
				};


				char* const globalMsg = malloc(sizeof(ushort) + SIZE);

				char* msg = globalMsg;
				pushMsg(ushort, SIZE);
				pushMsg(ushort, NET_MSG_PLAYER_DIE);
				pushMsg(uchar, i);
				*Array_push(char*, &handler->commonMessages) = globalMsg;

				continue;
			}


			// Handle attack
			float couldown = p->attack.couldown;
			const char attackingValue = p->attack.attacking;

			if (couldown > 0) {
				couldown -= frameSpeed;
				
				if (couldown > 0) {
					p->attack.couldown = couldown;
					continue;
				}
				
					
				if (attackingValue) {
					p->attack.couldown += PLAYER_ATTACK_COULDOWN;
					goto playerAttack;
				}

				p->attack.couldown = 0;
				continue;
			}
			
			if (attackingValue) {
				p->attack.couldown = PLAYER_ATTACK_COULDOWN;
				
				playerAttack:
				if (i == holdBy) {
					// Throw ball
					Vector speed;
					
					if (attackingValue == 1) {
						// Mouse
						speed = coords_getStartSpeed(
							p->attack.x - game->ball.x,
							p->attack.y - game->ball.y,
							BALL_GRAVITITY_EFFECT,
							BALL_THROW_SPEED
						);

					} else if (attackingValue == -1) {
						// Joystick
						speed.x = p->attack.x * PLAYER_JOYSTICK_FULL_THROW_BALL;
						speed.y = p->attack.y * PLAYER_JOYSTICK_FULL_THROW_BALL;

					} else {
						speed.x = 0;
						speed.y = -1;
					}

					game->ball.sx = speed.x;
					game->ball.sy = speed.y;

					game->ball.holdBy = BALL_NOT_HOLD;


					goto exitAttack;
				}

				Projectile* prj = Array_pushInEmpty(&game->projectiles, Projectile_isNull);
				prj->x = p->x;
				prj->y = p->y;

				Vector speed;
				if (attackingValue == 1) {
					speed = coords_getStartSpeed(
						p->attack.x - p->x,
						p->attack.y - p->y,
						PROJECTILE_GRAVITITY_EFFECT,
						PROJECTILE_THROW_SPEED
					);
				} else if (attackingValue == -1) {
					speed.x = p->attack.x * PLAYER_JOYSTICK_FULL_THROW_PROJECTILE;
					speed.y = p->attack.y * PLAYER_JOYSTICK_FULL_THROW_PROJECTILE;
				} else {
					speed.x = 0;
					speed.y = -1;
				}

				prj->sx = speed.x;
				prj->sy = speed.y;
				prj->owner = p - game->players;
			}

			exitAttack:
			{}
		}
	}


	// Projectiles
	Array_loop(Projectile, game->projectiles, prj) {
		const int index = Projectile_frame(prj, game->players, frameSpeed);
		if (index == 0x7fffffff)
			continue;
		
		// Player hit
		Player_hit(&game->players[index], &handler->commonMessages, PROJECTILE_DAMAGES, (uchar)index);
		
	}



	if (game->chrono >= GAME_DURATION) {
		printf("Game finished [by chrono security]\n");
		handler->results.gameFinished = true;
		Player_collectFinalResults(game, handler);
		return;
	}
}


void game_cleanUp(void* __game__) {
	Game* const game = __game__;

	Array_free(game->projectiles);
	printf("cleanup\n");
}



void* game_read(void* __game__, void* msg, ushort msgLabel, ushort playerIndex) {
	Game* const game = __game__;

	switch (msgLabel) {
	

	case NET_CMD_EVENT_JUMP:
	{
		game->players[playerIndex].actionMask |= PLAYER_ACTION_MASK_JUMP;
		return msg;
	}

	


	case NULL_USHORT: // frame discussion
	{
		// Player direction
		{
			letMsg(float, x);
			letMsg(float, y);
	
			
			if (x > 1) {x = 1;} else if (x < -1) {x = -1;}
			if (y > 1) {y = 1;} else if (y < -1) {y = -1;}
	

			game->players[playerIndex].dirX = x;
			game->players[playerIndex].dirY = y;
		}


		// Attack
		{
			letMsg(uchar, mode);
			switch (mode) {
			case 0: // mouse attack
			{
				Player* const player = &game->players[playerIndex];
				letMsg(float, x);
				letMsg(float, y);
				player->attack.x = x;
				player->attack.y = y;
				player->attack.attacking = 1;
				break;
			}
			
			case 1:
			{
				Player* const player = &game->players[playerIndex];
				letMsg(float, x);
				letMsg(float, y);
				player->attack.x = x;
				player->attack.y = y;
				player->attack.attacking = -1;
				break;
			}
			
			case 2:
			{
				game->players[playerIndex].attack.attacking = 0;
				break;
			}

			default:
				printf("CORRUPTED ATTACK %d\n", mode);
				break;
			}
		}


		return msg;
	}


	default:
		printf("WARNING: Unknown label: %d\n", msgLabel);
		return NULL;
	}
}


void game_write(void* __game__, Stream* stream, ushort playerIndex) {
	Game* game = __game__;

	const Player* const gamePlayer = &game->players[playerIndex];

	const ushort projectileLength = game->projectiles.length;

	char* const globalMsg = malloc(
		+ sizeof(float) // chrono
		+ sizeof(uchar) + 4*sizeof(float) // ball
		+ PLAYER_COUNT * (sizeof(float)*8  + sizeof(uchar)*2) // players
		+ projectileLength * (sizeof(float)*4 + sizeof(ushort)) + sizeof(ushort) // projectiles
	);
	void* msg = globalMsg;
	
	pushMsg(float, game->chrono);

	// Send ball
	if (Ball_isAlive(game->ball)) {
		pushMsg(uchar, 1);
		pushMsg(float, game->ball.x);
		pushMsg(float, game->ball.y);
		pushMsg(float, game->ball.sx);
		pushMsg(float, game->ball.sy);
		pushMsg(uchar, game->ball.holdBy);
	} else {
		pushMsg(uchar, 0);
	}

	// Send players
	Array_for(Player, game->players, PLAYER_COUNT, player) {
		const float respawnCouldown = player->respawnCouldown;

		if (respawnCouldown >= 0) {
			pushMsg(uchar, 1);
			pushMsg(char, (char)respawnCouldown);
			continue;
		}

		pushMsg(uchar, 0);
		pushMsg(float, player->x);
		pushMsg(float, player->y);
		pushMsg(float, player->sx);
		pushMsg(float, player->sy);

		if (player != gamePlayer) {
			pushMsg(char, player->attack.attacking);
			pushMsg(float, player->attack.x);
			pushMsg(float, player->attack.y);
			pushMsg(float, player->dirX);
			pushMsg(float, player->dirY);
		}
	}

	// Send projectiles
	for (int index = 0; index < projectileLength; index++) {
		Projectile* i = Array_get(Projectile, game->projectiles, index);

		const ushort owner = i->owner;
		if (owner == NULL_USHORT)
			continue;

		pushMsg(ushort, owner);
		pushMsg(float, i->x);
		pushMsg(float, i->y);
		pushMsg(float, i->sx);
		pushMsg(float, i->sy);
	}

	// Send projectile end
	{
		pushMsg(ushort, NULL_USHORT);
	}


	Stream_pushData(stream, globalMsg, (char*)msg - globalMsg);
	free(globalMsg);
}

void game_readFirst(void* __game__, void* msg, ushort playerIndex) {
	letMsg(ushort, test);
	printf("firstRead: %d\n", test);
}


void game_writeFirst(void* __game__, Stream* stream, ushort playerIndex) {
	ushort value = 58 + playerIndex * 10;

	Stream_push(stream, value);
}







#include "c_include.h"