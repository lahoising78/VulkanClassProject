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

Window *gf3d_hud_window_load(SJson *json);
void gf3d_hud_window_free(Window *window);
void gf3d_hud_window_update(Window *window, SDL_Event *keys, SDL_Event *mouse);
void gf3d_hud_window_draw(Window *window, uint32_t bufferFrame, VkCommandBuffer commandBuffer);

Vector2D gf3d_hud_element_get_position(HudElement e);
void gf3d_hud_element_set_position(HudElement e, Vector2D pos);

HudElement gf3d_hud_element_load_hud_file(SJson *json);

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
Button *gf3d_hud_button_create(Vector2D pos, Vector2D ext, Vector4D color, Vector4D textColor, const char *text)
{
    Button *button;

    button = (Button*)malloc(sizeof(Button));
    button->bg = gf3d_hud_label_create(pos, ext, color, textColor, text);
    button->on_click = NULL;

    return button;
}

Button *gf3d_hud_button_load(SJson *json)
{
    Button *button = NULL;
    int func;

    SJson *obj = NULL;

    slog("button load");
    button = (Button*)malloc(sizeof(Button));
    if(!button) return NULL;

    obj = sj_object_get_value(json, "label");
    button->bg = gf3d_hud_label_load(obj);

    /* onClick field in json will be the index in on_click OnClickCallback array */
    obj = sj_object_get_value(json, "onClick");
    sj_get_integer_value(obj, &func);

    button->on_click = on_clicks[func];

    return button;
}

void gf3d_hud_button_free(Button *button)
{
    if(!button) return;

    gf3d_hud_label_free(button->bg);
    button->bg = NULL;
}

void gf3d_hud_button_update(Button *button, SDL_Event *events)
{
    SDL_Event e;
    SDL_Rect a;
    SDL_Point p;

    if(!button) return;
    
    gf3d_hud_label_update(button->bg);

    if(!button->on_click) return;
    
    e = events[SDL_BUTTON_LEFT];
    if( e.button.type == SDL_MOUSEBUTTONUP )
    {
        vector2d_copy(a, button->bg->display->position);
        a.w = button->bg->display->extents.x;
        a.h = button->bg->display->extents.y;

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
    
    gf3d_hud_label_draw(button->bg, bufferFrame, commandBuffer);
}

Label *gf3d_hud_label_create(Vector2D pos, Vector2D ext, Vector4D color, Vector4D textColor, const char *text)
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

    label = gf3d_hud_label_create(pos, ext, color, textColor, sj_get_string_value(arr));

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

void gf3d_hud_label_set_text(Label *label, const char *text)
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
TextInput *gf3d_hud_text_input_create(Vector2D pos, Vector2D ext, Vector4D bgColor, Vector4D textColor, const char *text)
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
        return NULL;
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

    obj = sj_object_get_value(json, "file");
    if(obj)
    {
        return gf3d_hud_element_load_hud_file(obj);
    }

    obj = sj_object_get_value(json, "type");
    sj_get_integer_value(obj, &type);
    obj = sj_object_get_value(json, "name");
    if(obj)
    {
        gfc_line_cpy(e.name, sj_get_string_value(obj));
    }
    else
    {
        gfc_line_cpy(e.name, "Element");
    }

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

    case GF3D_HUD_TYPE_WINDOW:
        e.type = GF3D_HUD_TYPE_WINDOW;
        e.element.window = gf3d_hud_window_load(json);
        break;

    default:
        e.type = GF3D_HUD_TYPE_NONE;
        e.element.guiElement = NULL;
        sprintf(e.name, " ");
        break;
    }

    return e;
}

HudElement gf3d_hud_element_load_hud_file(SJson *json)
{
    int i;
    TextLine assetname;
    SJson *obj = NULL;
    SJson *arr = NULL;

    HudElement window = {0};
    HudElement e = {0};

    char filename[GFCLINELEN];

    obj = sj_object_get_value(json, "window");
    window = gf3d_hud_element_load(obj);
    if(window.type != GF3D_HUD_TYPE_WINDOW) 
    {
        gf3d_hud_element_free(&window);
        memset(&window, 0, sizeof(HudElement));
        return window;
    }

    obj = sj_object_get_value(json, "filename");
    gfc_line_cpy(filename, sj_get_string_value(obj));

    snprintf(assetname, GFCLINELEN, "guis/%s.json", filename);
    json = sj_load(assetname);
    if( !json )
    {
        slog("file was not found");
        gf3d_hud_element_free(&window);
        memset(&window, 0, sizeof(HudElement));
        return window;
    }
    slog("opened hud file %s", assetname);

    arr = sj_object_get_value(json, "elements");
    for(i = 0; i < window.element.window->count; i++)
    {
        obj = sj_array_get_nth(arr, i);
        if(!obj) break;

        e = gf3d_hud_element_load(obj);
        if(e.type == GF3D_HUD_TYPE_NONE || e.type == GF3D_HUD_TYPE_NUM) 
        {
            gf3d_hud_element_free(&e);
            continue;
        }
        slog("-== type: %d ==-", e.type);
        gf3d_hud_window_add_element(window.element.window, e);
    }

    sj_free(json);
    return window;
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

        case GF3D_HUD_TYPE_WINDOW:
            gf3d_hud_window_draw(e->element.window, bufferFrame, commandBuffer);
            break;

        default:
            slog("unrecognized hud type");
            break;
    }
}

