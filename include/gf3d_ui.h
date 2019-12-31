#ifndef _GF3D_UI_H_
#define _GF3D_UI_H_

#include "gfc_vector.h"
#include <SDL2/SDL.h>

typedef struct
{
    SDL_Rect                        rect;
    Vector4D                        color;

    uint8_t                         active;
    uint8_t                         visible;

    uint8_t                         _inuse;
} UI;

void gf3d_ui_free(UI *ui);
void gf3d_ui_render(UI *ui, SDL_Renderer *renderer);

#endif