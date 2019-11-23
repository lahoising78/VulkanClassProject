#ifndef _GF3D_HUD_H_
#define _GF3D_HUD_H_

#include "gf3d_gui_element.h"

typedef struct hud_pb_t
{
    GuiElement *back;
    GuiElement *fore;
    void (*draw) (struct hud_pb_t *bar, VkCommandBuffer commandBuffer);
    float *max;
    float *val;
} ProgressBar;

ProgressBar *gf3d_hud_pb_create(float *max, float *val);

void gf3d_hud_pb_set_background(ProgressBar *bar, Vector2D pos, Vector2D ext, Vector4D color);
void gf3d_hud_pb_set_foreground(ProgressBar *bar, Vector2D pos, Vector2D ext, Vector4D color);

#endif