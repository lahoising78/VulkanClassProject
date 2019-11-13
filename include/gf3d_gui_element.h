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
    Vector2D position;
    Vector2D extents;
    Color color;
    Texture *tex;
} GuiElement;

GuiElement gf3d_gui_element_create(Vector2D pos, Vector2D ext, Color color);

void gf3d_gui_element_draw(GuiElement element, SDL_Renderer *renderer);

#endif