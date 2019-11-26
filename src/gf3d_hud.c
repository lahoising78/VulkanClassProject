#include "gf3d_hud.h"

#include "simple_logger.h"
#include "gf3d_vec.h"

ProgressBar *gf3d_hud_progress_bar_load(SJson *json);
void gf3d_hud_progress_bar_free(ProgressBar *bar);
void gf3d_hud_progress_bar_update(ProgressBar *bar);
void gf3d_hud_progress_bar_draw(ProgressBar *bar, uint32_t bufferFrame, VkCommandBuffer commandBuffer);

Button *gf3d_hud_button_load(SJson *json);
void gf3d_hud_button_free(Button *button);
void gf3d_hud_button_update(Button *button, SDL_Event *events);
void gf3d_hud_button_draw(Button *button, uint32_t bufferFrame, VkCommandBuffer commandBuffer);

Label *gf3d_hud_label_load(SJson *json);
void gf3d_hud_label_free(Label *label);
void gf3d_hud_label_update(Label *label);
void gf3d_hud_label_draw(Label *label, uint32_t bufferFrame, VkCommandBuffer commandBuffer);

TextInput *gf3d_hud_text_input_load(SJson *json);
void gf3d_hud_text_input_free(TextInput *textInput);
void gf3d_hud_text_input_update(TextInput *textInput, SDL_Event *keys, SDL_Event *mouse);
void gf3d_hud_text_input_draw(TextInput *textInput, uint32_t bufferFrame, VkCommandBuffer commandBuffer);

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

ProgressBar *gf3d_hud_progress_bar_load(SJson *json)
{
    ProgressBar *bar = NULL;
    Vector2D bgWidth;
    Vector2D backPos, backExt;
    Vector4D backColor, foreColor;

    SJson *arr = NULL;
    SJson *obj = NULL;

    slog("progress bar load");

    arr = sj_object_get_value(json, "bgWidth");
    bgWidth = gf3d_vec2_load(arr);

    bar = gf3d_hud_progress_bar_create(NULL, NULL, bgWidth);

    obj = sj_object_get_value(json, "back");
    arr = sj_object_get_value(obj, "position");
    backPos = gf3d_vec2_load(arr);
    arr = sj_object_get_value(obj, "extents");
    backExt = gf3d_vec2_load(arr);
    arr = sj_object_get_value(obj, "color");
    backColor = gf3d_vec4_load(arr);
    gf3d_hud_progress_bar_set_background(bar, backPos, backExt, backColor);

    arr = sj_object_get_value(json, "foreColor");
    foreColor = gf3d_vec4_load(arr);
    gf3d_hud_progress_bar_set_foreground(bar, foreColor);

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
    
    if(bar->val && bar->max)
        bar->fore->extents.x = bar->foreMaxWidth * (*bar->val / *bar->max);

    gf3d_gui_element_update(bar->fore);
}

void gf3d_hud_progress_bar_draw(ProgressBar *bar, uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    if(!bar) return;
    gf3d_gui_element_draw(bar->back, bufferFrame, commandBuffer);
    gf3d_gui_element_draw(bar->fore, bufferFrame, commandBuffer);
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
    button->on_click = NULL;

    return button;
}

Button *gf3d_hud_button_load(SJson *json)
{
    Button *button = NULL;
    Vector2D pos, ext;
    Vector4D color;
    int func;

    SJson *obj = NULL;
    SJson *arr = NULL;

    slog("button load");

    arr = sj_object_get_value(json, "position");
    pos = gf3d_vec2_load(arr);
    arr = sj_object_get_value(json, "extents");
    ext = gf3d_vec2_load(arr);
    arr = sj_object_get_value(json, "color");
    color = gf3d_vec4_load(arr);

    button = gf3d_hud_button_create(pos, ext, color);

    /* onClick field in json will be the index in on_click OnClickCallback array */
    obj = sj_object_get_value(json, "onClick");
    sj_get_integer_value(obj, &func);

    button->on_click = on_clicks[func];

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
    }
}

