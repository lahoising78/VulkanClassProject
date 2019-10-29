#ifndef _GF3D_GUI_H_
#define _GF3D_GUI_H_

#include "gf3d_shape.h"
#include <vulkan/vulkan.h>

typedef struct GuiElement_S
{
    Shape shape;
    Vector3D offset;
    Vector3D size;
    float *val;
    float *max;
    Uint8 _inuse;
} GuiElement;


int gf3d_gui_manager_init(Uint32 count);

void gf3d_gui_manager_update();

void gf3d_gui_manager_draw(Uint32 bufferFrame, VkCommandBuffer commandBuffer);

GuiElement *gf3d_gui_new();

void gf3d_gui_free(GuiElement *gui);

// void gf3d_gui_update(GuiElement *gui);

// void gf3d_gui_draw(GuiElement *gui, Uint32 bufferFrame, VkCommandBuffer commandBuffer);

#endif