void gf3d_hud_element_free(HudElement *e)
{
    if(!e) return;
    slog("free %s", e->name);
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

    case GF3D_HUD_TYPE_WINDOW:
        gf3d_hud_window_free(e->element.window);
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

    case GF3D_HUD_TYPE_WINDOW:
        gf3d_hud_window_update(e->element.window, keys, mouse);
        break;
    
    default:
        slog("unrecognized hud type");
        break;
    }
}

Vector2D gf3d_hud_element_get_position(HudElement e)
{
    switch (e.type)
    {
        case GF3D_HUD_TYPE_GUI_ELEMENT:     return e.element.guiElement->position; break;
        case GF3D_HUD_TYPE_PROGRESS_BAR:    return e.element.pBar->back->position; break;
        case GF3D_HUD_TYPE_BUTTON:          return e.element.button->bg->display->position; break;
        case GF3D_HUD_TYPE_LABEL:           return e.element.label->display->position; break;
        case GF3D_HUD_TYPE_TEXT_INPUT:      return e.element.textInput->textDisplay->display->position; break;
        case GF3D_HUD_TYPE_WINDOW:          return e.element.window->bg->position; break;
        default: return vector2d(0.0, 0.0);
    }
}

void gf3d_hud_element_set_position(HudElement e, Vector2D pos)
{
    switch (e.type)
    {
        case GF3D_HUD_TYPE_GUI_ELEMENT:     
            vector2d_copy(e.element.guiElement->position, pos); 
            break;
        case GF3D_HUD_TYPE_PROGRESS_BAR:    
            vector2d_copy(e.element.pBar->back->position, pos); 
            vector2d_add(e.element.pBar->fore->position, e.element.pBar->back->position, e.element.pBar->bgWidth);
            break;
        case GF3D_HUD_TYPE_BUTTON:          
            vector2d_copy(e.element.button->bg->display->position, pos); 
            vector2d_copy(e.element.button->bg->textDisp->position, pos);
            break;
        case GF3D_HUD_TYPE_LABEL:           
            vector2d_copy(e.element.label->display->position, pos); 
            vector2d_copy(e.element.label->textDisp->position, pos);
            break;
        case GF3D_HUD_TYPE_TEXT_INPUT:      
            vector2d_copy(e.element.textInput->textDisplay->display->position, pos); 
            vector2d_copy(e.element.textInput->textDisplay->textDisp->position, pos);
            break;
        case GF3D_HUD_TYPE_WINDOW:          
            vector2d_copy(e.element.window->bg->position, pos); 
            break;
        default:                            break;
    }
}

void gf3d_hud_element_set_extents(HudElement e, Vector2D ext)
{
    switch (e.type)
    {
    case GF3D_HUD_TYPE_GUI_ELEMENT:
        vector2d_copy(e.element.guiElement->extents, ext);
        break;
    
    default:
        break;
    }
}

Vector2D gf3d_hud_element_get_extents(HudElement e)
{
    switch(e.type)
    {
    case GF3D_HUD_TYPE_GUI_ELEMENT:
        return e.element.guiElement->extents;

    default:
        return vector2d(0.0f, 0.0f);
    }
}

/* ==============HUD WINDOW=========== */
Window *gf3d_hud_window_create(uint32_t count, Vector2D pos, Vector2D ext, Vector4D color)
{
    Window *window = NULL;

    window = (Window*)malloc(sizeof(Window));
    if(!window)
    {
        slog("unable to create window");
        return NULL;
    }
    memset(window, 0, sizeof(Window));

    window->elements = (HudElement*)gfc_allocate_array(sizeof(HudElement), count);
    if(!window->elements)
    {
        slog("could not allocate enough elements");
        free(window);
        return NULL;
    }

    window->elementPositions = (Vector2D*)gfc_allocate_array(sizeof(Vector2D), count);
    if(!window->elementPositions)
    {
        slog("was unable to allocate enough positions");
        free(window);
        free(window->elements);
    }

    window->count = count;

    window->bg = gf3d_gui_element_create(pos, ext, color);

    return window;
}

