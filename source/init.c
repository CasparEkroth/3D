#include "../include/init.h"

bool initialize_window(Game *pGame){ // Initialiserar SDL och skapar fönster
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0 || TTF_Init() != 0 ){
        fprintf(stderr, "Error initializing SDL. %s\n", SDL_GetError());
        return false;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        fprintf(stderr, "Error initializing SDL_image: %s\n", IMG_GetError());
        return false;
    }
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
        fprintf(stderr,"SDL_mixer could not initialize! Mix_Error: %s\n",Mix_GetError());
        return false;
    }
    pGame->pWindow = SDL_CreateWindow(
        NULL, // Titel
        SDL_WINDOWPOS_CENTERED, // x
        SDL_WINDOWPOS_CENTERED, // y 
        STARTING_WINDOW_WIDTH, 
        STARTING_WINDOW_HEIGHT, 
        SDL_WINDOW_RESIZABLE  // Flags
    );
    if (!pGame->pWindow) {
        fprintf(stderr, "Error creating SDL Window: %s\n", SDL_GetError());
        return false;
    }
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    pGame->pRend = SDL_CreateRenderer(pGame->pWindow, -1, render_flags);
    if (!pGame->pRend) {
        fprintf(stderr, "Error creating SDL Renderer: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

void close_SDL(SDL_Window* pWindow,SDL_Renderer *pRenderer, Game *pGame){
    if (pRenderer) SDL_DestroyRenderer(pRenderer);
    if (pWindow) SDL_DestroyWindow(pWindow);
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void input(SDL_Event *event,Game* pGame,float *fYaw, float* fPitch){
    while (SDL_PollEvent(event)){
        switch (event->type){
        case SDL_QUIT:
            pGame->programIsRunning = false;
            break;
        case SDL_KEYDOWN: 
            pGame->keys[event->key.keysym.scancode] = true;
            break;
        case SDL_KEYUP:
            pGame->keys[event->key.keysym.scancode] = false; 
            break;
        case SDL_MOUSEMOTION:
            // e.motion.xrel, yrel are relative movement since last frame
            const float sensitivity = 0.0025f; // tweak to taste
            *fYaw   += (float)(event->motion.xrel * sensitivity);
            *fPitch -= (float)(event->motion.yrel * sensitivity);
            // clamp pitch so you don't flip upside-down
            if (*fPitch >  1.5f) *fPitch =  1.5f;
            if (*fPitch < -1.5f) *fPitch = -1.5f;
            break;
        default:
            break;
        }
    }
    if(pGame->keys[SDL_SCANCODE_ESCAPE]) pGame->programIsRunning = false;
}


