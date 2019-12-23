#ifndef _GF3D_UI_H_
#define _GF3D_UI_H_

#include "SDL2/SDL.h"
#include <vulkan/vulkan.h>

typedef struct
{
    SDL_Surface *surface;
    SDL_Renderer *renderer;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    uint8_t _inuse;
} uiLayer;

void gf3d_ui_manager_init(uint32_t count);

uiLayer *gf3d_ui_new();

void gf3d_ui_free( uiLayer *layer );

#endif