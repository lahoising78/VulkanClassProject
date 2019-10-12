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
    e->model = gf3d_model_load("naruto");
    e->modelOffset.z = -6.5f;
    e->scale = vector3d(0.05, 0.05, 0.05);
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
    }
    else if (events[SDL_SCANCODE_S].type == SDL_KEYDOWN)
    {
        e->rotation.x = 270.0f;
        vector3d_set_magnitude(&camera_f, MAX_SPEED);
        vector3d_sub(e->velocity, e->velocity, camera_f);
    }
    else if (events[SDL_SCANCODE_W].type == SDL_KEYUP)
    {
        vector3d_clear(e->velocity);
    }
    else if (events[SDL_SCANCODE_S].type == SDL_KEYUP)
    {
        vector3d_clear(e->velocity);
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
    }
    else if (events[SDL_SCANCODE_D].type == SDL_KEYUP)
    {
        vector3d_clear(e->velocity);
    }
    else if (events[SDL_SCANCODE_A].type == SDL_KEYUP)
    {
        vector3d_clear(e->velocity);
    }

    /* Jumping */
    if (events[SDL_SCANCODE_SPACE].type == SDL_KEYDOWN)
    {
        if (distanceToFloor < 0.7f)
        {
            e->state |= ES_Jumping;
            e->acceleration.z = GRAVITY * 40.0f;
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
    // gfc_matrix_rotate(self->modelMat, self->modelMat, ( self->rotation.y + 90 ) * GFC_DEGTORAD, vector3d(1, 0, 0));
}

void app_naruto_touch (struct Entity_S* self, struct Entity_S* other)
{
    
}