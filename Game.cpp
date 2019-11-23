 //
//  Game.cpp
//  Arkanoid
//
//  Created by Maciej Żurad on 11/18/12.
//  Copyright (c) 2012 Maciej Żurad. All rights reserved.
//

#include "Game.h"

Game::~Game(){
    
    delete fps_counter;
    delete game_state;
    
    closeSystems();
}

Game::Game(int argc, char** argv){
    
    music = Mix_LoadMUS("./data/sounds/music.mp3");
    sound = Mix_LoadWAV("./data/sounds/sfx.wav");
#if SDL1
    SDL_WM_SetCaption("ARKANOID", NULL);
#else
	
#endif
    
#if 0
    screen_w = atoi(argv[1]);
    screen_h = atoi(argv[2]);
#else
	screen_w = 1024;
	screen_h = 768;
#endif
    
    if(initSystems() == -1)
        cerr << "Problem occured while initializing SDL systems" << endl;
    
    font = TTF_OpenFont("./data/mainfont.ttf", 35);
	
    if(!font) {
        cerr << "Could not load font " << TTF_GetError << endl;
        exit(1);
    }
    
    running = true ;
    
    //just for now
    gameFPS = 60;
    musicOn = false;
    sfxOn = false;
    control_type = KEYBOARD;
    current_state = MENU;    
    
    fps_counter = new FpsCounter(gameFPS);

    game_state = new MenuState();
    game_state->InitState();
    
}

int Game::initSystems(){
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		cerr << "Problem while initializing SDL" << endl;
		return -1;
	}

#if SDL1	
	screen = SDL_SetVideoMode(screen_w, screen_h, 32, SDL_SWSURFACE);
#else
	window = SDL_CreateWindow("",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		screen_w,
		screen_h,
		SDL_SWSURFACE);

	if (window == NULL) {
		cerr << "Window could not be created!" << endl;
	}
	else {
		renderer = SDL_CreateRenderer(window, -1, 0);
		screen = SDL_GetWindowSurface(window);
	}

#endif
	
	if (TTF_Init() < 0) {
		cerr << "Problem initializing SDL_ttf" << endl;
		return -1;
	}
	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
		cerr << "Problem initializing SDL_mixer" << endl;
		return -1;
	}
    Mix_Init(MIX_INIT_MP3);
	return 0;
}

void Game::closeSystems(){
    if(!screen) SDL_FreeSurface(screen);
    if (!font) TTF_CloseFont(font);
    if (!sound) Mix_FreeChunk(sound);
    if (!music) Mix_FreeMusic(music);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
}

int Game::Loop(){
    while(running){
        if(fps_counter->measureFPS()){
            
            HandleEvents();
            SDL_FillRect(screen, NULL, 0);
            
            game_state->UpdateState();
            game_state->RenderState();
            
            if(displayFPS){
                static char buffer[10] = {0};
#if _S
                sprintf(buffer, "%d FPS", fps_counter->getFPS());
#else
				sprintf_s(buffer, sizeof(buffer), "%d FPS", fps_counter->getFPS());
#endif
#if SDL1
                SDL_WM_SetCaption(buffer, NULL);
#else
				SDL_SetWindowTitle(window, buffer);
#endif
            }else
#if SDL1
                SDL_WM_SetCaption("Arkanoid", NULL);
#else
				SDL_SetWindowTitle(window, "Arkanoid");
#endif
#if SDL1
            SDL_Flip(screen);
#else
			SDL_RenderPresent(renderer);
#endif
        }
    }
    return 0;
}


void Game::HandleEvents(){

    SDL_Event event;
    while(SDL_PollEvent(&event))
        if(event.type == SDL_QUIT)
            ShutDown();
#if SDL1
    Uint8* keystates = SDL_GetKeyState(NULL);
#else
	const Uint8* keystates = SDL_GetKeyboardState(NULL);
#endif
    
    if(keystates[SDLK_q])
            ShutDown();
    
    game_state->HandleEvents(keystates, event, control_type);
    
}


void ChangeState(){
    Game* game = g_GamePtr;
    if(game->current_state == PLAYING){
        game->current_state = MENU;
        delete game->game_state;
        game->game_state = new MenuState();
        game->game_state->InitState();
        
    } else if (game->current_state == MENU){
        game->current_state = PLAYING;
        delete game->game_state;
        game->game_state = new PlayingState();
        game->game_state->InitState();
    }
}

void ShutDown(){
    g_GamePtr->running = false;
}

void SwitchFPSVisibility(){
    g_GamePtr->displayFPS = !g_GamePtr->displayFPS;
    dynamic_cast<MenuState*>(g_GamePtr->GetState())->UpdateInfo(SHOWFPS);
}
void SwitchMusic(){
    g_GamePtr->musicOn = !g_GamePtr->musicOn;
    dynamic_cast<MenuState*>(g_GamePtr->GetState())->UpdateInfo(MUSICON);
    
    if(g_GamePtr->musicOn){
        Mix_PlayingMusic();
        Mix_PlayMusic(g_GamePtr->music, -1);
    } else {
        Mix_HaltMusic();
    }
        
}
void SwitchSfx(){
    g_GamePtr->sfxOn = !g_GamePtr->sfxOn;
    dynamic_cast<MenuState*>(g_GamePtr->GetState())->UpdateInfo(SOUNDON);
}

void Game::Draw(SDL_Surface* screen, SDL_Surface* source,int x, int y) {
    SDL_Rect offset = {(Sint16)x, (Sint16)y, 0, 0};
    SDL_Rect clip = {0, 0, (Uint16)source->w, (Uint16)source->h};
    SDL_BlitSurface(source, &clip, screen, &offset);
}

string IntToStr(int n){
    stringstream ss;
    ss << n;
    return ss.str();
}

void DisplayFinishText(int ms, const char* text){
    
    TTF_Font* font = TTF_OpenFont("./data/font.ttf", 70);
    
    int posX = g_GamePtr->GetScreen_W()/2;
    int posY = g_GamePtr->GetScreen_H()/2;
    
    
    SDL_Color color = {0x2b, 0xd7, 0xb7, 0};
    SDL_Color shade = {0xff, 0xff, 0xff, 0};
    
    SDL_Surface* text_image = TTF_RenderText_Solid(font, text, color);
    SDL_Surface* text_shade = TTF_RenderText_Solid(font, text, shade);
    
    Game::Draw(g_GamePtr->GetScreen(), text_shade, posX - text_shade->w/2 +2, posY - text_shade->h/2 +2);
    Game::Draw(g_GamePtr->GetScreen(), text_image, posX - text_image->w/2, posY - text_image->h/2);
    
#if SDL1
    SDL_Flip(g_GamePtr->GetScreen());
#else
	SDL_RenderPresent(g_GamePtr->Get_Renderer());
#endif
    
    int firstMeasure = SDL_GetTicks();
    while(SDL_GetTicks() - firstMeasure <= ms);
    
    SDL_FreeSurface(text_shade);
    SDL_FreeSurface(text_image);
    TTF_CloseFont(font);
}










