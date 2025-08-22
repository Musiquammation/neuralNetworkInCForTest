// clear && gcc main.c  -g -lm -ltools -o main `sdl2-config --cflags --libs` -lSDL2_ttf && ./main 
// clear && gcc main.c  -g -lm -ltools -o main && ./main 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "BotAI.h"


#include "../game/Game.h"
#include "../game/Projectile.h"
#include "../game/Player.h"




enum {BOT_COUNT = 64};
enum {LAPS = 500};
enum {SIM_NUM = 300};

structdef(GameHandler);

struct GameHandler {
	Game games[BOT_COUNT/4];
	SI_Handler handler;
};

BotPoolAI_Inputer botInputer;
BotPoolAI_Runner botRunner;




void botLap(void* data) {
	GameHandler* gh = data;
	
	
	Array_for(Game, gh->games, BOT_COUNT/4, game) {		
		gh->handler.commonMessages.length = 0;
		gh->handler.privateMessages.length = 0;
		gh->handler.privateMsgPlayerIndexes.length = 0;
		game_run(game, &gh->handler);
	}
	
}

float botRunner(const float* output, void* data, uint index) {
	GameHandler* gh = data;
	Game* game = &gh->games[index / 4];
	Player* player = &game->players[index % 4];

	float v;

	// Mouvement gauche/droite
	v = output[0];
	player->dirX = v * 2 - 1;

	// Saut
	v = output[1];
	if (v > 0.7f) {
		player->actionMask |= PLAYER_ACTION_MASK_JUMP;
		player->dirY = 0;
	} else if (v < 0.1f) {
		player->dirY = 1;
	} else {
		player->dirY = 0;
	}

	// Attaque (non prioritaire ici)
	v = output[2];
	if (v > 0.7f) {
		player->attack.attacking = 1;
		player->attack.x = player->x + output[3];
		player->attack.y = player->y + output[4];
		player->attack.x = 0;
		player->attack.y = 0;
	}

	// float score = player->privateScore;

	if (player->respawnCouldown > 0)
		return -3000.0f;

	if (game->ball.couldown > 0)
		return -1.0f;
	
	if (game->ball.holdBy == index%4)
		return 2.3f;

	// Calcul de distance
	float dx = game->ball.x - player->x;
	float dy = game->ball.y - player->y;
	float dist2 = dx * dx + dy * dy;

	enum {D = 1500};
	if (dist2 > D*D)
		return 0;

	return 1 - dist2/(D*D);
}

