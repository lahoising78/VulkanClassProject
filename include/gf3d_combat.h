#ifndef _GF3D_COMBAT_H_
#define _GF3D_COMBAT_H_

#include "gf3d_entity.h"

#define PROJECTILE_OWNER        0
#define PROJECTILE_TARGET       1

#define PROJECTILE_MAX_SPEED    90.0f

/* 
 * @brief attack
 * @param attacker : the entity attacking
 * @param target : the entity receiving the damage
 * @param damage : damage
 * @param knockback : value to push target
 * @param dir : direction to push target
 */
void gf3d_combat_attack(Entity *attacker, Entity *target, float damage, float knockback, Vector3D dir);

/* 
 * @brief do meele attack
 * @param attacker : the entity attacking
 * @param target : the entity that is being attacked
 * @param damage : damage to apply
 * @param knockback : value to push target
 */
void gf3d_combat_meele_attack(Entity *attacker, Entity *target, float damage, float knockback);

/* 
 * @brief create a projectile
 * @param owner : the owner of the projectile
 * @param target : the target of the projectile
 */
Entity *gf3d_combat_projectile_new(Entity *owner, Entity *target);

#endif