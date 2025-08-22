// clear && gcc main.c BotAI.c -g -lm -o main `sdl2-config --cflags --libs` && ./main 
// clear && gcc main.c BotAI.c -g -lm -o main && ./main 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "BotAI.h"






enum {BOT_COUNT = 100};
enum {LAPS = 350};
enum {SIM_NUM = 500};

structdef(Game);

struct Game {
	float goalX;
	float goalY;
	float goalDirX;
	float goalDirY;

	bool printResult;
	int lap;

	float px[BOT_COUNT];
	float py[BOT_COUNT];
};

BotPoolAI_Inputer botInputer;
BotPoolAI_Runner botRunner;



float Q_rsqrt( float number )
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;						// evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

void botLap(void* data) {
	Game* game = data;
	game->goalX += game->goalDirX;
	game->goalY += game->goalDirY;
	game->lap++;
}

float botRunner(const float* output, void* data, uint index) {
	Game* game = data;

	
	
	float vx = output[0]*2-1;
	float vy = output[1]*2-1;
	

	// float vx = output[0];
	// float vy = output[1];


	float norm = vx*vx + vy*vy;

	norm =  Q_rsqrt(norm);
	vx *= norm;
	vy *= norm;
	game->px[index] += vx;
	game->py[index] += vy;


	float dx = game->px[index] - game->goalX;
	float dy = game->py[index] - game->goalY;


	if (game->lap != LAPS) {
		// return 0;
	}

	float dist = sqrt(dx*dx + dy*dy);
	return 100 - 100*tanhf(dist * .003f);
}

#define GOAL_SPEED 1.5f


void botInputer(float* input, const void* data, uint index) {
	const Game* game = data;

	input[0] = 	BotAI_normalize(game->goalX, 0, 1000);
	input[1] = 	BotAI_normalize(game->goalY, 0, 1000);
	input[2] = 	BotAI_normalize(game->px[index], 0, 1000.0f) + .3f;
	input[3] = 	BotAI_normalize(game->py[index], 0, 1000.0f);
	input[4] = 	BotAI_normalize(game->goalDirX, -.5f * GOAL_SPEED, .5f * GOAL_SPEED);
	input[5] = 	BotAI_normalize(game->goalDirY, -.5f * GOAL_SPEED, .5f * GOAL_SPEED);


	// input[0] = 	game->goalX;
	// input[1] = 	game->goalY;
	// input[2] = 	game->px[index];
	// input[3] = 	game->py[index];
	// input[4] = 	game->goalDirX;
	// input[5] = 	game->goalDirY;

}

void botPrinter(const void* data, uint index) {
	const Game* game = data;
	printf("%+7.2f  %+7.2f", game->px[index], game->py[index]);
}

void botInit(void* data) {
	Game* game = data;
	game->printResult = false;
	game->goalX = 400.0f;
	game->goalY = 225.0f;
	game->goalDirX = GOAL_SPEED * (((float)rand()/RAND_MAX) - .5f);
	game->goalDirY = GOAL_SPEED * (((float)rand()/RAND_MAX) - .5f);

	game->lap = 0;
	for (int i = 0; i < BOT_COUNT; i++) {
		game->px[i] = 420.0f;
		game->py[i] = 235.0f;
	}
}



// #define DRAW_RESULTS

#ifdef DRAW_RESULTS
	#include <SDL2/SDL.h>
#endif

#include <time.h>

int main() {
	// srand(time(NULL));
	const ushort layers[] = {6, 6, 2};

	BotPoolAI pool;
	BotPoolAI_createEmpty(&pool, layers, sizeof(layers)/sizeof(layers[0]), BOT_COUNT);

	Game game;


	
	
	#ifndef DRAW_RESULTS
	{
		botInit(&game);
		botLap(&game);
		pool.botLength = 1;
		game.printResult = true;
		BotPoolAI_run(&pool, &game, botRunner, botInputer);
		pool.botLength = BOT_COUNT;
	}
	#else 
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			printf("Erreur SDL: %s\n", SDL_GetError());
			return 1;
		}

		SDL_Window* window = SDL_CreateWindow("Network test",
											SDL_WINDOWPOS_CENTERED,
											SDL_WINDOWPOS_CENTERED,
											800, 450,
											SDL_WINDOW_SHOWN);

		SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

		int quit = 0;
		SDL_Event event;

		// Reset game
		ushort simLap = 0;
		botInit(&game);
		game.printResult = true;
		int frame = 0;

		while (!quit) {
			// Gérer les événements
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT) {
					quit = 1;
				}
			}

			// Effacer l'écran (noir)
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);

			SDL_Rect rect;
			rect.w = 10;
			rect.h = 10;


			frame++;


			if (frame > LAPS) {
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			} else {
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			}

			for (uint i = 1; i < BOT_COUNT; i++) {
				rect.x = game.px[i];
				rect.y = game.py[i];
				SDL_RenderFillRect(renderer, &rect);
			}

			SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
			rect.x = game.px[0];
			rect.y = game.py[0];
			SDL_RenderFillRect(renderer, &rect);

			SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
			rect.x = game.goalX;
			rect.y = game.goalY;
			SDL_RenderFillRect(renderer, &rect);


			botLap(&game);

			BotPoolAI_run(&pool, &game, botRunner, botInputer);

			// Afficher le rendu
			SDL_RenderPresent(renderer);

			SDL_Delay(5);
			
			if (frame > LAPS) {
				game.printResult = false;
				frame = 0;
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
					20,
					5,
					0
				);
				

				botInit(&game);
				game.printResult = true;




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

			}

		}

		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 0;
	}
	#endif

	

	BootPoolAI_cleanup(&pool);
	printf("Success\n");
}