void botInputer(float* input, const void* data, uint index) {
	const GameHandler* gh = data;
	const Game* game = &gh->games[index/4];


	switch ((index) % 4) {
	case 0:
		input[0]  = game->players[0].x;
		input[1]  = game->players[0].y;
		input[2]  = game->players[0].dirX;
		input[3]  = game->players[0].sy;

		input[4]  = game->players[1].x;
		input[5]  = game->players[1].y;
		input[6]  = game->players[1].dirX;
		input[7]  = game->players[1].sy;

		input[8]  = game->players[2].x;
		input[9]  = game->players[2].y;
		input[10] = game->players[2].dirX;
		input[11] = game->players[2].sy;

		input[12] = game->players[3].x;
		input[13] = game->players[3].y;
		input[14] = game->players[3].dirX;
		input[15] = game->players[3].sy;
		break;

	case 1:
		input[0]  = game->players[1].x;
		input[1]  = game->players[1].y;
		input[2]  = game->players[1].dirX;
		input[3]  = game->players[1].sy;

		input[4]  = game->players[0].x;
		input[5]  = game->players[0].y;
		input[6]  = game->players[0].dirX;
		input[7]  = game->players[0].sy;

		input[8]  = game->players[2].x;
		input[9]  = game->players[2].y;
		input[10] = game->players[2].dirX;
		input[11] = game->players[2].sy;

		input[12] = game->players[3].x;
		input[13] = game->players[3].y;
		input[14] = game->players[3].dirX;
		input[15] = game->players[3].sy;
		break;

	case 2:
		input[0]  = game->players[2].x;
		input[1]  = game->players[2].y;
		input[2]  = game->players[2].sx;
		input[3]  = game->players[2].sy;

		input[4]  = game->players[3].x;
		input[5]  = game->players[3].y;
		input[6]  = game->players[3].sx;
		input[7]  = game->players[3].sy;

		input[8]  = game->players[0].x;
		input[9]  = game->players[0].y;
		input[10] = game->players[0].sx;
		input[11] = game->players[0].sy;

		input[12] = game->players[1].x;
		input[13] = game->players[1].y;
		input[14] = game->players[1].sx;
		input[15] = game->players[1].sy;
		break;

	case 3:
		input[0]  = game->players[3].x;
		input[1]  = game->players[3].y;
		input[2]  = game->players[3].sx;
		input[3]  = game->players[3].sy;

		input[4]  = game->players[2].x;
		input[5]  = game->players[2].y;
		input[6]  = game->players[2].sx;
		input[7]  = game->players[2].sy;

		input[8]  = game->players[0].x;
		input[9]  = game->players[0].y;
		input[10] = game->players[0].sx;
		input[11] = game->players[0].sy;

		input[12] = game->players[1].x;
		input[13] = game->players[1].y;
		input[14] = game->players[1].sx;
		input[15] = game->players[1].sy;
		break;
	}


	// Normalize player positions and speeds
	for (int i = 0; i < 16; i += 4) {
		input[i]     = BotAI_normalize(input[i], -GAME_FULL_WIDTH / 2, GAME_FULL_WIDTH / 2);     // x
		input[i + 1] = BotAI_normalize(input[i + 1], -GAME_FULL_HEIGHT / 2, GAME_FULL_HEIGHT / 2); // y
		input[i + 2] = BotAI_normalize(input[i + 2], -1, 1); // sx
		input[i + 3] = BotAI_normalize(input[i + 3], -800, 800); // sy
	}

	input[16] = BotAI_normalize(game->ball.x, -GAME_FULL_WIDTH / 2, GAME_FULL_WIDTH / 2);
	input[17] = BotAI_normalize(game->ball.y, -GAME_FULL_HEIGHT / 2, GAME_FULL_HEIGHT / 2);

	float v = game->ball.couldown;
	input[18] = v < 0 ? 0 : v;

	input[19] = BotAI_normalize(game->ball.sx, -800, 800);
	input[20] = BotAI_normalize(game->ball.sy, -800, 800);

	input[21] = (float)(game->ball.holdBy == (uchar)index);

	// Encode roomList states
	for (int i = 0; i < 7; i++) {
		if (i == 3) {
			input[22 + 3] = (game->roomList.used[0] & 8) ? 0.0f : 0.7f;
			continue;
		}

		char top = game->roomList.used[0] & (1 << i);
		char mid = game->roomList.used[1] & (1 << i);
		char bot = game->roomList.used[2] & (1 << i);

		if (top && mid && bot) {
			v = 0.0f;
		} else if (top && mid) {
			v = 0.1f;
		} else if (mid && bot) {
			v = 0.2f;
		} else if (bot && top) {
			v = 0.3f;
		} else if (top) {
			v = 0.4f;
		} else if (mid) {
			v = 0.5f;
		} else if (bot) {
			v = 0.6f;
		} else {
			v = 0.7f;
		}

		input[22 + i] = BotAI_normalize(v, 0, 0.7f);
	}

	input[29] = (float)game->players[index%4].jumps;


	// for (int i = 0; i < 29; i++) {printf("input[%d]: %f\n", i, input[i]);}

}



void botPrinter(const void* data, uint index) {
	const GameHandler* gh = data;
	
}
	

