#include "gf3d_ui.h"

void gf3d_ui_free(UI *ui)
{
    if(!ui) return;
    memset(ui, 0, sizeof(UI));
}

void gf3d_ui_render(UI *ui, SDL_Renderer *renderer)
{
    if(!ui || !renderer) return;

    SDL_SetRenderDrawColor(renderer, (uint8_t)ui->color.x, (uint8_t)ui->color.y, (uint8_t)ui->color.z, (uint8_t)ui->color.w );
    SDL_RenderFillRect(renderer, &ui->rect);
}