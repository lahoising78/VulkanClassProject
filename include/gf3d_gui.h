#ifndef _GF3D_GUI_H_
#define _GF3D_GUI_H_

#include <SDL2/SDL.h>
#include "gf3d_vgraphics.h"
#include "gf3d_gui_element.h"

typedef struct
{
    SDL_Surface *surface;
    SDL_Renderer *renderer;
    GuiElement **elements;
    Uint32 elementCount;
    Uint8 _inuse;
} Gui;

void gf3d_gui_manager_init(Uint32 count);

void gf3d_gui_manager_draw();

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

void gf3d_gui_draw(Gui *gui);

#endif