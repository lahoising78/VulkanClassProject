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

#include "gf3d_timer.h"

int main(int argc,char *argv[])
{
    int done = 0;
    int a;
    Uint8 validate = 1;
    const Uint8 * keys;
    Uint32 bufferFrame = 0;
    VkCommandBuffer commandBuffer;
    Player* p1;
    Entity* ent2;
    Timer timer = gf3d_timer_new();

    const Uint32 entity_max = 16;
    const Uint32 player_max = 2;
    // Model *model = NULL;
    // Matrix4 modelMat;
    // Model *model2 = NULL;
    // Matrix4 modelMat2;
    
    for (a = 1; a < argc;a++)
    {
        if (strcmp(argv[a],"-disable_validate") == 0)
        {
            validate = 0;
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
    
    // main game loop
    slog("gf3d main loop begin");
    gf3d_entity_manager_init( entity_max );
    app_player_manager_init( player_max );

    p1 = app_player_new();
    p1->input_handler = app_naruto_input_handler;

    p1->entity = app_naruto_new();
    ent2 = gf3d_entity_new();

    ent2->update = gf3d_entity_general_update;
    
    p1->entity->model = gf3d_model_load("dino");
    gfc_matrix_identity(p1->entity->modelMat);

    ent2->model = gf3d_model_load("dino");
    gfc_matrix_identity(ent2->modelMat);
    ent2->position = vector3d(10, 10, 10);

    // gf3d_camera_set_position(vector3d(1000, 10000, 10000));

    while(!done)
    {
        SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        //update game things here
        
        app_player_manager_update(keys); /* Give input to all players */
        gf3d_entity_manager_update(); /* Update all entities */
        gf3d_timer_start(&timer);

        gfc_matrix_rotate(
            ent2->modelMat,
            ent2->modelMat,
            0.002,
            vector3d(0,0,1));

        gf3d_camera_look_at_center( p1->entity->position, ent2->position );

        // configure render command for graphics command pool
        // for each mesh, get a command and configure it from the pool
        bufferFrame = gf3d_vgraphics_render_begin();
        gf3d_pipeline_reset_frame(gf3d_vgraphics_get_graphics_pipeline(),bufferFrame);
            commandBuffer = gf3d_command_rendering_begin(bufferFrame);

                gf3d_model_draw(p1->entity->model,bufferFrame,commandBuffer,p1->entity->modelMat);
                gf3d_model_draw(ent2->model,bufferFrame,commandBuffer,ent2->modelMat);
                
            gf3d_command_rendering_end(commandBuffer);
            
        gf3d_vgraphics_render_end(bufferFrame);

        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
    }    
    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    slog_sync();
    return 0;
}

/*eol@eof*/
