#ifndef _GF3D_ENTITY_H_
#define _GF3D_ENTITY_H_

#include "gf3d_model.h"
#include "gf3d_collision.h"

/* Used for update and stuff */
#define MAX_ACCELERATION 100.0f
#define START_ACCELERATION 50.0f
#define DAMP_ACCELERATION 50.0f
#define MAX_SPEED 100.0f
#define START_SPEED 50.0f
#define DAMP_SPEED 0.05f

typedef enum
{
    ES_Idle = 0,
    ES_Dying = 1,
    ES_Dead = 2
} EntityState;

typedef struct Entity_S
{
    Model *model;
    Matrix4 modelMat;
    Vector3D modelOffset; /* offset the model relative to position */

    CollisionArmor *hurtboxes; /* A pointer to one collision armor, which contains several collision boxes */

    Vector3D position;
    Vector3D velocity;
    Vector3D acceleration;
    Vector3D rotation;
    Vector3D scale;
    
    EntityState state;
    Uint8 _inuse;
    void (*think) (struct Entity_S* self);
    void (*update)(struct Entity_S* self);
    void (*touch) (struct Entity_S* self, struct Entity_S* other);
    
    float health;
    float healthmax;
    float armor;
    float experience;
    float level;
    float otherStuff;
    
    void *data;
} Entity;

/**
 * @brief initializes the entity subsystem
 * @param entity_max maximum number of simultaneous entities you wish to support
 */
void gf3d_entity_manager_init(Uint32 entity_max);

/* 
 * @brief calls update on all entities
 */
void gf3d_entity_manager_update(  );

void gf3d_entity_manager_draw_hurtboxes(Uint32 bufferFrame, VkCommandBuffer commandBuffer);

/* 
 * @brief an update function to be called by all entities at the end of their individual updates
 * @param self the entity to update
 */
void gf3d_entity_general_update( Entity *self );

void gf3d_entity_general_touch( Entity *self, Entity *other );

/**
 * @brief get an empty entity from the system
 * @return NULL on out of space or a pointer to an entity otherwise
 */
Entity *gf3d_entity_new();

/**
 * @brief free an active entity
 * @param self the entity to free
 */
void    gf3d_entity_free(Entity *self);

/* 
 * @brief allocate space for hurt boxes 
 * @param ent the entity to add hurtboxes to
 * @param count the number of hurtboxes to allocate
 */
void gf3d_entity_add_hurtboxes( Entity *ent, Uint32 count);

#endif