#include "app_editor.h"

#include "simple_logger.h"
#include "simple_json.h"

#include "gf3d_entity.h"
#include "gf3d_timer.h"
#include "gf3d_gui.h"
#include "app_editor_entity.h"

#define MAX_INPUT_KEY SDL_NUM_SCANCODES
#if SDL_BUTTON_X2 < 8
    #define MAX_INPUT_MOUSE 9
#else
    #define MAX_INPUT_MOUSE SDL_BUTTON_X2 + 1
#endif

#define GREEN_PRINT "\033[0;32m"
#define PRINT_COLOR_END "\033[0m"
#define DEFAULT_ELEMENT_COLOR vector4d(255.0f, 255.0f, 255.0f, 255.0f)
#define DEFAULT_ELEMENT_TEXT_COLOR vector4d(180.0f, 180.0f, 180.0f, 255.0f)

/* gui element indices */
#define GE_POSX         2
#define GE_POSY         4
#define GE_EXTX         7
#define GE_EXTY         9
#define GE_COLR         12
#define GE_COLG         14
#define GE_COLB         16
#define GE_COLA         18

/* progress bar indices */
#define PB_BGWX         2
#define PB_BGWY         4
#define PB_POSX         7
#define PB_POSY         9
#define PB_EXTX         12
#define PB_EXTY         14
#define PB_BGCR         17
#define PB_BGCG         19
#define PB_BGCB         21
#define PB_BGCA         23
#define PB_FGCR         26
#define PB_FGCG         28
#define PB_FGCB         30
#define PB_FGCA         32

/* button indices */
#define BTN_WIN         0
#define BTN_TCR         3
#define BTN_TCG         5
#define BTN_TCB         7
#define BTN_TCA         9
#define BTN_TXT         11

uint8_t app_editor_load(Gui **rightPane, Gui **leftPane, Gui **center, HudElement **inspectors);
Window *centerWindow = NULL;
Window *leftWindow = NULL;
Window *inspectorWindow = NULL;

int screenWidth = 1800;
int screenHeight = 700;

uint8_t lctrl = 0;
uint8_t rctrl = 0;
uint8_t inspectorSelected = 0;

HudType eType = GF3D_HUD_TYPE_GUI_ELEMENT;

void add_editor_entity(Button *btn);
void next_hud_type();
void set_add_btn_text(Gui *layer);
void update_inspector_values( HudElement nameInput );
void update_element_values( HudElement nameInput );
void removed_editor_entity(char *name);

OnClickCallback on_clicks[32] = {add_editor_entity};

void add_editor_entity(Button *btn)
{
    EditorEntity *e = NULL;
    HudElement lbl = {0};
    if(!centerWindow) return;

    e = app_editor_entity_create();
    if(!e) return;
    e->pos = vector2d(0.0f, 0.0f);
    e->ext = vector2d(50.0f, 50.0f);
    e->parent = centerWindow;
    e->ent.type = eType;
    e->ent.visible = 1;
    switch(eType)
    {
        case GF3D_HUD_TYPE_GUI_ELEMENT:
            e->ent.element.guiElement = gf3d_gui_element_create(
                vector2d(0.0f, 0.0f),
                vector2d(50.0f, 50.0f),
                DEFAULT_ELEMENT_COLOR
            );
            break;
        
        case GF3D_HUD_TYPE_PROGRESS_BAR:
            e->ent.element.pBar = gf3d_hud_progress_bar_create(
                NULL, NULL, vector2d(2.0f, 2.0f)
            );
            gf3d_hud_progress_bar_set_background(
                e->ent.element.pBar, e->pos, e->ext, DEFAULT_ELEMENT_COLOR
            );
            gf3d_hud_progress_bar_set_foreground(
                e->ent.element.pBar, DEFAULT_ELEMENT_TEXT_COLOR
            );
            break;

        case GF3D_HUD_TYPE_BUTTON:
            e->ent.element.button = gf3d_hud_button_create(
                e->pos, e->ext, DEFAULT_ELEMENT_COLOR,
                DEFAULT_ELEMENT_TEXT_COLOR, "button"
            );
            break;

        case GF3D_HUD_TYPE_LABEL:
            e->ent.element.label = gf3d_hud_label_create(
                e->pos, e->ext, DEFAULT_ELEMENT_COLOR,
                DEFAULT_ELEMENT_TEXT_COLOR, "label"
            );
            break;

        case GF3D_HUD_TYPE_TEXT_INPUT:
            e->ent.element.textInput = gf3d_hud_text_input_create(
                e->pos, e->ext, DEFAULT_ELEMENT_COLOR,
                DEFAULT_ELEMENT_TEXT_COLOR, "text box"
            );
            break;

        case GF3D_HUD_TYPE_WINDOW:
            e->ent.element.window = gf3d_hud_window_create(
                32, e->pos, e->ext, DEFAULT_ELEMENT_COLOR
            );
            break;

        default:
            app_editor_entity_free(e);
            return;
            break;
    }
    sprintf(e->ent.name, "element_%d", centerWindow->countActual);
    gf3d_hud_window_add_element(centerWindow, e->ent);

    if(!leftWindow) return;

    lbl.type = GF3D_HUD_TYPE_LABEL;
    lbl.element.label = gf3d_hud_label_create(
        vector2d(5.0f, leftWindow->countActual * 32.0f),
        vector2d(0.0f, 0.0f),
        vector4d(0.0f, 0.0f, 0.0f, 0.0f),
        vector4d(200.0f, 200.0f, 200.0f, 255.0f),
        e->ent.name
    );
    lbl.visible = 1;
    gf3d_hud_window_add_element(leftWindow, lbl);
}

