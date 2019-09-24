#ifndef _GF3D_ENTITY_H_
#define _GF3D_ENTITY_H_

#include "gf3d_model.h"

typedef enum
{
    ES_Idle = 0,
    ES_Dying = 1,
    ES_Dead = 2
} EntityState;

typedef struct Entity_S
{
    Model *model;
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

#endif