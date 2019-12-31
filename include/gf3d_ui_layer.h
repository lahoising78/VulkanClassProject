#ifndef _GF3D_UI_LAYER_H_
#define _GF3D_UI_LAYER_H_

#include "gf3d_vgraphics.h"
#include "SDL2/SDL.h"
#include "gf3d_texture.h"
#include "gf3d_ui.h"

typedef struct 
{
    SDL_Surface             *surface;
    SDL_Renderer            *renderer;
    Texture                 *texture;

    VkBuffer                stagingBuffer;
    VkDeviceMemory          stagingBufferMemory;
    VkBuffer                vertexBuffer;
    VkDeviceMemory          vertexBufferMemory;

    uint32_t                count;
    UI                      *uiList;

    uint8_t                 active;
    uint8_t                 visible;
    uint8_t                 _inuse;
} UILayer;

void gf3d_ui_manager_init(uint32_t count);
void gf3d_ui_manger_render(uint32_t bufferFrame, VkCommandBuffer commandBuffer);

UILayer *gf3d_ui_layer_new();
void gf3d_ui_layer_free(UILayer *layer);
UI *gf3d_ui_layer_get_ui(UILayer *layer);

#endif