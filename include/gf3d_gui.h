#ifndef _GF3D_GUI_H_
#define _GF3D_GUI_H_

#include "gf3d_gui_element.h"
#include <vulkan/vulkan.h>

typedef struct
{
    Vector2D pos;
    Vector2D extents;

    GuiElement **elements;
    // uint32_t elementCount;

    uint8_t _inuse;
} GuiLayer;

void gf3d_gui_manager_init(uint32_t count);

void gf3d_gui_manager_draw(uint32_t bufferFrame, VkCommandBuffer commandBuffer);

void gf3d_gui_layer_free(GuiLayer *gui);
GuiLayer *gf3d_gui_layer_new();

/* 
    @return -1 if there is no more space, or the index it was allocated on
 */
int gf3d_gui_layer_add_element(GuiLayer *gui, GuiElement *element);

#endif