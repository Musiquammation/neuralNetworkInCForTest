#include "rooms.h"

#include "Game.h"
#include "Ball.h"

#include <math.h>


Vector_short RoomList_getCenterCoords(float x, float y) {
	Vector_short result = {
		(short)floorf(x * (1.0f/GAME_SCREEN_WIDTH) + .5f),
		(short)floorf(y * (1.0f/GAME_SCREEN_HEIGHT) + .5f)
	};
	return result;
}

char RoomList_getValue(RoomList list, Vector_short room) {
	if (room.x == 0 && room.y != 1)
		return 127; // undef
	
	
	room.y += 1;


	const char mask = 1 << (room.x+3);
	if ((list.used[room.y] & mask) == 0)
		return 0; // unused
	
	return (list.team[room.y] & mask) == 0 ? -1 : 1;
}


void RoomList_setValue(RoomList* list, Vector_short room, bool teamB) {
	if (room.x == 0 && room.y != 1)
		return; // undef


	room.y += 1;
	const char mask = 1 << (room.x+3);
	list->used[room.y] |= mask;
	
	if (teamB)
		list->used[room.y] |= mask;
	
}




