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
    int i;
    for(i = 0; i < gf3d_entity_manager.entity_max; i++)
    {
        gf3d_entity_free(&gf3d_entity_manager.entity_list[i]);
    }

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
    Entity *e = NULL, *o = NULL;
    int i, j;

    for (i = 0; i < gf3d_entity_manager.entity_max; i++)
    {
        e = &gf3d_entity_manager.entity_list[i];
        if (!e->_inuse)
        {
            continue;
        }
        if (!e->update) 
        {
            gf3d_collision_armor_update(e->hurtboxes, e->position);
            continue;
        }
        
        e->update(e);
    }

    /* Checking collision */
    for(i = 0; i < gf3d_entity_manager.entity_max; i++)
    {
        e = &gf3d_entity_manager.entity_list[i];
        if ( !e->_inuse || !e->touch || !e->hurtboxes ) continue;
        for(j = i+1; j < gf3d_entity_manager.entity_max; j++)
        {
            o = &gf3d_entity_manager.entity_list[j];
            if ( !o->_inuse || !o->hurtboxes || o == e ) continue;
            
            if (!gf3d_collision_check(e->hurtboxes, o->hurtboxes) )
            {
                continue;
            }

            e->touch(e, o);
        }
    }
    gf3d_timer_start(&timer);
}

void gf3d_entity_manager_draw_hurtboxes( Uint32 bufferFrame, VkCommandBuffer commandBuffer )
{
    Entity *ent;
    int i;
    for(i = 0; i < gf3d_entity_manager.entity_max; i++)
    {
        ent = &gf3d_entity_manager.entity_list[i];
        gf3d_collision_armor_draw(ent->hurtboxes, bufferFrame, commandBuffer);
    }
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
    /* update collision boxes */
    gf3d_collision_armor_update(self->hurtboxes, self->position);

    /* get the actual position of model */
    vector3d_add(buff, self->position, self->modelOffset);
    /* set position of model */
    gfc_matrix_make_translation(self->modelMat, buff);
    
    /* set scale of model */
    gf3d_model_scale( self->modelMat, self->scale);

    /* set rotation of model */
    gfc_matrix_rotate(self->modelMat, self->modelMat, (self->rotation.x + 90) * GFC_DEGTORAD, vector3d(0, 0, 1));
}

void gf3d_entity_general_touch( Entity *self, Entity *other )
{
    float smag, omag; /* to store velocity magnitude squared of self and other */
    Vector3D dir;
    if ( vector3d_equal( self->position, other->position) )
    {
        other->position.x += 1;
    }

    smag = vector3d_magnitude_squared(self->velocity);
    omag = vector3d_magnitude_squared(other->velocity); 
    if ( omag > smag)
    {
        vector3d_copy(dir, other->velocity);
    }
    else if ( omag < smag )
    {
        vector3d_copy(dir, self->velocity);
    }
    else
    {
        vector3d_sub(dir, other->position, self->position);
    }
    vector3d_normalize(&dir);

    while( gf3d_collision_check(self->hurtboxes, other->hurtboxes) )
    {
        vector3d_add(other->position, other->position, dir);
        gf3d_collision_armor_update(other->hurtboxes, other->position);
    }
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
        ent->scale = vector3d(1,1,1);
        ent->hurtboxes = NULL;
        
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

    if(self->hurtboxes) gf3d_collision_armor_free(self->hurtboxes);
}

void gf3d_entity_add_hurtboxes( Entity *ent, Uint32 count)
{
    if (!count || !ent) return;

    ent->hurtboxes = gf3d_collision_armor_new( count );
    if (!ent->hurtboxes)
    {
        slog("didn't add them right");
        return;
    }
    slog("added the armor");
}