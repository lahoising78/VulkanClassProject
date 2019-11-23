#include "gf3d_hud.h"

#include "simple_logger.h"

void gf3d_hud_pb_draw(ProgressBar *bar, VkCommandBuffer commandBuffer);

ProgressBar *gf3d_hud_progress_bar_create(float *max, float *val)
{
    ProgressBar *bar = NULL;

    bar = (ProgressBar*)malloc(sizeof(ProgressBar));
    if(!bar) 
    {
        slog("failed to allocate bar");
        return NULL;
    }

    bar->max = max;
    bar->val = val;
    bar->back = NULL;
    bar->fore = NULL;
    bar->draw = gf3d_hud_pb_draw;

    return bar;
}

void gf3d_hud_pb_set_background(ProgressBar *bar, Vector2D pos, Vector2D ext, Vector4D color)
{
    bar->back = gf3d_gui_element_create(pos, ext, color);
    bar->back->draw = gf3d_gui_element_draw;
}

void gf3d_hud_pb_set_foreground(ProgressBar *bar, Vector2D pos, Vector2D ext, Vector4D color)
{
    bar->fore = gf3d_gui_element_create(pos, ext, color);
    bar->fore->draw = gf3d_gui_element_draw;
}

void gf3d_hud_pb_draw(ProgressBar *bar, VkCommandBuffer commandBuffer)
{
    slog("draw bar");
}