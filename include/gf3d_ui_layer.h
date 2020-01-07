#ifndef _GF3D_UI_LAYER_H_
#define _GF3D_UI_LAYER_H_

#include "gf3d_texture.h"

typedef struct
{
    Texture                     *display;
    uint32_t                    count;

    uint8_t                     visible;
    uint8_t                     active;
    
    uint8_t                     _inuse;
} UILayer;

void gf3d_ui_manager_init(uint32_t count);
void gf3d_ui_manager_draw(uint32_t bufferFrame, VkCommandBuffer commandBuffer);

#endif