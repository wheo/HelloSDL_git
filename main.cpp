#include <iostream>
#include "SDL_main.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include "SDL_image.h"
#include "Game.h"

int main(int argc, char* argv[]) {

	srand(SDL_GetTicks()); // Initnalizing random seed
	Game game(argc, argv); // Creating Game	
	return game.Loop();
}