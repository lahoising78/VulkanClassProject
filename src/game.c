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

#include <unistd.h>

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
float timeSinceStart = 0.0f;

Stage stage;

// Gui *main_menu = NULL;
// Gui *stage_menu = NULL;
// Gui *pHud = NULL;
// Gui *pause_menu = NULL;
int done = 0;
uint8_t in_game = 0;
uint8_t game_paused = 1;
Player *p = NULL;

void load_valle(Button *btn);
void load_chunin(Button *btn);
void stage_menu_go_back(Button *btn);
void start_game_button(Button *btn);
void exit_game_button(Button *btn);
void resume_game(Button *btn);
void exit_from_fight(Button *btn);

int main(int argc,char *argv[])
{
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
    
    Player* p1 = NULL;
    Entity* ent2 = NULL;

    float frame = 0.0f;
    Timer frameTimer = gf3d_timer_new();
    
    uint8_t lctrl = 0;
    uint8_t lshift = 0;

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

    // main_menu = gf3d_gui_load("main_menu");
    // main_menu->elements[2].element.button->on_click = start_game_button;
    // main_menu->elements[3].element.button->on_click = exit_game_button;
    
    // stage_menu = gf3d_gui_load("stage_menu");
    // stage_menu->elements[2].element.button->on_click = load_valle;
    // stage_menu->elements[3].element.button->on_click = load_chunin;
    // stage_menu->elements[4].element.button->on_click = stage_menu_go_back;
    // stage_menu->active = 0;
    // stage_menu->visible = 0;

    // pause_menu = gf3d_gui_load("pause_menu");
    // pause_menu->elements[3].element.button->on_click = exit_from_fight;
    // pause_menu->elements[4].element.button->on_click = resume_game;
    // pause_menu->active = pause_menu->visible = 0;

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
    stage.fighters[0] = p1->entity;
    stage.fighters[1] = ent2;

    pHud = gf3d_gui_load("health_and_chakra");
    pHud->elements[0].element.pBar->max = &ent2->healthmax;
    pHud->elements[0].element.pBar->val = &ent2->health;
    pHud->elements[1].element.pBar->max = &p1->entity->healthmax;
    pHud->elements[1].element.pBar->val = &p1->entity->health;
    pHud->elements[2].element.pBar->max = &p1->entity->chakraMax;
    pHud->elements[2].element.pBar->val = &p1->entity->chakra;
    pHud->elements[3].element.pBar->max = &ent2->chakraMax;
    pHud->elements[3].element.pBar->val = &ent2->chakra;
    pHud->active = pHud->visible = 0;

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
        
        if(in_game && !game_paused) app_player_manager_update(events); /* Give input to all players */
        if(!game_paused) gf3d_entity_manager_update(); /* Update all entities */
        gf3d_gui_manager_update(events, mouse);

        if(p1->entity && ent2)
        {
            if(p1->entity->health <= 0.0f || ent2->health <= 0.0f)
            {
                exit_from_fight(NULL);
                p1->entity->health = p1->entity->healthmax;
                ent2->health = ent2->healthmax;
            }
        }

        fps++;
        if ( gf3d_timer_get_ticks(&timer) >= 1.0f)
        {
            slog("fps: %.2f", fps);
            fps = 0;
            gf3d_timer_start(&timer);
        }


        // gf3d_vgraphics_rotate_camera(worldTime);
        if(in_game && !game_paused)
        {
            // vector3d_slog(stage.fighters[0]->position);
            gf3d_camera_look_at_center( p1->entity->position, ent2->position );
        }

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
        if( in_game && !game_paused && events[SDL_SCANCODE_M].type == SDL_KEYDOWN )
        {
            game_paused = 1;
            // pause_menu->active = pause_menu->visible = 1;
        }

        switch (events[SDL_SCANCODE_LCTRL].type)
        {
        case SDL_KEYDOWN:
            lctrl = 1;
            break;
        
        case SDL_KEYUP:
            lctrl = 0;
            break;
        }
        
        switch (events[SDL_SCANCODE_LSHIFT].type)
        {
        case SDL_KEYDOWN:
            lshift = 1;
            break;
        
        case SDL_KEYUP:
            lshift = 0;
            break;
        }

        if( lctrl && lshift && events[SDL_SCANCODE_E].type == SDL_KEYDOWN )
        {
            execl("./application", "./application", "-e", NULL);
            done = 1;
        }
        // if(events[SDL_SCANCODE_TAB].type == SDL_KEYDOWN && (gf3d_timer_get_ticks(&drawShapesDelay) > 0.2 || !drawShapesDelay.started || drawShapesDelay.paused) )
        // {
        //     slog("switch character");
        //     gf3d_timer_start(&drawShapesDelay);
        //     if(p1->input_handler == app_naruto_input_handler)
        //     {
        //         p1->input_handler = app_gaara_input_handler;
        //     }
        //     else if (p1->input_handler == app_gaara_input_handler)
        //     {
        //         p1->input_handler = app_naruto_input_handler;
        //     }
        //     ent2->data = p1->entity;
        //     p1->entity = ent2;
        //     ent2 = (Entity*)ent2->data;
        //     p1->entity->data = NULL;
        // }

        frame = gf3d_timer_get_ticks(&frameTimer);
        worldTime = frame;
        timeSinceStart += frame;
    }    
    
    SDL_JoystickClose( controller );
    controller = NULL;
    app_stage_free(&stage);
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    slog_sync();
    return 0;
}

void start_game_button(Button *btn)
{
    slog("hello from start game");
    // main_menu->visible = 0;
    // main_menu->active = 0;
    // stage_menu->visible = 1;
    // stage_menu->active = 1;
}

void exit_game_button(Button *btn)
{
    slog("hello from exit game");
    done = 1;
}

void set_stage_fighters(Stage *stage)
{
    if(!stage) return;
    in_game = 1;
    game_paused = 0;
    pHud->active = pHud->visible = 1;    
}

void load_valle(Button *btn)
{
    slog("hello from load valle");
    stage = app_stage_load("stage_final_valley");
    // stage_menu->visible = stage_menu->active = 0;
    // main_menu->active = main_menu->visible = 0;
    set_stage_fighters(&stage);
}

void load_chunin(Button *btn)
{
    slog("hello from load chunin");
    stage = app_stage_load("stage_chunin_exam");
    // main_menu->active = main_menu->visible = 0;
    // stage_menu->active = stage_menu->visible = 0;
    set_stage_fighters(&stage);
}

void stage_menu_go_back(Button *btn)
{
    slog("go back");
    // main_menu->active = main_menu->visible = 1;
    // stage_menu->active = stage_menu->visible = 0;
}

void resume_game(Button *btn)
{
    slog("yep, go back to playing");
    // pause_menu->visible = pause_menu->active = 0;
    game_paused = 0;
}

void exit_from_fight(Button *btn)
{
    slog("why are you trying to quit? ;-;");
    // pause_menu->visible = pause_menu->active = 0;
    pHud->active = pHud->visible = 0;
    game_paused = 0;
    in_game = 0; 
    // main_menu->active = main_menu->visible = 1;
    app_stage_free(&stage);
}

/*eol@eof*/
