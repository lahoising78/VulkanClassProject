#ifndef _GF3D_ENTITY_H_
#define _GF3D_ENTITY_H_

#include "gf3d_model.h"
#include "gf3d_collision.h"
#include "gf3d_game_defines.h"
#include "gf3d_animation.h"

extern float worldTime;

typedef enum
{
    ES_Idle =           1,
    ES_Dying =          2,
    ES_Dead =           4,
    ES_Jumping =        8,
    ES_Attacking =      16,
    ES_Walking_Out =    32
} EntityState;

typedef struct Entity_S
{
    Model *model;
    Matrix4 modelMat;
    Vector3D modelOffset; /* offset the model relative to position */
    AnimationManager *animationManager;

    CollisionArmor *hurtboxes; /* A pointer to one collision armor, which contains several collision boxes */
    CollisionArmor *hitboxes;
    CollisionArmor *modelBox;
    float mass;

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
    float chakra;
    float chakraMax;

    /* to flag if an entity is in the middle of an animation that shouldn't be interrupted 
       locked < 0 :             when being attacked 
       locked % 100 == 0 :   when using a jutsu*/
    Uint32 locked; 
    float hitstun;
    
    void *data;

    struct Entity_S *enemy;
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

/* 
 * @brief draw all entities
 * @param bufferFrame : the buffer frame
 * @param commandBuffer : the command buffer
 */
void gf3d_entity_manager_draw(Uint32 bufferFrame, VkCommandBuffer commandBuffer, float frame);

void gf3d_entity_manager_draw_collision_boxes(Uint32 bufferFrame, VkCommandBuffer commandBuffer);

/* 
 * @brief an update function to be called by all entities at the end of their individual updates
 * @param self the entity to update
 */
void gf3d_entity_general_update( Entity *self );

// void gf3d_entity_simple_collision( Entity *self, Entity *other );

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
 * @brief allocate space for collision boxes 
 * @param collisionArmor collision armor to save space for
 * @param count the number of collision boxes to allocate
 */
// void gf3d_entity_alloc_collision_boxes( CollisionArmor *collisionArmor, Uint32 count);

#endif