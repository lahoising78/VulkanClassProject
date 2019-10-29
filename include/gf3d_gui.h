#ifndef _GF3D_GUI_H_
#define _GF3D_GUI_H_

#include "gf3d_shape.h"
#include <vulkan/vulkan.h>

typedef struct GuiElement_S
{
    Shape shape;
    Uint8 _inuse;
} GuiElement;


int gf3d_gui_manager_init(Uint32 count);

GuiElement *gf3d_gui_new();

void gf3d_gui_free(GuiElement *gui);

#endif