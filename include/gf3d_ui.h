#ifndef _GF3D_UI_H_
#define _GF3D_UI_H_

#include "gf3d_vgraphics.h"
#include "SDL2/SDL.h"
#include "gf3d_texture.h"

typedef struct 
{
    SDL_Surface             *surface;
    SDL_Renderer            *renderer;
    Texture                 *texture;

    uint32_t                count;

    uint8_t                 active;
    uint8_t                 visible;
    uint8_t                 _inuse;
} UILayer;

void gf3d_ui_manager_init(uint32_t count);

UILayer *gf3d_ui_layer_new();
void gf3d_ui_layer_free(UILayer *layer);

#endif