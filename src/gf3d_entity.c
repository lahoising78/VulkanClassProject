#include <stdlib.h>
#include <string.h>

#include "simple_logger.h"
#include "gf3d_entity.h"
#include "gf3d_timer.h"

Timer timer;

typedef struct
{
    Entity *entity_list;
    Uint32 entity_max;
} EntityManager;

static EntityManager gf3d_entity_manager = {0}; /* this is only available on this file */

void gf3d_entity_manager_close()
{
    if(gf3d_entity_manager.entity_list != NULL)
    {
        free(gf3d_entity_manager.entity_list);
    }
    memset(&gf3d_entity_manager, 0, sizeof(EntityManager));
}

void gf3d_entity_manager_init( Uint32 entity_max )
{
    gf3d_entity_manager.entity_list = (Entity*)gfc_allocate_array(sizeof(Entity), entity_max);
    if(!gf3d_entity_manager.entity_list)
    {
        slog("failed to allocate entity list");
        return;
    }
    gf3d_entity_manager.entity_max = entity_max;
    atexit(gf3d_entity_manager_close);
}

void gf3d_entity_manager_update(  )
{
    Entity *e = NULL;
    int i;

    for (i = 0; i < gf3d_entity_manager.entity_max; i++)
    {
        e = &gf3d_entity_manager.entity_list[i];
        if (!e) 
        {
            continue;
        }
        if (!e->_inuse) 
        {
            continue;
        }
        
        e->update(e);
    }

    gf3d_timer_start(&timer);
}

void gf3d_entity_general_update( Entity *self )
{
    Vector3D buff = vector3d(0,0,0);
    float deltaTime;
    float acc, vel;

    deltaTime = gf3d_timer_get_ticks(&timer);

    /* vf = vi + a*t */
    vector3d_scale(buff, self->acceleration, deltaTime);
    vector3d_add(self->velocity, self->velocity, buff);

    /* df = di + v*t */
    vector3d_scale(buff, self->velocity, deltaTime);
    vector3d_add(self->position, self->position, buff);

    vel = vector3d_magnitude(self->velocity);
    acc = vector3d_magnitude(self->acceleration);
    
    if ( acc < 0.05f)
    {
        vector3d_clear(self->acceleration);
        vector3d_clear(self->velocity);
    }
    /* If you are still moving, apply damp */
    else if ( vel )
    {
        vector3d_set_magnitude(&self->acceleration, acc - DAMP_ACCELERATION);
    }
    else 
    {
        vector3d_clear(self->acceleration);
    }

    // slog("a: %f, v: %f", vector3d_magnitude(self->acceleration), mag);

    /* get the actual position of model */
    vector3d_add(buff, self->position, self->modelOffset);
    /* set position of model */
    gfc_matrix_make_translation(self->modelMat, buff);
    
    /* set scale of model */
    gf3d_model_scale( self->modelMat, self->scale);

    /* set rotation of model */
    gfc_matrix_rotate(self->modelMat, self->modelMat, (self->rotation.x + 90) * GFC_DEGTORAD, vector3d(0, 0, 1));
}

Entity *gf3d_entity_new()
{
    Entity *ent = NULL;
    int i;
    for(i = 0; i <gf3d_entity_manager.entity_max; i++)
    {
        if (gf3d_entity_manager.entity_list[i]._inuse) continue;

        memset(&gf3d_entity_manager.entity_list[i], 0, sizeof(Entity));
        ent = &gf3d_entity_manager.entity_list[i];

        /* stuff to do when initiating an entity */
        gf3d_entity_manager.entity_list[i]._inuse = 1;
        ent->update = gf3d_entity_general_update;
        ent->scale = vector3d(1,1,1);
        
        return &gf3d_entity_manager.entity_list[i];
    }
    slog("request for entity failed: all full up");
    return NULL;
}

void gf3d_entity_free(Entity *self)
{
    if (!self)
    {
        slog("self pointer is not valid");
        return;
    }

    self->_inuse = 0;

    if (self->data != NULL)
    {
        slog("warning: data not freed at entity free");
    }
}
