#include "Ball.h"
#include "Game.h"
#include "rooms.h"
#include "coords.h"

char Ball_move(Game* game, float speed) {
	if (game->ball.couldown > 0) {
		if ((game->ball.couldown -= speed) > 0)
			return 0;
		
		// Init ball
		game->ball.holdBy = BALL_NOT_HOLD;
		game->ball.lastHolder = BALL_NOT_HOLD;	
		game->ball.x = 0;
		game->ball.y = 0;
		game->ball.sx = 0;
		game->ball.sy = -BALL_SPAWN_JUMP;
	}

	{
		const uchar holdBy = game->ball.holdBy;
		if (holdBy != BALL_NOT_HOLD) {
			// Check player
			if (game->players[holdBy].respawnCouldown <= 0)
				return 0; // player is alive
			
			// If here, player is dead
			game->ball.holdBy = BALL_NOT_HOLD;
		}
	}
	
	// If here, move ball
	const float x = (game->ball.x += game->ball.sx * speed);
	const float y = (game->ball.y += game->ball.sy * speed);

	if (Game_isHorizontalOutOfMap(x) || Game_isVerticalOutOfMap(y)) {
		Ball_exit(&game->ball, BALL_COULDOWN_OUT);
		return -1;
	}

	// Gravity effect
	game->ball.sy += BALL_GRAVITITY_EFFECT * speed;
	return 1;

	
}


void Ball_exit(Ball* ball, float couldown) {
	ball->holdBy = BALL_NOT_HOLD;
	ball->lastHolder = BALL_NOT_HOLD;
	ball->couldown = couldown;
	ball->x = 0;
	ball->y = 0;
	ball->sx = 0;
	ball->sy = 0;
}

void Ball_bonceAround(float x0, float y0, Ball* ball) {
	ball->x = x0; 
	ball->y = y0;
}

uchar Ball_checkHold(Game* game) {
	float x = game->ball.x;
	float y = game->ball.y;

	float bestDist = (PLAYER_SIZE+BALL_SIZE)*(PLAYER_SIZE+BALL_SIZE);
	Player* bestPlayer = NULL;

	Player* const forbiddenPlayer = game->players + game->ball.lastHolder;
	
	Array_for(Player, game->players, PLAYER_COUNT, player) {
		if (player == forbiddenPlayer)
			continue;

		float dist = coords_getSqDist(x - player->x, y - player->y);
		if (dist <= bestDist) {
			bestDist = dist;
			bestPlayer = player;
		}
	}

	if (bestPlayer == NULL)
		return BALL_NOT_HOLD;
	
	/// TODO: hold by
	const uchar holdBy = bestPlayer - game->players;
	// const uchar holdBy = BALL_NOT_HOLD;
	game->ball.holdBy = holdBy;
	game->ball.lastHolder = holdBy;
	return holdBy;
}



Vector_short Ball_checkHook(Game* game) {
	{
		const uchar lastHolder = game->ball.lastHolder;
		if (lastHolder == BALL_NOT_HOLD)
			goto retNull;
	
		Vector ballPos = {game->ball.x, game->ball.y};
	
		Vector_short pos = RoomList_getCenterCoords(ballPos.x, ballPos.y);
	
		
		if (pos.x == 0 && pos.y != 1)
			goto retNull; // undef
			
		const char roomMask = 1 << (pos.x+3);
		const short roomY = pos.y+1;
	
	
		if ((game->roomList.used[roomY] & roomMask))
			goto retNull; // already used
	
		
		const float dx = pos.x * GAME_SCREEN_WIDTH  - ballPos.x;
		const float dy = pos.y * GAME_SCREEN_HEIGHT - ballPos.y;
		
		if (dx*dx + dy*dy > BALL_HOOK_RADIUS*BALL_HOOK_RADIUS)
			goto retNull;
		
		
		// If here, ball is taken by the hook
		if (lastHolder < PLAYER_COUNT/2) {
			game->roomList.scoreA++;
		} else {
			game->roomList.team[roomY] |= roomMask;
			game->roomList.scoreB++;
			pos.y += 64; // to say teamB won
		}
	
		game->roomList.used[roomY] |= roomMask;
	
	
		// Give point to the player
		game->players[lastHolder].privateScore++;


		Ball_exit(&game->ball, BALL_COULDOWN_WIN);
	
		return pos;
	}

	
	retNull:
	Vector_short ret = {.x = 0x7fff};
	return ret;
}

