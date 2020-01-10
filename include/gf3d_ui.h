#ifndef _GF3D_UI_H_
#define _GF3D_UI_H_

#include "gf3d_ui_component.h"

typedef struct
{
    Vector2D position;
    Vector2D extents;

    uiComponent *components;
    uint8_t count;

    uint8_t visible;
    uint8_t active;

    uint8_t _inuse;
} uiLayer;

/* UI MANAGER FUNCTIONS */

void gf3d_ui_manager_init(uint32_t count);
void gf3d_ui_manager_update();
void gf3d_ui_manager_draw( uint32_t bufferFrame, VkCommandBuffer commandBuffer );

/* UI LAYER FUNCTIONS */

uiLayer *gf3d_ui_new();
void gf3d_ui_free( uiLayer *layer );

uiComponent *gf3d_ui_get_component(uiLayer *layer);

#endif