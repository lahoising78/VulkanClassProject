#ifndef _GF3D_HUD_H_
#define _GF3D_HUD_H_

#include "gf3d_gui_element.h"

typedef enum
{
    GF3D_HUD_TYPE_NONE =            0,
    GF3D_HUD_TYPE_GUI_ELEMENT =     1,
    GF3D_HUD_TYPE_PROGRESS_BAR =    2,
    GF3D_HUD_TYPE_BUTTON =          3
} HudType;

/* ==========PROGRESS BAR======== */

typedef struct hud_pb_t
{
    GuiElement *back;
    GuiElement *fore;
    Vector2D bgWidth;
    float foreMaxWidth;
    float *max;
    float *val;
} ProgressBar;

ProgressBar *gf3d_hud_progress_bar_create(float *max, float *val, Vector2D bgWidth);

void gf3d_hud_progress_bar_set_background(ProgressBar *bar, Vector2D pos, Vector2D ext, Vector4D color);
void gf3d_hud_progress_bar_set_foreground(ProgressBar *bar, Vector4D color);

/* =========BUTTON======== */
typedef struct hud_button_t
{
    GuiElement *bg;
    void (*on_click) (struct hud_button_t *button);
} Button;

Button *gf3d_hud_button_create(Vector2D pos, Vector2D ext, Vector4D color);

/* ========HUD LABELS======= */

typedef struct
{
    GuiElement *display;
    char *text;
} Label;

Label *gf3d_hud_label_create(char *text);

/* =========HUD ELEMENT======= */

typedef struct hud_element_t
{
    union
    {
        GuiElement  *guiElement;
        ProgressBar *pBar;
        Button *button;
    } element;

    HudType type;
} HudElement;

void gf3d_hud_element_draw(HudElement *e, uint32_t bufferFrame, VkCommandBuffer commandBuffer);
void gf3d_hud_element_free(HudElement *e);
void gf3d_hud_element_update(HudElement *e, SDL_Event *events);

#endif