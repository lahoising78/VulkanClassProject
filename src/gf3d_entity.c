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
static float deltaTime = 0.0f;

void gf3d_entity_simple_collision( Entity *self, Entity *other );

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

    deltaTime = gf3d_timer_get_ticks(&timer);

    for (i = 0; i < gf3d_entity_manager.entity_max; i++)
    {
        e = &gf3d_entity_manager.entity_list[i];
        if (!e->_inuse)
        {
            continue;
        }
        if (!e->update) 
        {
            gf3d_collision_armor_update(e->hitboxes, e->position, e->rotation);
            gf3d_collision_armor_update(e->hurtboxes, e->position, e->rotation);
            gf3d_collision_armor_update(e->modelBox, e->position, e->rotation);
            continue;
        }
        
        e->update(e);
    }

    /* Checking collision */
    for(i = 0; i < gf3d_entity_manager.entity_max; i++)
    {
        e = &gf3d_entity_manager.entity_list[i];
        if ( !e->_inuse ) continue;

        for(j = i+1; j < gf3d_entity_manager.entity_max; j++)
        {
            o = &gf3d_entity_manager.entity_list[j];
            if ( !o->_inuse || o == e ) continue;
            
            if ( e->hurtboxes && o->hitboxes )
            {
                if ( o->touch && gf3d_collision_check(e->hurtboxes, o->hitboxes) )
                {
                    o->touch(o, e);
                    continue;
                }
            }
            else if ( e->hitboxes && o->hurtboxes )
            {
                if ( e->touch && gf3d_collision_check(e->hitboxes, o->hurtboxes) )
                {
                    e->touch(e, o);
                }
            }

            if ( e->modelBox && o->modelBox && gf3d_collision_check( e->modelBox, o->modelBox ))
            {
                gf3d_entity_simple_collision(e, o);
                slog("collision");
            }

        }
    }
    gf3d_timer_start(&timer);
}

void gf3d_entity_manager_draw(Uint32 bufferFrame, VkCommandBuffer commandBuffer, float frame)
{
    Entity *e;
    int i;
    for (i = 0; i < gf3d_entity_manager.entity_max; i++)
    {
        e = &gf3d_entity_manager.entity_list[i];
        if(!e || !e->_inuse) continue;
        if (e->animationManager) gf3d_animation_draw(e->animationManager, bufferFrame, commandBuffer, e->modelMat, frame);
        else if (e->model) gf3d_model_draw(e->model, bufferFrame, commandBuffer, e->modelMat, 0);
    }
}

void gf3d_entity_manager_draw_collision_boxes( Uint32 bufferFrame, VkCommandBuffer commandBuffer )
{
    Entity *ent;
    int i;
    for(i = 0; i < gf3d_entity_manager.entity_max; i++)
    {
        ent = &gf3d_entity_manager.entity_list[i];
        gf3d_collision_armor_draw(ent->modelBox, bufferFrame, commandBuffer);
        gf3d_collision_armor_draw(ent->hurtboxes, bufferFrame, commandBuffer);
        gf3d_collision_armor_draw(ent->hitboxes, bufferFrame, commandBuffer);
    }
}

void gf3d_entity_general_update( Entity *self )
{
    Vector3D buff = vector3d(0,0,0);
    Vector3D buff2 = vector3d(0, 0, 0);
    Vector2D planeVel;
    float distanceToFloor = 0.0f;
    Uint8 onFloor = 0;

    /* handle gravity and stuff */
    if(self->modelBox)
    {
        distanceToFloor = distance_to_floor( self->modelBox->shapes[0].position.z - self->modelBox->shapes[0].extents.z );
        onFloor = on_floor(distanceToFloor);
        if (onFloor && !(self->state & ES_Jumping))
        {
            self->acceleration.z = 0;
            self->velocity.z = 0;
        }
        else if (distanceToFloor > 0.7)
        {
            if (self->velocity.z < 0.05)
            {
                self->state &= ~ES_Jumping;
            }
            self->acceleration.z = -GRAVITY;
        }   
    } 

    planeVel = vector2d(self->velocity.x, self->velocity.y);
    if( vector2d_magnitude_squared(planeVel) >= 2.0f )
    {
        vector2d_negate(planeVel, planeVel);
        vector2d_set_magnitude(&planeVel, DAMP_SPEED);
        self->velocity.x += planeVel.x;
        self->velocity.y += planeVel.y;
    }
    else
    {
        self->velocity.x = self->velocity.y = 0.0f;
    }
    
    /* Cap speed */
    if ( abs(self->velocity.x) > MAX_SPEED )
    {
        if (self->velocity.x > 0) self->velocity.x = MAX_SPEED;
        else self->velocity.x = -MAX_SPEED;
    }
    if ( abs(self->velocity.y) > MAX_SPEED )
    {
        if (self->velocity.y > 0) self->velocity.y = MAX_SPEED;
        else self->velocity.y = -MAX_SPEED;
    }

    /* vf = vi + a*t */
    vector3d_scale(buff, self->acceleration, deltaTime);
    vector3d_add(self->velocity, self->velocity, buff);

    /* df = di + v*t + at^2/2*/
    vector3d_scale(buff, self->velocity, deltaTime);
    vector3d_add(buff, self->position, buff);
    vector3d_scale(buff2, self->acceleration, deltaTime * deltaTime / 2);
    vector3d_add(buff, buff2, buff);
    if ( within_stage(buff) )
        vector3d_copy(self->position, buff);

    /* update collision boxes */
    gf3d_collision_armor_update(self->hitboxes, self->position, self->rotation);
    gf3d_collision_armor_update(self->hurtboxes, self->position, self->rotation);
    gf3d_collision_armor_update(self->modelBox, self->position, self->rotation);
    
    /* keep matrix and model updated with transform, scale and rotation */
    vector3d_add(buff, self->position, self->modelOffset);
    gfc_matrix_make_translation(self->modelMat, buff);
    gf3d_model_scale( self->modelMat, self->scale);
    gfc_matrix_rotate(self->modelMat, self->modelMat, (self->rotation.x + 90) * GFC_DEGTORAD, vector3d(0, 0, 1));
}

// void gf3d_entity_general_touch( Entity *self, Entity *other )
void gf3d_entity_simple_collision( Entity *self, Entity *other )
{
    float smag, omag; /* to store velocity magnitude squared of self and other */
    Vector3D dir, buff;
    Entity *p, *op;

    // slog("got to simple collision");
    if ( vector3d_equal( self->position, other->position) )
    {
        other->position.x += 1;
    }

    /* find who needs to push who depending on the speed */
    smag = vector3d_magnitude_squared(self->velocity);
    omag = vector3d_magnitude_squared(other->velocity); 
    if ( omag > smag)
    {
        vector3d_copy(dir, other->velocity);
        p = other;
        op = self;
    }
    else if ( omag < smag )
    {
        vector3d_copy(dir, self->velocity);
        p = self;
        op = other;
    }
    else
    {
        vector3d_sub(dir, other->position, self->position);
        p = other;
        op = self;
    }

    /* elastic collision */
    vector3d_normalize(&dir);
    vector3d_set_magnitude(&dir, MAX_SPEED);
    vector3d_copy(op->velocity, dir);
    vector3d_negate(p->velocity, dir);
    
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
        ent->mass = -1.0f;
        ent->state = ES_Idle;
        
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
    self->hurtboxes = NULL;
    if(self->modelBox) gf3d_collision_armor_free(self->modelBox);
    self->modelBox = NULL;
    // if(self->animationManager) gf3d_animation_manager_free(self->animationManager);
    // self->animationManager = NULL;
}