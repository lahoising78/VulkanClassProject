#include "simple_logger.h"
#include "app_naruto.h"
#include "gf3d_camera.h"

Entity *app_naruto_new()
{
    Entity *e = NULL;

    e = gf3d_entity_new();
    e->health = 100;
    e->healthmax = 100;
    e->think = app_naruto_think;
    e->touch = app_naruto_touch;
    e->update = app_naruto_update;
    e->model = gf3d_model_load("naruto", NULL);
    gf3d_mesh_free(e->model->mesh[0]);
    e->animationManager = gf3d_animation_manager_init(5, e->model);
    gf3d_animation_load(e->animationManager, "idle", "naruto_idle", 1, 67);
    gf3d_animation_load(e->animationManager, "running", "naruto_running", 1, 20);
    gf3d_animation_load(e->animationManager, "jump up", "naruto_jump_up", 1, 25);
    gf3d_animation_load(e->animationManager, "jump down", "naruto_jump_down", 1, 12);
    gf3d_animation_play(e->animationManager, "idle", 1);
    e->modelOffset.z = -6.5f;
    e->scale = vector3d(5, 5, 5);
    // e->scale = vector3d(1, 1, 1);
    // e->scale = vector3d(0.1, 0.1, 0.1);
    gfc_matrix_identity(e->modelMat);

    return e;
}

void app_naruto_input_handler( struct Player_s *self, SDL_Event* events )
{
    Entity *e = self->entity;
    Vector3D camera_f, camera_r;
    Uint8 onFloor = 0;
    float distanceToFloor = 0.0f;
    int i;

    const int usedScancodes[] = {
        SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_A, SDL_SCANCODE_SPACE
    };

    if (self->entity->modelBox)
    {
        distanceToFloor = distance_to_floor( e->modelBox->shapes[0].position.z - e->modelBox->shapes[0].extents.z );
        onFloor = on_floor( distanceToFloor );
    }
    
    /* Get camera angles */
    gf3d_camera_get_angles(&camera_f, &camera_r, NULL);
    vector3d_normalize(&camera_f);
    vector3d_normalize(&camera_r);

    /* Forward and Backwards */
    if (events[SDL_SCANCODE_W].type == SDL_KEYDOWN)
    {
        e->rotation.x = 90.0f;
        vector3d_set_magnitude(&camera_f, MAX_SPEED);
        vector3d_add(e->velocity, e->velocity, camera_f);
        if(!gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
        {
            gf3d_animation_play(e->animationManager, "running", 1);
        }
    }
    else if (events[SDL_SCANCODE_S].type == SDL_KEYDOWN)
    {
        e->rotation.x = 270.0f;
        vector3d_set_magnitude(&camera_f, MAX_SPEED);
        vector3d_sub(e->velocity, e->velocity, camera_f);
        if(!gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
        {
            gf3d_animation_play(e->animationManager, "running", 1);
        }
    }
    else if (events[SDL_SCANCODE_W].type == SDL_KEYUP)
    {
        vector3d_clear(e->velocity);

        if( gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
        {
            gf3d_animation_play(e->animationManager, "idle", 1);
        }
    }
    else if (events[SDL_SCANCODE_S].type == SDL_KEYUP)
    {
        vector3d_clear(e->velocity);

        if( gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
        {
            gf3d_animation_play(e->animationManager, "idle", 1);
        }
    }

    /* Right and Left */
    if (events[SDL_SCANCODE_D].type == SDL_KEYDOWN)
    {
        /* To rotate according to direction */
        if (e->velocity.y > 0)
        {
            e->rotation.x = 45.0f;
        }
        else if (e->velocity.y < 0)
        {
            e->rotation.x = -45.0f;
        }
        else
        {
            e->rotation.x = 0.0f;
        }
        
        vector3d_set_magnitude(&camera_r, MAX_SPEED);
        vector3d_sub(e->velocity, e->velocity, camera_r);
        if(!gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
        {
            gf3d_animation_play(e->animationManager, "running", 1);
        }
    }
    else if (events[SDL_SCANCODE_A].type == SDL_KEYDOWN)
    {
        /* To rotate according to direction */
        if (e->velocity.y > 0)
        {
            e->rotation.x = 135.0f;
        }
        else if (e->velocity.y < 0)
        {
            e->rotation.x = 225.0f;
        }
        else
        {
            e->rotation.x = 180.0f;
        }
        
        vector3d_set_magnitude(&camera_r, MAX_SPEED);
        vector3d_add(e->velocity, e->velocity, camera_r);
        if(!gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
        {
            gf3d_animation_play(e->animationManager, "running", 1);
        }
    }
    else if (events[SDL_SCANCODE_D].type == SDL_KEYUP)
    {
        vector3d_clear(e->velocity);
        if( gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
        {
            gf3d_animation_play(e->animationManager, "idle", 1);
        }
    }
    else if (events[SDL_SCANCODE_A].type == SDL_KEYUP)
    {
        vector3d_clear(e->velocity);
        if( gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
        {
            gf3d_animation_play(e->animationManager, "idle", 1);
        }
    }

    /* Jumping */
    if (events[SDL_SCANCODE_SPACE].type == SDL_KEYDOWN)
    {
        if (distanceToFloor < 0.7f)
        {
            e->state |= ES_Jumping;
            e->acceleration.z = GRAVITY * 40.0f;
            gf3d_animation_play(e->animationManager, "jump up", 1);
        }
    }

    /* 
    since we set the events in an array, even if we stop using
    a key, we get the key as input after we stop pressing it.
    To ignore these, we set the type to something else
    */
    for(i = 0; i < sizeof(usedScancodes) / sizeof(int); i++)
    {
        if ( events[ usedScancodes[i] ].type == SDL_KEYUP )
            events[ usedScancodes[i] ].type = -SDL_KEYUP;
    }
}

void app_naruto_think (struct Entity_S* self)
{

}

void app_naruto_update(struct Entity_S* self)
{
    gf3d_entity_general_update(self);
    gfc_matrix_rotate(self->modelMat, self->modelMat, ( self->rotation.y + 90 ) * GFC_DEGTORAD, vector3d(1, 0, 0));
}

void app_naruto_touch (struct Entity_S* self, struct Entity_S* other)
{
    
}