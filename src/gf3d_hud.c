#include "gf3d_hud.h"

#include "simple_logger.h"
#include "gf3d_game_defines.h"

void gf3d_hud_progress_bar_free(ProgressBar *bar);
void gf3d_hud_progress_bar_update(ProgressBar *bar);
void gf3d_hud_progress_bar_draw(ProgressBar *bar, VkCommandBuffer commandBuffer);

void gf3d_hud_button_free(Button *button);
void gf3d_hud_button_update(Button *button, SDL_Event *events);
void gf3d_hud_button_draw(Button *button, VkCommandBuffer commandBuffer);

/* ====PROGRESS BAR SPECIFIC==== */
ProgressBar *gf3d_hud_progress_bar_create(float *max, float *val, Vector2D bgWidth)
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
    vector2d_copy(bar->bgWidth, bgWidth);
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

void gf3d_hud_progress_bar_update(ProgressBar *bar)
{
    if(!bar) return;
    gf3d_gui_element_update(bar->back);
    
    bar->fore->extents.x = bar->foreMaxWidth * (*bar->val / *bar->max);

    gf3d_gui_element_update(bar->fore);
}

void gf3d_hud_progress_bar_draw(ProgressBar *bar, VkCommandBuffer commandBuffer)
{
    if(!bar) return;
    gf3d_gui_element_draw(bar->back, commandBuffer);
    gf3d_gui_element_draw(bar->fore, commandBuffer);
}

void gf3d_hud_progress_bar_set_background(ProgressBar *bar, Vector2D pos, Vector2D ext, Vector4D color)
{
    if(!bar) return;
    bar->back = gf3d_gui_element_create(pos, ext, color);
}

void gf3d_hud_progress_bar_set_foreground(ProgressBar *bar, Vector4D color)
{
    Vector2D pos;
    Vector2D ext;

    if(!bar) return;
    
    vector2d_add(pos, bar->back->position, bar->bgWidth);
    vector2d_mul(ext, bar->bgWidth, 2);
    vector2d_sub(ext, bar->back->extents, ext);
    bar->foreMaxWidth = ext.x;

    bar->fore = gf3d_gui_element_create(pos, ext, color);
}

/* ==========HUD BUTTON========= */
Button *gf3d_hud_button_create(Vector2D pos, Vector2D ext, Vector4D color)
{
    Button *button;

    button = (Button*)malloc(sizeof(Button));
    button->bg = gf3d_gui_element_create(pos, ext, color);

    return button;
}

void gf3d_hud_button_free(Button *button)
{
    if(!button) return;

    gf3d_gui_element_free(button->bg);
    button->bg = NULL;
}

void gf3d_hud_button_update(Button *button, SDL_Event *events)
{
    SDL_Event e;
    SDL_Rect a;
    SDL_Point p;
    int x, y;
    x = y = 0;

    if(!button) return;
    
    gf3d_gui_element_update(button->bg);

    if(!button->on_click) return;
    
    e = events[SDL_BUTTON_LEFT];
    if( e.button.type == SDL_MOUSEBUTTONUP )
    {
        vector2d_copy(a, button->bg->position);
        a.w = button->bg->extents.x;
        a.h = button->bg->extents.y;

        p.x = e.button.x; 
        p.y = e.button.y;

        if( SDL_PointInRect(&p, &a) == SDL_TRUE )
        {
            button->on_click(button);
        }
        else
        {
            slog("click: %d %d", p.x, p.y);
        }
    }
}

void gf3d_hud_button_draw(Button *button, VkCommandBuffer commandBuffer)
{
    if(!button) return;
    
    gf3d_gui_element_draw(button->bg, commandBuffer);
}

/* ==========HUD GENERAL========== */
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

        case GF3D_HUD_TYPE_BUTTON:
            gf3d_hud_button_draw(e->element.button, commandBuffer);
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

    case GF3D_HUD_TYPE_BUTTON:
        gf3d_hud_button_free(e->element.button);
        break;

    default:
        slog("unrecognized hud type");
        break;
    }

    e->type = GF3D_HUD_TYPE_NONE;
}

void gf3d_hud_element_update(HudElement *e, SDL_Event *events)
{
    if(!e) return;

    switch (e->type)
    {
    case GF3D_HUD_TYPE_GUI_ELEMENT:
        gf3d_gui_element_update(e->element.guiElement);
        break;

    case GF3D_HUD_TYPE_PROGRESS_BAR:
        gf3d_hud_progress_bar_update(e->element.pBar);
        break;

    case GF3D_HUD_TYPE_BUTTON:
        gf3d_hud_button_update(e->element.button, events);
        break;
    
    default:
        slog("unrecognized hud type");
        break;
    }
}