HudElement *gf3d_hud_window_get_element_by_name(Window *window, const char *name)
{
    int i;

    if(!window) return NULL;
    if(!window->elements) return NULL;

    for(i = 0; i < window->count; i++)
    {
        // if(strncmp(window->elements[i].name, name, strlen(name)) == 0)
        //     return &window->elements[i];
    }

    return NULL;
}

Window *gf3d_hud_window_load(SJson *json)
{
    int i;
    Window *window = NULL;
    SJson *obj = NULL;
    SJson *arr = NULL;

    if(!json) return NULL;

    window = (Window*)malloc(sizeof(Window));
    if(!window) 
    {
        slog("unable to allocate a new window");
        return NULL;
    }

    obj = sj_object_get_value(json, "bg");
    window->bg = gf3d_gui_element_load(obj);

    obj = sj_object_get_value(json, "count");
    if(obj)
    {
        sj_get_integer_value(obj,(int*)(&window->count));
    }
    else
    {
        window->count = 32;
    }
    window->elementPositions = (Vector2D*)gfc_allocate_array(sizeof(Vector2D), window->count);
    memset(window->elementPositions, 0, sizeof(Vector2D) * window->count);
    window->elements = (HudElement*)gfc_allocate_array(sizeof(HudElement), window->count);
    memset(window->elements, 0, sizeof(HudElement) * window->count);

    arr = sj_object_get_value(json, "elements");
    for(i = 0; i < window->count; i++)
    {
        obj = sj_array_get_nth(arr, i);
        if(!obj)
        {
            break;
        }

        gf3d_hud_window_add_element(window, gf3d_hud_element_load(obj));
    }

    return window;
}

void gf3d_hud_window_free(Window *window)
{
    int i;
    HudElement *e = NULL;
    if(!window);

    gf3d_gui_element_free(window->bg);

    for(i = 0; i < window->count; i++)
    {
        e = (&window->elements[i]);
        gf3d_hud_element_free(e);
        memset(e, 0, sizeof(HudElement));
    }
    if(window->elements) free(window->elements);

    if(window->elementPositions) free(window->elementPositions);

    memset(window, 0, sizeof(Window));
}

void gf3d_hud_window_update(Window *window, SDL_Event *keys, SDL_Event *mouse)
{
    int i;
    Vector2D pos;
    if(!window) return;

    gf3d_gui_element_update(window->bg);

    for(i = 0; i < window->count; i++)
    {
        if(window->elements[i].type != GF3D_HUD_TYPE_NONE)
        {
            vector2d_copy(pos, gf3d_hud_element_get_position(window->elements[i]));
            vector2d_copy(window->elementPositions[i], pos );
            vector2d_add(pos, pos, window->bg->position);
            gf3d_hud_element_set_position(window->elements[i], pos);
            gf3d_hud_element_update(&window->elements[i], keys, mouse);
            gf3d_hud_element_set_position( window->elements[i], window->elementPositions[i] );
        }
    }
}

void gf3d_hud_window_draw(Window *window, uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    int i;
    if(!window) return;

    gf3d_gui_element_draw(window->bg, bufferFrame, commandBuffer);

    for(i = 0; i < window->count; i++)
    {
        if(window->elements[i].type != GF3D_HUD_TYPE_NONE)
        {
            gf3d_hud_element_draw(&window->elements[i], bufferFrame, commandBuffer);
        }
    }
}

void gf3d_hud_window_add_element(Window *window, HudElement e)
{
    int i;
    if(!e.type || !window) return;

    for(i = 0; i < window->count; i++)
    {
        if(window->elements[i].type) continue;
        window->elements[i] = e;
        window->countActual++;
        break;
    }
}

void gf3d_hud_window_remove_element(Window *window, HudElement e)
{
    int i;
    if(!e.type || !window) return;

    for(i = 0; i < window->count; i++)
    {
        if(!window->elements[i].type) continue;
        if( gfc_line_cmp(window->elements[i].name, e.name) == 0 )
        {
            gf3d_hud_element_free(&window->elements[i]);
            memset(&window->elements[i], 0, sizeof(HudElement));
            window->countActual--;
            return;
        }
    }
}

void gf3d_hud_window_remove_element_at_index(Window *window, uint32_t i)
{
    if(!window || i >= window->count) return;

    gf3d_hud_element_free( &window->elements[i] );
    memset(&window->elements[i], 0, sizeof(HudElement));
    window->countActual--;
}