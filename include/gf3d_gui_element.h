#ifndef _GF3D_GUI_ELEMENT_H_
#define _GF3D_GUI_ELEMENT_H_

#include "gfc_vector.h"
#include <vulkan/vulkan.h>
#include "gf3d_texture.h"
#include "gf3d_swapchain.h"

typedef struct 
{
    int r;
    int g;
    int b;
    int a;
} ColorRGBA;

typedef struct
{
    Vector2D position;
    Vector2D extents;
    ColorRGBA color;
} GuiElement;

void gf3d_gui_element_set_square_tex(Texture *tex);

ColorRGBA colorRGBA(int r, int g, int b, int a);

GuiElement gf3d_gui_element_create(Vector2D pos, Vector2D extents, ColorRGBA color);

void gf3d_gui_element_draw(GuiElement *e, VkCommandBuffer commandBuffer, VkImage *image);

#endif