void botInit(void* data) {
	GameHandler* gh = data;

	Array_for(Game, gh->games, BOT_COUNT/4, game) {
		game->chrono = 0;
		game->ball.x = 0;
		game->ball.y = 0;
		game->ball.sx = 0;
		game->ball.sy = -BALL_SPAWN_JUMP;
		game->ball.couldown = .1f;
		game->ball.lastHolder = BALL_NOT_HOLD;
		memset(&game->roomList, 0, sizeof(game->roomList));
	
	
		game->projectiles.length = 0;

		for (ushort i = 0; i < PLAYER_COUNT/2; i++) {
			Player_init(&game->players[i], true, i);
		}
	
		for (ushort i = PLAYER_COUNT/2; i < PLAYER_COUNT; i++) {
			Player_init(&game->players[i], false, i+(PLAYER_COUNT/2));
		}
	}


}






#define DRAW_RESULTS


BotPoolAI pool;
GameHandler game;


#ifdef DRAW_RESULTS
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <pthread.h>


void draw(SDL_Renderer* renderer, Game* game, float camX, float camY, float camZoom);

ushort learnLap = NULL_USHORT;
ushort simLap = 0;
int frame = LAPS;

void* learnThread(void*) {
	BotPoolAI_learn(
		pool,
		botInputer,
		botRunner,
		botPrinter,
		botInit,
		botLap,
		&game,
		LAPS,
		SIM_NUM,
		SIM_NUM * simLap,
		12,
		4,
		0,
		&learnLap
	);

	simLap++;
	frame = 0;
	learnLap = NULL_USHORT;
	return NULL;
}



#endif

#include <time.h>




