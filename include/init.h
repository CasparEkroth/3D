#ifndef INIT_H
#define INIT_H

#include <SDL.h>
#include <stdbool.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
//#include <SDL_net.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>

#define STARTING_WINDOW_WIDTH 800 
#define STARTING_WINDOW_HEIGHT 600

typedef struct{
    SDL_Window* pWindow;
    SDL_Renderer *pRend;
    bool keys[SDL_NUM_SCANCODES];
    SDL_Point mouse_pos;
    bool programIsRunning;
    float fElapsedTime;
}Game;




bool initialize_window(Game *pGame);


void input(SDL_Event *event,Game* pGame);

void close_SDL(SDL_Window* pWindow,SDL_Renderer *pRend, Game *pGame);


#endif