int app_editor_main(int argc, char *argv[])
{
    int i;
    int fps = 0;
    uint8_t running = 1;
    uint8_t validate = 1;
    Timer frameTimer;

    uint32_t bufferFrame;
    VkCommandBuffer commandBuffer;

    SDL_Event keys[ MAX_INPUT_KEY ];
    SDL_Event mouse[ MAX_INPUT_MOUSE ];
    SDL_Event e;

    Gui *rightPane = NULL;
    Gui *leftPane = NULL;
    Gui *center = NULL;
    HudElement *inspectors[GF3D_HUD_TYPE_NUM-1];

    HudElement buf = {0};

    for(i = 1; i < argc; i++)
    {
        if( gfc_line_cmp(argv[i], "-no-validate") )
        {
            validate = 0;
        }
    }

    init_logger("gf3d.log");    
    slog("\033[0;32m================= gf3d edit begin =================\033[0m");
    gf3d_vgraphics_init(
        "program name",         //program name
        screenWidth,                   //screen width
        screenHeight,                    //screen height
        vector4d(0.51,0.75,1,1),//background color
        0,                      //fullscreen
        validate                //validation
    );
    app_editor_entity_manager_init(32);

    running = app_editor_load(&rightPane, &leftPane, &center, inspectors);

    slog("\033[0;32m================= Main Loop ===================\033[0m");
    frameTimer = gf3d_timer_new();
    gf3d_timer_start(&frameTimer);
    while(running)
    {
        if( gf3d_timer_get_ticks(&frameTimer) >= 1.0f )
        {
            slog("fps: %d", fps);
            fps = 0;
            gf3d_timer_start(&frameTimer);
        }
        fps++;

        memset(keys, 0, sizeof(keys));
        memset(mouse, 0, sizeof(mouse));

        while( SDL_PollEvent(&e) )
        {
            if (e.button.button < MAX_INPUT_MOUSE )
            {
                mouse[ e.button.button ] = e;
            }
            if( e.key.keysym.scancode < MAX_INPUT_KEY )
                keys[ e.key.keysym.scancode ] = e;
        }

        for(i = 0; i < GF3D_HUD_TYPE_NUM-1; i++)
        {
            if( app_editor_entity_manager_get_selected() &&
                app_editor_entity_manager_get_selected()->ent.type == i+1 ) 
                {
                    inspectors[i]->visible = 1;
                    inspectorWindow = inspectors[i]->element.window;
                }
            else
            {
                inspectors[i]->visible = 0;
            }
        }

        app_editor_entity_manager_update(keys, mouse);
        update_inspector_values( rightPane->elements[1] );
        gf3d_gui_manager_update(keys, mouse);

        bufferFrame = gf3d_vgraphics_render_begin();
        gf3d_pipeline_reset_frame(gf3d_vgraphics_get_graphics_pipeline(),bufferFrame);
        gf3d_pipeline_reset_frame(gf3d_vgraphics_get_graphics_pipeline2D(), bufferFrame);
            commandBuffer = gf3d_command_rendering_begin(bufferFrame, gf3d_vgraphics_get_graphics_pipeline());

                gf3d_entity_manager_draw(bufferFrame, commandBuffer, worldTime);
                gf3d_entity_manager_draw_collision_boxes(bufferFrame, commandBuffer);
                gf3d_gui_manager_draw(bufferFrame, commandBuffer);

            gf3d_command_rendering_end(commandBuffer);
            
        gf3d_vgraphics_render_end(bufferFrame);

        if( keys[SDL_SCANCODE_ESCAPE].type == SDL_KEYDOWN )
        {
            running = 0;
        }
        else
        {
            switch (keys[SDL_SCANCODE_LCTRL].type)
            {
            case SDL_KEYDOWN:
                lctrl = 1;
                break;
            
            case SDL_KEYUP:
                lctrl = 0;
                break;
            }

            /* reload */
            if( lctrl && keys[SDL_SCANCODE_R].type == SDL_KEYDOWN )
            {
                if( leftWindow && leftWindow->countActual == 0 )
                {
                    running = app_editor_load(&rightPane, &leftPane, &center, inspectors);
                }
            }
            /* next gui type */
            else if ( (e = keys[SDL_SCANCODE_E]).type == SDL_KEYDOWN && e.key.repeat == 0)
            {
                next_hud_type();
                set_add_btn_text(leftPane);
            }
            /* inspector is selected */
            else if ( (e = mouse[SDL_BUTTON_LEFT]).type == SDL_MOUSEBUTTONUP )
            {
                if( e.button.x >= rightPane->bg->position.x && e.button.x < rightPane->bg->position.x + rightPane->bg->extents.x )
                {
                    inspectorSelected = 1;
                }
                else
                {
                    inspectorSelected = 0;
                }
            }
            /* update element values when enter key is pressed */
            else if ( keys[SDL_SCANCODE_RETURN].type == SDL_KEYDOWN )
            {
                update_element_values( rightPane->elements[1] );
            }
        }
    }

    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    slog_sync();

    return 0;
}

