#include "../game/Game.h"
#include "../game/Projectile.h"
#include <SDL2/SDL.h>

void draw(SDL_Renderer* renderer, Game* game, float camX, float camY, float camZoom) {
	const int screenCenterX = 1600 / 2;
	const int screenCenterY = 900 / 2;

	SDL_SetRenderDrawColor(renderer, 255, 255, 63, 255);

	enum {PSIZE = PROJECTILE_SIZE/6};

	Array_loop(Projectile, game->projectiles, p) {
		SDL_Rect rect = {
			(int)(((p->x - PSIZE / 2.0f - camX) * camZoom) + screenCenterX),
			(int)(((p->y - PSIZE / 2.0f - camY) * camZoom) + screenCenterY),
			(int)(PSIZE * camZoom),
			(int)(PSIZE * camZoom)
		};
		SDL_RenderFillRect(renderer, &rect);
	}
	
	for (int i = 0; i < PLAYER_COUNT; ++i) {
		if (game->players[i].respawnCouldown > 0)
			continue;
			
		SDL_Color color = (i < PLAYER_COUNT / 2) ? (SDL_Color){0, 0, 255} : (SDL_Color){255, 0, 0};
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
		SDL_Rect rect = {
			(int)(((game->players[i].x - PLAYER_SIZE / 2.0f - camX) * camZoom) + screenCenterX),
			(int)(((game->players[i].y - PLAYER_SIZE / 2.0f - camY) * camZoom) + screenCenterY),
			(int)(PLAYER_SIZE * camZoom),
			(int)(PLAYER_SIZE * camZoom)
		};
		SDL_RenderFillRect(renderer, &rect);
	}

	

	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_Rect ballRect = {
		(int)(((game->ball.x - BALL_SIZE / 2.0f - camX) * camZoom) + screenCenterX),
		(int)(((game->ball.y - BALL_SIZE / 2.0f - camY) * camZoom) + screenCenterY),
		(int)(BALL_SIZE * camZoom),
		(int)(BALL_SIZE * camZoom)
	};

	SDL_RenderFillRect(renderer, &ballRect);
}
