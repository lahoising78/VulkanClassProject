#ifndef _GF3D_COMMON_H_
#define _GF3D_COMMON_H_

#include "gf3d_entity.h"

/* 
 * @brief create the chackra when charging
 * @param ent : the entity to attack chakra to
 */
Entity *gf3d_common_chakra_new(Entity *owner);

/* 
 * @brief free the chakra entity
 * @param ent : the entity to free
 */
void gf3d_common_chakra_free(Entity *self);

/* 
 * @brief update the chakra
 * @param self : chakra to update
 */
void gf3d_common_chakra_update(Entity *self);

#endif