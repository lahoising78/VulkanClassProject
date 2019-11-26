#include "app_editor.h"

#include "simple_logger.h"
#include "simple_json.h"

#include "gf3d_entity.h"
#include "gf3d_timer.h"
#include "gf3d_gui.h"

#define MAX_INPUT_KEY SDL_NUM_SCANCODES
#if SDL_BUTTON_X2 < 8
    #define MAX_INPUT_MOUSE 8
#else
    #define MAX_INPUT_MOUSE SDL_BUTTON_X2
#endif

#define GREEN_PRINT "\033[0;32m"
#define PRINT_COLOR_END "\033[0m"

uint8_t app_editor_load(Gui **rightPane, Gui **leftPane);

int screenWidth = 1800;
int screenHeight = 700;

uint8_t lctrl = 0;
uint8_t rctrl = 0;

void hola(Button *btn);

OnClickCallback on_clicks[32] = {hola};

void hola(Button *btn)
{
    slog("hola");
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

    running = app_editor_load(&rightPane, &leftPane);

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
            if( e.key.keysym.scancode < MAX_INPUT_KEY )
                keys[ e.key.keysym.scancode ] = e;
            else if (e.button.button < MAX_INPUT_MOUSE )
                mouse[ e.button.button ] = e;
        }

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
                running = app_editor_load(&rightPane, &leftPane);
            }
        }
    }

    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    slog_sync();

    return 0;
}

uint8_t app_editor_load(Gui **rightPane, Gui **leftPane)
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

    *leftPane = gf3d_gui_load("editor_content_view");
    if(!(*leftPane))
    {
        slog("unable to load left pane");
        return 0;
    }

    slog("right pane element count: %d", (*rightPane)->elementCount);
    
    return 1;
}