#ifndef _GF3D_GUI_H_
#define _GF3D_GUI_H_

#include <SDL2/SDL.h>
#include "gf3d_vgraphics.h"
#include "gf3d_gui_element.h"

typedef struct
{
    Vector2D pos;
    Vector3D color;
    Vector2D texel;
} GuiVertex;

typedef struct
{
    SDL_Surface *surface;
    SDL_Renderer *renderer;
    Texture *ui_tex;
    GuiElement **elements;
    Uint32 elementCount;
    Uint8 _inuse;

    GuiVertex vertices[4];
} Gui;

void gf3d_gui_manager_init(Uint32 count, Pipeline *pipe, VkDevice device);

void gf3d_gui_manager_draw(Uint32 bufferFrame, VkCommandBuffer commandBuffer);

/* 
    @brief create a new gui layer
    @param count : how many elements to allow
    @param depth : layer level of gui
    @note depth -1 means it should be added on top of the already existent layers
 */
Gui *gf3d_gui_new(Uint32 count, int depth);

/* 
    @brief free the gui layer
    @param gui : the layer to free
 */
void gf3d_gui_free(Gui *gui);

void gf3d_gui_add_element(Gui *gui, GuiElement *element);

void gf3d_gui_draw(Gui *gui, Uint32 bufferFrame, VkCommandBuffer commandBuffer);

VkVertexInputBindingDescription *gf3d_gui_get_bind_description();
VkVertexInputAttributeDescription *gf3d_gui_get_attribute_descriptions(Uint32 *count);

#endif