void gf3d_hud_button_draw(Button *button, uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    if(!button) return;
    
    gf3d_gui_element_draw(button->bg, bufferFrame, commandBuffer);
}

Label *gf3d_hud_label_create(Vector2D pos, Vector2D ext, Vector4D color, Vector4D textColor, char *text)
{
    Label *label = NULL;

    label = (Label*)malloc(sizeof(Label));
    if(!label)
    {
        slog("not enough space for label");
        return NULL;
    }

    vector4d_copy(label->textColor, textColor);
    label->display = gf3d_gui_element_create(pos, ext, color);

    ext.x = strlen(text) * 16.0f;
    ext.y = 32.0f;
    
    label->textDisp = gf3d_gui_element_create(pos, ext, textColor);
    
    label->textDisp->vertices[0].texel = vector2d(0.0f, 0.0f);
    label->textDisp->vertices[1].texel = vector2d(1.0f, 0.0f);
    label->textDisp->vertices[2].texel = vector2d(1.0f, 1.0f);
    label->textDisp->vertices[3].texel = vector2d(0.0f, 1.0f);

    label->text = (char*)gfc_allocate_array(sizeof(char), LABEL_MAX_CHARACTERS);

    gf3d_hud_label_set_text(label, text);

    return label;
}

Label *gf3d_hud_label_load(SJson *json)
{
    Label *label = NULL;
    Vector2D pos, ext;
    Vector4D color, textColor;
    char *text;

    SJson *obj = NULL;
    SJson *arr = NULL;

    slog("label load");

    arr = sj_object_get_value(json, "position");
    obj = sj_array_get_nth(arr, 0);
    sj_get_float_value(obj, &pos.x);
    obj = sj_array_get_nth(arr, 1);
    sj_get_float_value(obj, &pos.y);

    arr = sj_object_get_value(json, "extents");
    obj = sj_array_get_nth(arr, 0);
    sj_get_float_value(obj, &ext.x);
    obj = sj_array_get_nth(arr, 1);
    sj_get_float_value(obj, &ext.y);

    arr = sj_object_get_value(json, "color");
    obj = sj_array_get_nth(arr, 0);
    sj_get_float_value(obj, &color.x);
    obj = sj_array_get_nth(arr, 1);
    sj_get_float_value(obj, &color.y);
    obj = sj_array_get_nth(arr, 2);
    sj_get_float_value(obj, &color.z);
    obj = sj_array_get_nth(arr, 3);
    sj_get_float_value(obj, &color.w);

    arr = sj_object_get_value(json, "textColor");
    obj = sj_array_get_nth(arr, 0);
    sj_get_float_value(obj, &textColor.x);
    obj = sj_array_get_nth(arr, 1);
    sj_get_float_value(obj, &textColor.y);
    obj = sj_array_get_nth(arr, 2);
    sj_get_float_value(obj, &textColor.z);
    obj = sj_array_get_nth(arr, 3);
    sj_get_float_value(obj, &textColor.w);

    arr = sj_object_get_value(json, "text");
    text = sj_get_string_value(arr);

    label = gf3d_hud_label_create(pos, ext, color, textColor, text);

    vector2d_slog(pos);
    vector2d_slog(ext);
    vector4d_slog(color);
    vector4d_slog(textColor);
    slog("%s", text);

    return label;
}

void gf3d_hud_label_free(Label *label)
{
    if(!label) return;

    gf3d_gui_element_free(label->display);
    gf3d_gui_element_free(label->textDisp);
    if(label->text) free(label->text);
}

void gf3d_hud_label_update(Label *label)
{
    if(!label) return;

    gf3d_gui_element_update(label->display);
    gf3d_gui_element_update(label->textDisp);
}

void gf3d_hud_label_draw(Label *label, uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    if(!label) return;

    gf3d_gui_element_draw(label->display, bufferFrame, commandBuffer);
    gf3d_gui_element_draw(label->textDisp, bufferFrame, commandBuffer);
}