uint8_t app_editor_load(Gui **rightPane, Gui **leftPane, Gui **center, HudElement **inspectors)
{
    int i;
    slog("%s=================== Load Entities ==================%s", GREEN_PRINT, PRINT_COLOR_END);
    app_editor_entity_manager_clean();

    if( *rightPane )
    {
        gf3d_gui_free(*rightPane);
    }

    *rightPane = gf3d_gui_load("app_editor_right_pane");
    // *rightPane = gf3d_gui_load("editor_gui_element");
    if(!*rightPane)
    {
        slog("unable to load right pane");
        return 0;
    }
    inspectorWindow = (*rightPane)->elements[2].element.window;
    // inspectors[0] = inspectorWindow;

    for(i = 0; i < GF3D_HUD_TYPE_NUM-1 && i < (*rightPane)->elementCount; i++ )
    {
        inspectors[i] = &(*rightPane)->elements[i+2];
    }

    slog("-==== what type is it %d ====-", (*rightPane)->elements[2].type);

    if( *leftPane )
    {
        gf3d_gui_free(*leftPane);
    }

    *leftPane = gf3d_gui_load("editor_content_view");
    if(!(*leftPane))
    {
        slog("unable to load left pane");
        return 0;
    }
    leftWindow = (*leftPane)->elements[2].element.window;

    if( *center )
    {
        gf3d_gui_free( *center );
    }

    *center = gf3d_gui_load("editor_preview");
    if(!(*center))
    {
        slog("unable to load center preview");
        return 0;
    }
    centerWindow = (*center)->elements[0].element.window;

    set_add_btn_text(*leftPane);

    slog("right pane element count: %d", (*rightPane)->elementCount);
    
    return 1;
}

