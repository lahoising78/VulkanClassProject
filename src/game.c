#include <SDL.h>   
#include "SDL.h"         

#include "simple_logger.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_model.h"
#include "gf3d_camera.h"
#include "gf3d_texture.h"
#include "gf3d_entity.h"

#include "app_player.h"
#include "app_naruto.h"
#include "app_gaara.h"
#include "app_stage.h"

#include "gf3d_timer.h"
#include "gf3d_shape.h"

#include "gf3d_gui.h"
#include "gf3d_hud.h"
#include "gfc_color.h"
#include "app_editor.h"

#define HEALTH_HEIGHT       0.025f
#define HEALTH_DOWN_OFFSET  -0.385f

#define CHAKRA_HEIGHT       0.0125f
#define CHAKRA_DOWN_OFFSET  -0.425f

#if SDL_BUTTON_X2 < 8
    #define MOUSE_EVENT_MAX 8
#else
    #define MOUSE_EVENT_MAX SDL_BUTTON_X2
#endif

float worldTime = 0.0f;

void click(Button *button)
{
    slog("stop poking me senpai >.<");
}

int main(int argc,char *argv[])
{
    int done = 0;
    int a;
    Uint8 validate = 1;
    Uint8 drawShapes = 0;
    SDL_Event events[ SDL_NUM_SCANCODES ];
    SDL_Event mouse[ MOUSE_EVENT_MAX ];
    SDL_Event e;
    Uint32 bufferFrame = 0;
    VkCommandBuffer commandBuffer;
    Timer timer = gf3d_timer_new(), drawShapesDelay = gf3d_timer_new();
    float fps = 0;

    const Uint32 entity_max = 16;
    const Uint32 player_max = 2;
    
    Player* p1;
    Entity* ent2;
    Entity* stage;
    Stage stag;

    float frame = 0.0f;
    Timer frameTimer = gf3d_timer_new();

    /* controllers */
    SDL_Joystick *controller = NULL;

    for (a = 1; a < argc;a++)
    {
        if (strcmp(argv[a],"-disable_validate") == 0)
        {
            validate = 0;
        }
        else if (strcmp(argv[a], "-s") == 0)
        {
            drawShapes = 1;
        }
        else if ( strcmp(argv[a], "-edit") == 0 || strcmp(argv[a], "-e") == 0 )
        {
            return app_editor_main(argc, argv);
        }
    }

    init_logger("gf3d.log");    
    slog("gf3d begin");
    gf3d_vgraphics_init(
        "program name",         //program name
        1200,                   //screen width
        700,                    //screen height
        vector4d(0.51,0.75,1,1),//background color
        0,                      //fullscreen
        validate                //validation
    );
    // Shape guis[8];

    if( SDL_NumJoysticks() < 1 )
    {
        slog("No joysticks connected");
    }
    else
    {
        controller = SDL_JoystickOpen(0);
        if(!controller)
        {
            slog("Couldn't open game controller: %s", SDL_GetError());
        }
    }

    // main game loop
    slog("gf3d main loop begin");
    gf3d_entity_manager_init( entity_max );
    app_player_manager_init( player_max );
    gf3d_animation_manager_all_init(8);

    // /* Set up the stage */
    // stage->healthmax = stage->health = 5000.0f;
    // stage = gf3d_entity_new();
    // stage->position = vector3d(0, 0, MAX_STAGE_Z);
    // stage->scale = vector3d(MAX_STAGE_X, MAX_STAGE_Y, STAGE_SCALE_Z);
    // stage->model = gf3d_model_load("stage", NULL);
    // gfc_matrix_identity(stage->modelMat);
    // gfc_matrix_make_translation(stage->modelMat, stage->position);
    // gf3d_model_scale(stage->modelMat, stage->scale);

    stag = app_stage_load("stage_final_valley");

    /* Setup first player */
    p1 = app_player_new();
    p1->input_handler = app_naruto_input_handler;
    p1->entity = app_naruto_new();
    p1->entity->position = vector3d(10, 10, 0);
    p1->entity->rotation = vector3d(0, 0, 0);
    
    p1->entity->modelBox = gf3d_collision_armor_new(2);
    gf3d_collision_armor_add_shape( 
        p1->entity->modelBox,
        gf3d_shape( p1->entity->position, vector3d(1, 1, 4), gf3d_model_load("cube", NULL) ),
        // vector3d(0, 0, 0)
        vector3d(0, 0, -0.3),
        "entire body"
    );
    
    p1->entity->hurtboxes = gf3d_collision_armor_new(3);
    gf3d_collision_armor_add_shape(
        p1->entity->hurtboxes,
        gf3d_shape(p1->entity->position, vector3d(1, 1, 1), gf3d_model_load("cube", NULL)),
        vector3d(0, 0, 1.2),
        "head"
    );
    gf3d_collision_armor_add_shape(
        p1->entity->hurtboxes,
        gf3d_shape(p1->entity->position, vector3d(1, 1, 1.5), gf3d_model_load("cube", NULL)),
        vector3d(0, 0, 0),
        "torso"
    );
    gf3d_collision_armor_add_shape(
        p1->entity->hurtboxes,
        gf3d_shape(p1->entity->position, vector3d(1, 1, 1.5f), gf3d_model_load("cube", NULL)),
        vector3d(0, 0, -3.0f),
        "legs"
    );
    
    p1->entity->hitboxes = gf3d_collision_armor_new(3);

    /* Setup second player */
        // ent2 = gf3d_entity_new();
        // ent2->position = vector3d(0,0, 0);
        // ent2->scale = vector3d(1, 1, 1);
        // ent2->model = gf3d_model_load("dino", "dino");
        // gfc_matrix_identity(ent2->modelMat);
        // gfc_matrix_make_translation(ent2->modelMat, ent2->position);
        // gf3d_model_scale(ent2->modelMat, ent2->scale);
    ent2 = app_gaara_new();
    ent2->position = vector3d(-10, -10, 0);
    
    ent2->modelBox = gf3d_collision_armor_new(1);
    gf3d_collision_armor_add_shape(
        ent2->modelBox, 
        gf3d_shape( ent2->position, vector3d(1, 1, 4), gf3d_model_load("cube", NULL) ),
        vector3d(0, 0, -0.3),
        "entire body"
    );

    ent2->hurtboxes = gf3d_collision_armor_new(3);
    gf3d_collision_armor_add_shape(
        ent2->hurtboxes,
        gf3d_shape(ent2->position, vector3d(1, 1, 1), gf3d_model_load("cube", NULL)),
        vector3d(0, 0, 1.2),
        "head"
    );
    gf3d_collision_armor_add_shape(
        ent2->hurtboxes,
        gf3d_shape(ent2->position, vector3d(1, 1, 1.5), gf3d_model_load("cube", NULL)),
        vector3d(0, 0, 0),
        "torso"
    );
    gf3d_collision_armor_add_shape(
        ent2->hurtboxes,
        gf3d_shape(ent2->position, vector3d(1, 1, 1.5f), gf3d_model_load("cube", NULL)),
        vector3d(0, 0, -3.0f),
        "legs"
    );

    // just in case we need it later
    gf3d_model_load("dino", "dino");
    gf3d_model_load("sand", "sand");
    gf3d_model_load("shuriken", "shuriken");

    ent2->enemy = p1->entity;
    p1->entity->enemy = ent2;

    gf3d_timer_start(&timer);
    gf3d_animation_manager_timer_start();
    
    while(!done)
    {
        //update game things here
        gf3d_timer_start(&frameTimer);

        memset(mouse, 0, sizeof(mouse));
        memset(mouse, 0, sizeof(events));

        while( SDL_PollEvent(&e) )
        {
            if ( e.key.keysym.scancode < SDL_NUM_SCANCODES )
                events[ e.key.keysym.scancode ] = e;
            else if ( e.button.button < 128 )
                mouse[e.button.button] = e;
            // slog("key: %d", e.key.keysym.scancode);
        }
        
        app_player_manager_update(events); /* Give input to all players */
        // stage->health -= worldTime * 50;
        gf3d_entity_manager_update(); /* Update all entities */
        gf3d_vgraphics_rotate_camera(worldTime);
        gf3d_gui_manager_update(events, mouse);

        fps++;
        if ( gf3d_timer_get_ticks(&timer) >= 1.0f)
        {
            slog("fps: %.2f", fps);
            fps = 0;
            gf3d_timer_start(&timer);
        }

        gf3d_camera_look_at_center( p1->entity->position, ent2->position );

        // configure render command for graphics command pool
        // for each mesh, get a command and configure it from the pool
        bufferFrame = gf3d_vgraphics_render_begin();
        gf3d_pipeline_reset_frame(gf3d_vgraphics_get_graphics_pipeline(),bufferFrame);
        gf3d_pipeline_reset_frame(gf3d_vgraphics_get_graphics_pipeline2D(), bufferFrame);
            commandBuffer = gf3d_command_rendering_begin(bufferFrame, gf3d_vgraphics_get_graphics_pipeline());

                gf3d_entity_manager_draw(bufferFrame, commandBuffer, frame);
                if ( drawShapes ) 
                {
                    gf3d_entity_manager_draw_collision_boxes(bufferFrame, commandBuffer);
                }
                gf3d_gui_manager_draw(bufferFrame, commandBuffer);

            gf3d_command_rendering_end(commandBuffer);
            
        gf3d_vgraphics_render_end(bufferFrame);
        gf3d_animation_manager_timer_start();

        if (events[SDL_SCANCODE_ESCAPE].type == SDL_KEYDOWN)done = 1; // exit condition
        if (events[SDL_SCANCODE_BACKSLASH].type == SDL_KEYDOWN && (gf3d_timer_get_ticks(&drawShapesDelay) > 0.2 || !drawShapesDelay.started || drawShapesDelay.paused)  )
        {
            gf3d_timer_start(&drawShapesDelay);
            drawShapes = !drawShapes; /* toggle drawing shapes */
        }
        if(events[SDL_SCANCODE_TAB].type == SDL_KEYDOWN && (gf3d_timer_get_ticks(&drawShapesDelay) > 0.2 || !drawShapesDelay.started || drawShapesDelay.paused) )
        {
            slog("switch character");
            gf3d_timer_start(&drawShapesDelay);
            if(p1->input_handler == app_naruto_input_handler)
            {
                p1->input_handler = app_gaara_input_handler;
            }
            else if (p1->input_handler == app_gaara_input_handler)
            {
                p1->input_handler = app_naruto_input_handler;
            }
            ent2->data = p1->entity;
            p1->entity = ent2;
            ent2 = (Entity*)ent2->data;
            p1->entity->data = NULL;
        }

        frame = gf3d_timer_get_ticks(&frameTimer);
        worldTime = frame;
        
    }    
    
    SDL_JoystickClose( controller );
    controller = NULL;
    app_stage_free(&stag);
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    slog_sync();
    return 0;
}

/*eol@eof*/
