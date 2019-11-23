#ifndef _GF3D_GUI_ELEMENT_H_
#define _GF3D_GUI_ELEMENT_H_

#include "gfc_vector.h"
#include "gfc_color.h"
#include "gf3d_texture.h"

enum GuiElementType
{
    GF3D_GUI_IMAGE = 0,
    GF3D_GUI_RECT = 1
};

typedef struct
{
    Vector2D pos;
    Vector4D color;
    Vector2D texel;
} GuiVertex;

typedef struct gui_element_t
{
    Vector2D position;
    Vector2D extents;
    Vector4D color;
    Texture *tex;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    GuiVertex vertices[4];
} GuiElement;

void gf3d_gui_element_set_vk_device(VkDevice device);

GuiElement *gf3d_gui_element_create(Vector2D pos, Vector2D ext, Vector4D color);
void gf3d_gui_element_free(GuiElement *e);

void gf3d_gui_element_draw(GuiElement *element, VkCommandBuffer commandBuffer);
// void gf3d_gui_element_draw(GuiElement *element, SDL_Renderer *renderer);

#endif