void next_hud_type()
{   
    eType++;
    if(eType > GF3D_HUD_TYPE_NUM - 1 )
        eType = 1;
    else if (eType < 1) 
        eType = 1;

    slog("current type: %d", eType);
}

void set_add_btn_text(Gui *layer)
{
    HudElement buf = {0};
    char t[GFCLINELEN];
    buf = gf3d_gui_get_element_by_name(layer, "add button");
    if(buf.type == GF3D_HUD_TYPE_BUTTON)
    {
        switch (eType)
        {
        case GF3D_HUD_TYPE_GUI_ELEMENT:
            sprintf(t, "+GE");
            break;

        case GF3D_HUD_TYPE_PROGRESS_BAR:
            sprintf(t, "+PB");
            break;

        case GF3D_HUD_TYPE_BUTTON:
            sprintf(t, "+Btn");
            break;

        case GF3D_HUD_TYPE_LABEL:
            sprintf(t, "+Lbl");
            break;

        case GF3D_HUD_TYPE_TEXT_INPUT:
            sprintf(t, "+TxtIn");
            break;

        case GF3D_HUD_TYPE_WINDOW:
            sprintf(t, "+Win");
            break;

        default:
            sprintf(t, "+");
            break;
        }
        buf.element.button->bg->display->extents.x = strlen(t) * 16.0f;
        buf.element.button->bg->textDisp->extents.x = strlen(t) * 16.0f;
        gf3d_hud_label_set_text( buf.element.button->bg, t );
    }
}

void update_inspector_element(HudElement *e, float val)
{
    char buf[GFCLINELEN];
    if(!e) return;
    if(e->type == GF3D_HUD_TYPE_TEXT_INPUT)
    {
        snprintf(buf, GFCLINELEN, "%.2f", val);
        if( gfc_line_cmp( e->element.textInput->textDisplay->text, buf ) != 0 )
        {
            e->element.textInput->textDisplay->textDisp->extents.x = strlen(buf) * e->element.textInput->textDisplay->size;
            gf3d_hud_label_set_text(e->element.textInput->textDisplay, buf);
        }
    }
}

void update_inspector_element_char(HudElement *e, char *text)
{
    if(!e) return;
    
    switch( e->type )
    {
    case GF3D_HUD_TYPE_TEXT_INPUT:
        if( gfc_line_cmp( e->element.textInput->textDisplay->text, text ) != 0 )
        {
            e->element.textInput->textDisplay->textDisp->extents.x = strlen(text) * 16.0f;
            gf3d_hud_label_set_text(e->element.textInput->textDisplay, text);
        }
        break;

    default:
        break;
    }
}