int main() {
	srand(time(NULL));
	const ushort layers[] = {30, 8, 8, 5};

	BotPoolAI_createEmpty(&pool, layers, sizeof(layers)/sizeof(layers[0]), BOT_COUNT, true);
	BotPoolAI_openFile(&pool, "data/r");

	Array_for(Game, game.games, BOT_COUNT/4, g) {
		Array_create(&g->projectiles, sizeof(Projectile));
	}



	game.handler.speed = .1f; // 10fps
	game.handler.willFinish = false;
	game.handler.results.gameFinished = false;
	Array_create(&game.handler.commonMessages, sizeof(char*));
	Array_create(&game.handler.privateMessages, sizeof(void*));
	Array_create(&game.handler.privateMsgPlayerIndexes, sizeof(ushort));


	
	
	#ifndef DRAW_RESULTS
	{
		botInit(&game);
		botLap(&game);
		pool.botLength = 1;
		BotPoolAI_run(&pool, &game, botRunner, botInputer);
		pool.botLength = BOT_COUNT;
	}

	
	#else 
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			printf("SDL_Init: %s\n", SDL_GetError());
			return 1;
		}

		if (TTF_Init() == -1) {
			printf("TTF_Init: %s\n", TTF_GetError());
			return 1;
		}

		TTF_Font* font = TTF_OpenFont(
			"/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
			24
		);
		if (!font) {
			printf("TTF_OpenFont: %s\n", TTF_GetError());
			exit(1);
		}

		

		SDL_Window* window = SDL_CreateWindow("Network test",
											SDL_WINDOWPOS_CENTERED,
											SDL_WINDOWPOS_CENTERED,
											1600, 900,
											SDL_WINDOW_SHOWN);

		SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

		pthread_t thread;

		// Reset game
		botInit(&game);
		
		bool runLoop = true;
		bool dragging = false;
		int lastMouseX = 0;
		int lastMouseY = 0;
		float camX = 0;
		float camY = 0;
		float camZoom = .2f;


		while (runLoop) {
			// Gérer les événements
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_QUIT:
						runLoop = false;
						break;
					
						case SDL_KEYDOWN:
						if (event.key.keysym.sym == SDLK_ESCAPE)
							runLoop = false;
						
						break;

					case SDL_MOUSEWHEEL: {
						int mouseX, mouseY;
						SDL_GetMouseState(&mouseX, &mouseY);

						const int screenCenterX = 1600 / 2;
						const int screenCenterY = 900 / 2;

						// Position souris en coordonnées monde
						float worldX = camX + (mouseX - screenCenterX) / camZoom;
						float worldY = camY + (mouseY - screenCenterY) / camZoom;

						float zoomFactor = (event.wheel.y > 0) ? 1.1f : 0.9f;
						camZoom *= zoomFactor;

						// Adapter camX, camY pour garder worldX/worldY sous la souris
						camX = worldX - (mouseX - screenCenterX) / camZoom;
						camY = worldY - (mouseY - screenCenterY) / camZoom;
						break;
					}

					case SDL_MOUSEBUTTONDOWN:
						if (event.button.button == SDL_BUTTON_MIDDLE) {
							dragging = true;
							lastMouseX = event.button.x;
							lastMouseY = event.button.y;
						}
						break;
					case SDL_MOUSEBUTTONUP:
						if (event.button.button == SDL_BUTTON_MIDDLE) {
							dragging = false;
						}
						break;
					case SDL_MOUSEMOTION:
						if (dragging) {
							int dx = event.motion.x - lastMouseX;
							int dy = event.motion.y - lastMouseY;
							camX -= dx / camZoom;
							camY -= dy / camZoom;
							lastMouseX = event.motion.x;
							lastMouseY = event.motion.y;
						}
						break;
				}
			}

			botLap(&game);
			BotPoolAI_run(&pool, &game, botRunner, botInputer);


			// Effacer l'écran (noir)
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);
			
			if (learnLap == NULL_USHORT) {
				draw(renderer, &game.games[0], camX, camY, camZoom);
			} else {
				frame = learnLap;
			}

			{
				char bff[32];
				sprintf(bff, "%03d/%03d", frame, learnLap == NULL_USHORT ? LAPS : SIM_NUM);

				SDL_Color white = {255, 255, 255, 255};
				SDL_Surface* surface = TTF_RenderText_Blended(font, bff, white);
				SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

				SDL_Rect dstRect = {10, 10, surface->w, surface->h};
				SDL_RenderCopy(renderer, texture, NULL, &dstRect);

				SDL_FreeSurface(surface);
				SDL_DestroyTexture(texture);
			}

			SDL_RenderPresent(renderer);

			
			
			// Afficher le rendu
			if (learnLap == NULL_USHORT) {
				frame++;
				SDL_Delay(40);
			} else {
				frame = 0;
				SDL_Delay(1000);
			}
			
			if (learnLap == NULL_USHORT && frame >= LAPS) {
				frame = 0;

				botInit(&game);
				BotPoolAI_saveFile(&pool, "data/new");

				pthread_create(&thread, NULL, learnThread, NULL);
				pthread_detach(thread);

				/*
				FILE *file = fopen("weights.txt", "w");
				if (file == NULL) {
					perror("Cannot open file");
					return 1;
				}


				const ushort* const layer_last = &layers[pool.layerLength];
				ushort last = layers[0];
				ushort bufferSize = last;
				float* w = pool.botWeights[0];
				ushort i = 0;
				for (const ushort* ptr = &layers[1]; ptr < layer_last; ptr++) {
					ushort current = *ptr;
					if (current > bufferSize) {
						bufferSize = current;
					}

					uint weightLength = current * (last+1);
					
					fprintf(file, "FROM %02d TO %02d\n", last, current);
					
					float* end = &w[weightLength];
					
					for (; w < end; w++)
						fprintf(file, "\t%+f\n", *w);
					
					last = current;
					i++;
				}

				// Ferme le file
				fclose(file);
				*/

			}

		}

		TTF_CloseFont(font);
		TTF_Quit();


		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 0;
	}
	#endif

	

	BootPoolAI_cleanup(&pool);
	printf("Success\n");
}




#include "../game/Game.c"
#include "BotAI.c"


#ifdef DRAW_RESULTS
	#include "draw.c"
#endif