void gf3d_hud_label_set_text(Label *label, char *text)
{
    SDL_Surface *surface = NULL;
    // SDL_Renderer *renderer = NULL;
    TTF_Font *font = NULL;
    SDL_Color color;

    if(!label || !label->text) return;

    if(label->text) gfc_line_cpy(label->text, text);
    slog("new tex: %s", label->text);

    if(strlen(text) <= 0)
    {
        surface = SDL_CreateRGBSurface(0, 1, 1, 32, rmask, gmask, bmask, amask);
        gf3d_gui_element_attach_texture_from_surface(label->textDisp, surface);
        SDL_FreeSurface(surface);
        return;
    }

    font = gf3d_gui_element_get_font();
    if(!font) 
    {
        slog("no font found");
        return;
    }

    color.r = (uint8_t)label->textColor.x;
    color.g = (uint8_t)label->textColor.y;
    color.b = (uint8_t)label->textColor.z;
    color.a = (uint8_t)label->textColor.w;

    surface = TTF_RenderText_Solid(font, text, color);
    if( surface->format->format != SDL_PIXELFORMAT_ARGB8888 )
    {
        surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
    }

    gf3d_gui_element_attach_texture_from_surface(label->textDisp, surface);
    gfc_line_cpy(label->display->tex->filename, text);

    SDL_FreeSurface(surface);
}

/* =========HUD TEXT INPUT======== */
TextInput *gf3d_hud_text_input_create(Vector2D pos, Vector2D ext, Vector4D bgColor, Vector4D textColor, char *text)
{
    TextInput *textInput = NULL;

    textInput = (TextInput*)malloc(sizeof(textInput));
    if(!textInput)
    {
        slog("unable to allocate a new text input");
        return NULL;
    }

    textInput->textDisplay = gf3d_hud_label_create(pos, ext, bgColor, textColor, text);
    textInput->selected = 0;

    return textInput;
}

TextInput *gf3d_hud_text_input_load(SJson *json)
{
    TextInput *textInput = NULL;

    SJson *obj = NULL;

    slog("text input load");
    
    textInput = (TextInput*)malloc(sizeof(TextInput));
    if(!textInput)
    {
        slog("unable to allocate a new text input");
        return;
    }

    obj = sj_object_get_value(json, "label");
    textInput->textDisplay = gf3d_hud_label_load(obj);
    textInput->selected = 0;

    return textInput;
}

void gf3d_hud_text_input_free(TextInput *textInput)
{
    if(!textInput) return;

    gf3d_hud_label_free(textInput->textDisplay);
    textInput->selected = 0;
}

void gf3d_hud_text_input_update(TextInput *textInput, SDL_Event *keys, SDL_Event *mouse)
{
    SDL_Event e;
    SDL_Point p;
    SDL_Rect r;
    int i;

    Vector2D pos, ext;

    if(!textInput) return;

    e = mouse[SDL_BUTTON_LEFT];
    switch (e.button.type)
    {
    case SDL_MOUSEBUTTONUP:
        vector2d_copy(p, e.button);

        pos = textInput->textDisplay->display->position;
        vector2d_copy(r, pos);

        ext = textInput->textDisplay->display->extents;
        r.w = ext.x;
        r.h = ext.y;

        if(SDL_PointInRect(&p, &r) == SDL_TRUE)
        {
            textInput->selected = 1;
        }
        else
        {
            textInput->selected = 0;
        }
        break;
    }

    if(textInput->selected)
    {
        for(i = 0; i < SDL_NUM_SCANCODES; i++)
        {
            e = keys[i];
            switch(e.key.type)
            {
            case SDL_KEYDOWN:
                if( e.key.keysym.sym != SDLK_BACKSPACE || strlen(textInput->textDisplay->text) <= 0 ) break;
                textInput->textDisplay->text[ strlen(textInput->textDisplay->text) - 1 ] = 0;
                textInput->textDisplay->textDisp->extents.x = strlen(textInput->textDisplay->text) * 16.0f;
                gf3d_hud_label_set_text(textInput->textDisplay, textInput->textDisplay->text);
                break;

            case SDL_TEXTINPUT:
                if( strlen(textInput->textDisplay->text) < LABEL_MAX_CHARACTERS - 1 )
                {
                    strcat(textInput->textDisplay->text, e.text.text);
                    textInput->textDisplay->textDisp->extents.x = strlen(textInput->textDisplay->text) * 16.0f;
                    gf3d_hud_label_set_text(textInput->textDisplay, textInput->textDisplay->text);
                }
                break;
            }
        }
    }
    
    gf3d_hud_label_update(textInput->textDisplay);
}

