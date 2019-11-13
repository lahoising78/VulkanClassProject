#include "gf3d_gui_element.h"

#include "simple_logger.h"

GuiElement gf3d_gui_element_create(Vector2D pos, Vector2D ext, Color color)
{
    GuiElement element;
    element.position = pos;
    element.extents = ext;
    element.color = color;
    element.tex = NULL;
    return element;
}

void gf3d_gui_element_draw(GuiElement element, SDL_Renderer *renderer)
{
    SDL_Rect rect;

    if(!renderer) return;

    rect.x = element.position.x;
    rect.y = element.position.y;
    rect.w = element.extents.x;
    rect.h = element.extents.y;

    SDL_SetRenderDrawColor(renderer, element.color.r * 255, element.color.g * 255, element.color.b * 255, element.color.a * 255);
    SDL_RenderFillRect(renderer, &rect);
}

/* ==========COLOR======== */
Color gfc_color(float r,float g,float b,float a)
{
    Color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}