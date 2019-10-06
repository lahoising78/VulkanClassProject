#include "simple_logger.h"
#include "app_naruto.h"

Entity *app_naruto_new()
{
    Entity *e = NULL;

    e = gf3d_entity_new();
    e->health = 100;
    e->healthmax = 100;
    e->think = app_naruto_think;
    e->touch = app_naruto_touch;
    e->update = app_naruto_update;
    e->model = gf3d_model_load("Naruto");
    e->modelOffset.z = -6.5f;
    gfc_matrix_identity(e->modelMat);

    vector3d_clear(e->rotation);
    vector3d_clear(e->position);
    vector3d_clear(e->velocity);
    vector3d_clear(e->acceleration);

    return e;
}

void app_naruto_input_handler( struct Player_s *self, const Uint8* keys )
{
    Entity *e = self->entity;
    Vector3D forward;

    if (keys[SDL_SCANCODE_W])
    {
        vector3d_angle_vectors(e->rotation, &forward, NULL, NULL);
        vector3d_set_magnitude(&forward, START_SPEED);
        vector3d_add(e->velocity, e->velocity, forward);
        if ( vector3d_magnitude(e->velocity) > MAX_SPEED )
        {
            vector3d_set_magnitude( &e->acceleration, MAX_SPEED );
        }
    }
    else if (keys[SDL_SCANCODE_S])
    {
        vector3d_angle_vectors(e->rotation, &forward, NULL, NULL);
        vector3d_set_magnitude(&forward, START_SPEED);
        vector3d_sub(e->velocity, e->velocity, forward);
        if (vector3d_magnitude(e->velocity) > MAX_SPEED)
        {
            vector3d_set_magnitude(&e->velocity, MAX_SPEED);
        }
    }

    if (keys[SDL_SCANCODE_D])
    {
        if (e->rotation.x < 0)
        {
            e->rotation.x = 360.0f;
        }
        else
        {
            e->rotation.x--;
        }
    }
    else if (keys[SDL_SCANCODE_A])
    {
        if (e->rotation.x > 360)
        {
            e->rotation.x = 0;
        }
        else
        {
            e->rotation.x++;
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
    gf3d_entity_general_touch(self, other);
}