void update_inspector_values( HudElement nameInput )
{
    EditorEntity *ent = NULL;

    if(inspectorSelected) 
    {
        // slog("inspector selected");
        return;
    }
    if(!inspectorWindow) return;

    ent = app_editor_entity_manager_get_selected();
    if( !ent || ent->dragging ) return;

    update_inspector_element_char(&nameInput, ent->ent.name);

    switch (ent->ent.type)
    {
    case GF3D_HUD_TYPE_GUI_ELEMENT:
        update_inspector_element( &inspectorWindow->elements[GE_POSX],  ent->pos.x );
        update_inspector_element( &inspectorWindow->elements[GE_POSY],  ent->pos.y );
        update_inspector_element( &inspectorWindow->elements[GE_EXTX],  ent->ext.x );
        update_inspector_element( &inspectorWindow->elements[GE_EXTY],  ent->ext.y );
        update_inspector_element( &inspectorWindow->elements[GE_COLR],  ent->ent.element.guiElement->color.x );
        update_inspector_element( &inspectorWindow->elements[GE_COLG],  ent->ent.element.guiElement->color.y );
        update_inspector_element( &inspectorWindow->elements[GE_COLB],  ent->ent.element.guiElement->color.z );
        update_inspector_element( &inspectorWindow->elements[GE_COLA],  ent->ent.element.guiElement->color.w );
        break;

    case GF3D_HUD_TYPE_PROGRESS_BAR:
        update_inspector_element( &inspectorWindow->elements[PB_POSX],  ent->pos.x );
        update_inspector_element( &inspectorWindow->elements[PB_POSY],  ent->pos.y );
        update_inspector_element( &inspectorWindow->elements[PB_EXTX],  ent->ext.x );
        update_inspector_element( &inspectorWindow->elements[PB_EXTY],  ent->ext.y );
        update_inspector_element( &inspectorWindow->elements[PB_BGWX],  ent->ent.element.pBar->bgWidth.x );
        update_inspector_element( &inspectorWindow->elements[PB_BGWY],  ent->ent.element.pBar->bgWidth.y );
        update_inspector_element( &inspectorWindow->elements[PB_BGCR],  ent->ent.element.pBar->back->color.x );
        update_inspector_element( &inspectorWindow->elements[PB_BGCG],  ent->ent.element.pBar->back->color.y );
        update_inspector_element( &inspectorWindow->elements[PB_BGCB],  ent->ent.element.pBar->back->color.z );
        update_inspector_element( &inspectorWindow->elements[PB_BGCA],  ent->ent.element.pBar->back->color.w );
        update_inspector_element( &inspectorWindow->elements[PB_FGCR],  ent->ent.element.pBar->fore->color.x );
        update_inspector_element( &inspectorWindow->elements[PB_FGCG],  ent->ent.element.pBar->fore->color.y );
        update_inspector_element( &inspectorWindow->elements[PB_FGCB],  ent->ent.element.pBar->fore->color.z );
        update_inspector_element( &inspectorWindow->elements[PB_FGCA],  ent->ent.element.pBar->fore->color.w );
        break;

    case GF3D_HUD_TYPE_BUTTON:
        update_inspector_element( &inspectorWindow->elements[BTN_WIN].element.window->elements[ GE_POSX ], ent->pos.x );
        update_inspector_element( &inspectorWindow->elements[BTN_WIN].element.window->elements[ GE_POSY ], ent->pos.y );
        update_inspector_element( &inspectorWindow->elements[BTN_WIN].element.window->elements[ GE_EXTX ], ent->ext.x );
        update_inspector_element( &inspectorWindow->elements[BTN_WIN].element.window->elements[ GE_EXTY ], ent->ext.y );
        update_inspector_element( &inspectorWindow->elements[BTN_WIN].element.window->elements[ GE_COLR ], ent->ent.element.button->bg->display->color.x );
        update_inspector_element( &inspectorWindow->elements[BTN_WIN].element.window->elements[ GE_COLG ], ent->ent.element.button->bg->display->color.y );
        update_inspector_element( &inspectorWindow->elements[BTN_WIN].element.window->elements[ GE_COLB ], ent->ent.element.button->bg->display->color.z );
        update_inspector_element( &inspectorWindow->elements[BTN_WIN].element.window->elements[ GE_COLA ], ent->ent.element.button->bg->display->color.w );
        update_inspector_element( &inspectorWindow->elements[BTN_TCR], ent->ent.element.button->bg->textColor.x );
        update_inspector_element( &inspectorWindow->elements[BTN_TCG], ent->ent.element.button->bg->textColor.y );
        update_inspector_element( &inspectorWindow->elements[BTN_TCB], ent->ent.element.button->bg->textColor.z );
        update_inspector_element( &inspectorWindow->elements[BTN_TCA], ent->ent.element.button->bg->textColor.w );
        update_inspector_element_char( &inspectorWindow->elements[BTN_TXT], ent->ent.element.button->bg->text );
        break;

    default:
        break;
    }
    
}

