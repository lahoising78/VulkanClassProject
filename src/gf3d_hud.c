#include "gf3d_hud.h"

#include "simple_logger.h"

void gf3d_hud_progress_bar_free(ProgressBar *bar);
void gf3d_hud_progress_bar_draw(ProgressBar *bar, VkCommandBuffer commandBuffer);

/* ====PROGRESS BAR SPECIFIC==== */
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

    return bar;
}

void gf3d_hud_progress_bar_free(ProgressBar *bar)
{
    if(!bar) return;
    gf3d_gui_element_free(bar->back);
    gf3d_gui_element_free(bar->fore);
    bar->max = NULL;
    bar->val = NULL;
}

void gf3d_hud_progress_bar_set_background(ProgressBar *bar, Vector2D pos, Vector2D ext, Vector4D color)
{
    if(!bar) return;
    bar->back = gf3d_gui_element_create(pos, ext, color);
}

void gf3d_hud_progress_bar_set_foreground(ProgressBar *bar, Vector2D pos, Vector2D ext, Vector4D color)
{
    if(!bar) return;
    bar->fore = gf3d_gui_element_create(pos, ext, color);
}

void gf3d_hud_progress_bar_draw(ProgressBar *bar, VkCommandBuffer commandBuffer)
{
    if(!bar) return;
    gf3d_gui_element_draw(bar->back, commandBuffer);
    gf3d_gui_element_draw(bar->fore, commandBuffer);
}

/* ========HUD GENERAL======== */
void gf3d_hud_element_draw(HudElement *e, VkCommandBuffer commandBuffer)
{
    if(!e) return;
    switch(e->type)
    {
        case GF3D_HUD_TYPE_GUI_ELEMENT:
            gf3d_gui_element_draw(e->element.guiElement, commandBuffer);
            break;

        case GF3D_HUD_TYPE_PROGRESS_BAR:
            gf3d_hud_progress_bar_draw(e->element.pBar, commandBuffer);
            break;

        default:
            slog("unrecognized hud type");
            break;
    }
}

void gf3d_hud_element_free(HudElement *e)
{
    if(!e) return;
    switch (e->type)
    {
    case GF3D_HUD_TYPE_GUI_ELEMENT:
        gf3d_gui_element_free(e->element.guiElement);
        break;
    
    case GF3D_HUD_TYPE_PROGRESS_BAR:
        gf3d_hud_progress_bar_free(e->element.pBar);
        break;

    default:
        slog("unrecognized hud type");
        break;
    }

    e->type = GF3D_HUD_TYPE_NONE;
}