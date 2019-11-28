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

uint8_t app_editor_load(Gui **rightPane, Gui **leftPane, Gui **center);
Window *centerWindow = NULL;

int screenWidth = 1800;
int screenHeight = 700;

uint8_t lctrl = 0;
uint8_t rctrl = 0;

HudType eType = GF3D_HUD_TYPE_GUI_ELEMENT;

void add_editor_entity(Button *btn);
void next_hud_type();
void set_add_btn_text(Gui *layer);

OnClickCallback on_clicks[32] = {add_editor_entity};

void add_editor_entity(Button *btn)
{
    EditorEntity *e = NULL;
    if(!centerWindow) return;

    e = app_editor_entity_create();
    if(!e) return;
    e->pos = vector2d(0.0f, 0.0f);
    e->ext = vector2d(50.0f, 50.0f);
    e->parent = centerWindow;
    e->ent.type = eType;
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

    running = app_editor_load(&rightPane, &leftPane, &center);

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

        app_editor_entity_manager_update(keys, mouse);
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
                running = app_editor_load(&rightPane, &leftPane, &center);
            }
            /* next gui type */
            else if ( (e = keys[SDL_SCANCODE_E]).type == SDL_KEYDOWN && e.key.repeat == 0)
            {
                next_hud_type();
                set_add_btn_text(leftPane);
            }
        }
    }

    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    slog_sync();

    return 0;
}

uint8_t app_editor_load(Gui **rightPane, Gui **leftPane, Gui **center)
{
    slog("%s=================== Load Entities ==================%s", GREEN_PRINT, PRINT_COLOR_END);

    if( *rightPane )
    {
        gf3d_gui_free(*rightPane);
    }

    // *rightPane = gf3d_gui_load("app_editor_right_pane");
    *rightPane = gf3d_gui_load("editor_gui_element");
    if(!*rightPane)
    {
        slog("unable to load right pane");
        return 0;
    }

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