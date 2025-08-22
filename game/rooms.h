#ifndef GAMES_ORB_ROOMS_H_
#define GAMES_ORB_ROOMS_H_

#include <tools/tools.h>

#include "coords.h"

structdef(RoomList);
structdef(RoomList_Result);

enum {
	FINAL_ROOM_COUNT = 19
};

struct RoomList {
	char used[3];
	char team[3];
	char scoreA;
	char scoreB;
};

struct RoomList_Result {
	short teamA;
	short teamB;
};



Vector_short RoomList_getCenterCoords(float x, float y);
char RoomList_getValue(RoomList list, Vector_short room);
void RoomList_setValue(RoomList* list, Vector_short room, bool teamB);



#endif