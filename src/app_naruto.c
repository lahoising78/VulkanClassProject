#include "simple_logger.h"
#include "app_naruto.h"
#include "gf3d_camera.h"

void app_naruto_punch(Entity *e);

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
    e->animationManager = gf3d_animation_manager_init(10, e->model);
    gf3d_animation_load(e->animationManager, "idle", "naruto_idle", 1, 67);
    gf3d_animation_load(e->animationManager, "running", "naruto_running", 1, 20);
    gf3d_animation_load(e->animationManager, "jump up", "naruto_jump_up", 1, 25);
    gf3d_animation_load(e->animationManager, "jump down", "naruto_jump_down", 1, 12);
    gf3d_animation_load(e->animationManager, "punch", "naruto_punch", 1, 27);
    gf3d_animation_load(e->animationManager, "kick", "naruto_kick", 1, 49);
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

    /* Jumping */
    if (events[SDL_SCANCODE_SPACE].type == SDL_KEYDOWN && !e->locked)
    {
        if (distanceToFloor < 0.7f)
        {
            e->state |= ES_Jumping;
            e->acceleration.z = GRAVITY * 40.0f;
            gf3d_animation_play(e->animationManager, "jump up", 1);
        }
    }
    /* Punching */
    else if (events[SDL_SCANCODE_J].type == SDL_KEYDOWN)
    {
        app_naruto_punch(e);
    }

    if (e->locked) return;
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

void app_naruto_punch(struct Entity_S* e)
{
    // int punchCount = e->locked;
    float fcount = 0.0f;
    float currf  = 0.0f;
    if(e->state & ES_Jumping)
    {

    }
    else if(e->state & ES_Idle)
    {

        if ( !gf3d_animation_is_playing(e->animationManager, "punch") )
        {
            /* start punching animation, set attacking flag, and clean velocity */
            gf3d_animation_play(e->animationManager, "punch", 1);
            e->state &= ~ES_Idle;
            e->state |= ES_Attacking;
            e->velocity.x = e->velocity.y = 0.0f;
            e->locked = 1;
        }
    }
    else if (e->state & ES_Attacking)
    {
        if ( gf3d_animation_is_playing(e->animationManager, "punch") )
        {
            fcount = gf3d_animation_get_frame_count(e->animationManager, "punch");
            currf = gf3d_animation_get_current_frame(e->animationManager);
            if ( fcount - currf <= 8.0f )
            {
                e->locked = 2;
            }
        }
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