void gf3d_hud_text_input_draw(TextInput *textInput, uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    if(!textInput) return;

    gf3d_hud_label_draw(textInput->textDisplay, bufferFrame, commandBuffer);
}

/* ==========HUD GENERAL========== */
HudElement gf3d_hud_element_load(SJson *json)
{
    SJson *obj = NULL;

    HudElement e = {0};
    int type = GF3D_HUD_TYPE_NONE;

    obj = sj_object_get_value(json, "type");
    sj_get_integer_value(obj, &type);

    switch (type)
    {
    case GF3D_HUD_TYPE_GUI_ELEMENT:
        e.type = GF3D_HUD_TYPE_GUI_ELEMENT;
        e.element.guiElement = gf3d_gui_element_load(json);
        break;
    
    case GF3D_HUD_TYPE_PROGRESS_BAR:
        e.type = GF3D_HUD_TYPE_PROGRESS_BAR;
        e.element.pBar = gf3d_hud_progress_bar_load(json);
        break;

    case GF3D_HUD_TYPE_BUTTON:
        e.type = GF3D_HUD_TYPE_BUTTON;
        e.element.button = gf3d_hud_button_load(json);
        break;

    case GF3D_HUD_TYPE_LABEL:
        e.type = GF3D_HUD_TYPE_LABEL;
        e.element.label = gf3d_hud_label_load(json);
        break;

    case GF3D_HUD_TYPE_TEXT_INPUT:
        e.type = GF3D_HUD_TYPE_TEXT_INPUT;
        e.element.textInput = gf3d_hud_text_input_load(json);
        break;

    default:
        e.type = GF3D_HUD_TYPE_NONE;
        e.element.guiElement = NULL;
        break;
    }

    return e;
}

void gf3d_hud_element_draw(HudElement *e, uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    if(!e) return;
    switch(e->type)
    {
        case GF3D_HUD_TYPE_GUI_ELEMENT:
            gf3d_gui_element_draw(e->element.guiElement, bufferFrame, commandBuffer);
            break;

        case GF3D_HUD_TYPE_PROGRESS_BAR:
            gf3d_hud_progress_bar_draw(e->element.pBar, bufferFrame, commandBuffer);
            break;

        case GF3D_HUD_TYPE_BUTTON:
            gf3d_hud_button_draw(e->element.button, bufferFrame, commandBuffer);
            break;

        case GF3D_HUD_TYPE_LABEL:
            gf3d_hud_label_draw(e->element.label, bufferFrame, commandBuffer);
            break;

        case GF3D_HUD_TYPE_TEXT_INPUT:
            gf3d_hud_text_input_draw(e->element.textInput, bufferFrame, commandBuffer);
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

    case GF3D_HUD_TYPE_LABEL:
        gf3d_hud_label_free(e->element.label);
        break;

    case GF3D_HUD_TYPE_TEXT_INPUT:
        gf3d_hud_text_input_free(e->element.textInput);
        break;

    default:
        slog("unrecognized hud type");
        break;
    }

    e->type = GF3D_HUD_TYPE_NONE;
}

void gf3d_hud_element_update(HudElement *e, SDL_Event *keys, SDL_Event *mouse)
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
        gf3d_hud_button_update(e->element.button, mouse);
        break;

    case GF3D_HUD_TYPE_LABEL:
        gf3d_hud_label_update(e->element.label);
        break;

    case GF3D_HUD_TYPE_TEXT_INPUT:
        gf3d_hud_text_input_update(e->element.textInput, keys, mouse);
        break;
    
    default:
        slog("unrecognized hud type");
        break;
    }
}