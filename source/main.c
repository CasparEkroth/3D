#include "../include/init.h"
#define NAME 40

int main(int argc, char **argv){
    (void)argc;
    (void)argv;
    Game g = {0};
    SDL_Event event = {0};
    if(initialize_window(&g)){
    g.programIsRunning = true;
    }else g.programIsRunning = false;
    
    while (g.programIsRunning){
        input(event,&g);
        update(&g);
        render(&g);
    }
    close_SDL(g.pWindow,g.pRenderere,&g);
    return 0;
}