void update_element_values( HudElement nameInput )
{
    EditorEntity *ent = NULL;
    HudElement *e = NULL;

    Vector2D pos = {0};
    Vector2D ext = {0};
    Vector4D col = {0};
    Vector2D bgw = {0};

    if(!inspectorSelected) return;
    if(!inspectorWindow) return;
    if(nameInput.type != GF3D_HUD_TYPE_TEXT_INPUT) return;

    ent = app_editor_entity_manager_get_selected();
    if(!ent) return;

    e = &ent->ent;
    if( gfc_line_cmp(nameInput.element.textInput->textDisplay->text, e->name) != 0 )
    {
        gfc_line_cpy(e->name, nameInput.element.textInput->textDisplay->text);
    }

    switch(e->type)
    {
    case GF3D_HUD_TYPE_GUI_ELEMENT:
        
        pos.x = (float)atof(inspectorWindow->elements[ GE_POSX ].element.textInput->textDisplay->text);
        pos.y = (float)atof(inspectorWindow->elements[ GE_POSY ].element.textInput->textDisplay->text);
        vector2d_copy(ent->pos, pos);

        ext.x = (float)atof(inspectorWindow->elements[ GE_EXTX ].element.textInput->textDisplay->text);
        ext.y = (float)atof(inspectorWindow->elements[ GE_EXTY ].element.textInput->textDisplay->text);
        vector2d_copy(ent->ext, ext);
        
        col.x = (float)atof(inspectorWindow->elements[ GE_COLR ].element.textInput->textDisplay->text);
        col.y = (float)atof(inspectorWindow->elements[ GE_COLG ].element.textInput->textDisplay->text);
        col.z = (float)atof(inspectorWindow->elements[ GE_COLB ].element.textInput->textDisplay->text);
        col.w = (float)atof(inspectorWindow->elements[ GE_COLA ].element.textInput->textDisplay->text);
        vector4d_copy(e->element.guiElement->color, col);
        
        break;

    case GF3D_HUD_TYPE_PROGRESS_BAR:

        bgw.x = (float)atof(inspectorWindow->elements[ PB_BGWX ].element.textInput->textDisplay->text);
        bgw.y = (float)atof(inspectorWindow->elements[ PB_BGWY ].element.textInput->textDisplay->text);
        vector2d_copy(ent->ent.element.pBar->bgWidth, bgw);

        pos.x = (float)atof(inspectorWindow->elements[ PB_POSX ].element.textInput->textDisplay->text);
        pos.y = (float)atof(inspectorWindow->elements[ PB_POSY ].element.textInput->textDisplay->text);
        vector2d_copy(ent->pos, pos);

        ext.x = (float)atof(inspectorWindow->elements[ PB_EXTX ].element.textInput->textDisplay->text);
        ext.y = (float)atof(inspectorWindow->elements[ PB_EXTY ].element.textInput->textDisplay->text);
        vector2d_copy(ent->ext, ext);
        
        col.x = (float)atof(inspectorWindow->elements[ PB_BGCR ].element.textInput->textDisplay->text);
        col.y = (float)atof(inspectorWindow->elements[ PB_BGCG ].element.textInput->textDisplay->text);
        col.z = (float)atof(inspectorWindow->elements[ PB_BGCB ].element.textInput->textDisplay->text);
        col.w = (float)atof(inspectorWindow->elements[ PB_BGCA ].element.textInput->textDisplay->text);
        vector4d_copy(e->element.pBar->back->color, col);

        col.x = (float)atof(inspectorWindow->elements[ PB_FGCR ].element.textInput->textDisplay->text);
        col.y = (float)atof(inspectorWindow->elements[ PB_FGCG ].element.textInput->textDisplay->text);
        col.z = (float)atof(inspectorWindow->elements[ PB_FGCB ].element.textInput->textDisplay->text);
        col.w = (float)atof(inspectorWindow->elements[ PB_FGCA ].element.textInput->textDisplay->text);
        vector4d_copy(e->element.pBar->fore->color, col);

        break;

    case GF3D_HUD_TYPE_BUTTON:

        

        break;

    default:
        slog("default state...");
        break;
    }
    
    gf3d_hud_element_set_extents(*e, ext);
    app_editor_entity_fix_pos(ent);
    gf3d_hud_element_set_position(*e, ent->pos);
}

void removed_editor_entity(char *name)
{
    int i;
    for(i = 0; i < leftWindow->count; i++)
    {
        if( !leftWindow->elements[i].type ) continue;
        if( gfc_line_cmp(leftWindow->elements[i].element.label->text, name) == 0 )
        {
            gf3d_hud_window_remove_element_at_index(leftWindow, i);
            break;
        }
    }
}