#include "bots.h"
#include "Game.h"

void bots_init() {

}

void bots_cleanup() {

}

void bots_run(void* __game__, ushort playerIndex, ushort level) {
	Game* const game = __game__;

	printf("Bot runs in orb as %d with level %d\n", playerIndex, level);
}



void bots_learn(void* __game__) {
	Game* const game = __game__;

}
