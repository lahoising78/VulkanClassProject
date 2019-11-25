#include "app_editor.h"

#include "simple_logger.h"

#include "gf3d_gui.h"

#define MAX_INPUT_KEY SDL_NUM_SCANCODES

int app_editor_main(int argc, char *argv[])
{
    uint8_t running = 1;

    SDL_Event keys[ MAX_INPUT_KEY ];
    SDL_Event e;

    slog("hello there");

    while(running)
    {
        memset(keys, 0, sizeof(keys));

        while( SDL_PollEvent(&e) )
        {
            if( e.key.keysym.scancode < MAX_INPUT_KEY )
                keys[ e.key.keysym.scancode ] = e;
        }

        if( keys[ SDL_SCANCODE_ESCAPE ].key.type == SDL_KEYDOWN )
        {
            running = 1;
        }
    }

    return 0;
}