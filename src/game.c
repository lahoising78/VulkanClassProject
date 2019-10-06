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
#include "gf3d_shape.h"

int main(int argc,char *argv[])
{
    int done = 0;
    int a;
    Uint8 validate = 1;
    Uint8 drawShapes = 0;
    const Uint8 * keys;
    Uint32 bufferFrame = 0;
    VkCommandBuffer commandBuffer;
    Player* p1;
    Entity* ent2;
    Timer timer = gf3d_timer_new(), drawShapesDelay = gf3d_timer_new();
    Shape s;

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
        else if (strcmp(argv[a], "-s") == 0)
        {
            drawShapes = 1;
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

    ent2->model = gf3d_model_load("dino");
    gfc_matrix_identity(ent2->modelMat);
    ent2->update = gf3d_entity_general_update;
    ent2->touch = gf3d_entity_general_touch;
    
    s = gf3d_shape( vector3d(-10, -10, -10), vector3d(0.5f, 0.5f, 0.5f), gf3d_model_load("cube") );

    // ent2->position = vector3d(10, 10, 10);
    // p1->entity->scale.z = 3;
    gf3d_entity_add_hurtboxes(p1->entity, 1);
    gf3d_collision_armor_add_shape( 
        p1->entity->hurtboxes,
        gf3d_shape( p1->entity->position, vector3d(1, 1, 2), gf3d_model_load("cube") ),
        vector3d(0, 0, 0)
    );
    
    gf3d_entity_add_hurtboxes(ent2, 1);
    gf3d_collision_armor_add_shape(
        ent2->hurtboxes, 
        gf3d_shape( ent2->position, vector3d(3, 3, 2), gf3d_model_load("cube") ),
        vector3d(0, 0, -2)
    );

    while(!done)
    {
        SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        //update game things here
        
        app_player_manager_update(keys); /* Give input to all players */
        gf3d_entity_manager_update(); /* Update all entities */

        if (drawShapes)
        {
            gf3d_shape_update_mat(&s);
        }

        gf3d_timer_start(&timer);

        gf3d_camera_look_at_center( p1->entity->position, ent2->position );

        // configure render command for graphics command pool
        // for each mesh, get a command and configure it from the pool
        bufferFrame = gf3d_vgraphics_render_begin();
        gf3d_pipeline_reset_frame(gf3d_vgraphics_get_graphics_pipeline(),bufferFrame);
            commandBuffer = gf3d_command_rendering_begin(bufferFrame);

                gf3d_model_draw(p1->entity->model,bufferFrame,commandBuffer,p1->entity->modelMat);
                gf3d_model_draw(ent2->model,bufferFrame,commandBuffer,ent2->modelMat);
                if ( drawShapes ) 
                {
                    gf3d_model_draw(s.model, bufferFrame, commandBuffer, s.matrix);
                    gf3d_entity_manager_draw_hurtboxes(bufferFrame, commandBuffer);
                }
                
            gf3d_command_rendering_end(commandBuffer);
            
        gf3d_vgraphics_render_end(bufferFrame);

        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
        if (keys[SDL_SCANCODE_BACKSLASH] && (gf3d_timer_get_ticks(&drawShapesDelay) > 0.1 || !drawShapesDelay.started || drawShapesDelay.paused)  )
        {
            gf3d_timer_start(&drawShapesDelay);
            drawShapes = !drawShapes; /* toggle drawing shapes */
        }
    }    
    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    slog_sync();
    return 0;
}

/*eol@eof*/
