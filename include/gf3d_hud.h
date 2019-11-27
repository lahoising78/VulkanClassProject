#ifndef _GF3D_HUD_H_
#define _GF3D_HUD_H_

#include "gf3d_gui_element.h"
#include "gf3d_game_defines.h"

#define LABEL_MAX_CHARACTERS 256

typedef enum
{
    GF3D_HUD_TYPE_NONE =            0,
    GF3D_HUD_TYPE_GUI_ELEMENT =     1,
    GF3D_HUD_TYPE_PROGRESS_BAR =    2,
    GF3D_HUD_TYPE_BUTTON =          3,
    GF3D_HUD_TYPE_LABEL =           4,
    GF3D_HUD_TYPE_TEXT_INPUT =      5,
    GF3D_HUD_TYPE_WINDOW =          6,
    GF3D_HUD_TYPE_NUM
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

/* ========HUD LABELS======= */

typedef struct
{
    GuiElement *display;
    GuiElement *textDisp;
    Vector4D textColor;
    char *text;
} Label;

Label *gf3d_hud_label_create(Vector2D pos, Vector2D ext, Vector4D color, Vector4D textColor, const char *text);
void gf3d_hud_label_set_text(Label *label, const char *text);

/* =========BUTTON======== */

typedef struct hud_button_t
{
    Label *bg;
    void (*on_click) (struct hud_button_t *button);
} Button;

Button *gf3d_hud_button_create(Vector2D pos, Vector2D ext, Vector4D color, Vector4D textColor, const char *text);
extern OnClickCallback on_clicks[32];

/* ===========HUD TEXT INPUT=========== */

typedef struct
{
    Label *textDisplay;
    uint8_t selected;
} TextInput;

TextInput *gf3d_hud_text_input_create(Vector2D pos, Vector2D ext, Vector4D bgColor, Vector4D textColor, const char *text);

/* =========HUD ELEMENT======= */
struct hud_window_t;
typedef struct hud_element_t
{
    union
    {
        GuiElement  *guiElement;
        ProgressBar *pBar;
        Button *button;
        Label *label;
        TextInput *textInput;
        struct hud_window_t *window;
        
    } element;

    HudType type;
    char name[GFCLINELEN];
} HudElement;

HudElement gf3d_hud_element_load(SJson *json);

void gf3d_hud_element_draw(HudElement *e, uint32_t bufferFrame, VkCommandBuffer commandBuffer);
void gf3d_hud_element_free(HudElement *e);
void gf3d_hud_element_update(HudElement *e, SDL_Event *keys, SDL_Event *mouse);

void gf3d_hud_element_set_position(HudElement e, Vector2D pos);
Vector2D gf3d_hud_element_get_position(HudElement e);
void gf3d_hud_element_set_extents(HudElement e, Vector2D ext);
Vector2D gf3d_hud_element_get_extents(HudElement e);

/* ===========HUD WINDOW========= */

typedef struct hud_window_t
{
    GuiElement *bg;
    HudElement *elements;
    Vector2D *elementPositions;
    uint32_t count;
} Window;

Window *gf3d_hud_window_create(uint32_t count, Vector2D pos, Vector2D ext, Vector4D color);
void gf3d_hud_window_add_element(Window *window, HudElement e);

#endif