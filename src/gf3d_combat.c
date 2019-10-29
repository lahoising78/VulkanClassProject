#include "gf3d_combat.h"
#include "simple_logger.h"

// extern float worldTime;

void gf3d_projectile_update(Entity *self);
void gf3d_projectile_touch(Entity *self, Entity *other);

int  gf3d_combat_can_damage(Entity *self);

void gf3d_combat_attack(Entity *attacker, Entity *target, float damage, float knockback, Vector3D dir, float hitstun)
{
    Vector3D push;
    if(!attacker || !target) return;

    if(!gf3d_combat_can_damage(target)) return;
    target->hitstun = hitstun;

    /* knockback */
    vector3d_copy(push, dir);
    vector3d_set_magnitude(&push, knockback);
    vector3d_add(target->velocity, target->velocity, push);

    /* do damage */
    target->health -= damage;
    // target->locked = -1;
}

void gf3d_combat_meele_attack(Entity *attacker, Entity *target, float damage, float knockback, float hitstun)
{
    Vector3D forward;
    // Vector3D buff;

    if(!attacker || !target) return;
    // slog("combat hitstun: %f", hitstun);
    if(!gf3d_combat_can_damage(target)) return;
    target->hitstun = hitstun; 
    // if(target->locked < 0) return;

    /* add knockback */
    vector3d_angle_vectors(attacker->rotation, &forward, NULL, NULL);
    vector3d_scale(forward, forward, knockback);
    // vector3d_set_magnitude(&forward, knockback);
    vector3d_add(target->velocity, target->velocity, forward);

    /* do damage */
    target->health -= damage;
    // target->locked = -1;

    /* make target look at attacker */
    target->rotation.x = 180 + attacker->rotation.x;
    if(target->rotation.x > 180) target->rotation.x -= 360;
    else if (target->rotation.x < -180) target->rotation.x += 360;
}

/* *************************************
 * GENERAL STUFF
 * *************************************/
int gf3d_combat_can_damage(Entity *self)
{
    int canDamage = self->hitstun <= 0.0f;
    if(canDamage) slog("can damage: yes");
    return canDamage;
}

/* 
 ***************************************
            Projectile Stuff
 ***************************************
 */
Entity *gf3d_combat_projectile_new(Entity *owner, Entity *target)
{
    Entity *projectile = gf3d_entity_new();
    Entity **data = NULL;
    Vector3D forward;

    if(!projectile) 
    {
        slog("unable to create more projectiles");
        return NULL;
    }

    projectile->data = (Entity **)gfc_allocate_array(sizeof(Entity*), 2);
    if(!projectile->data)
    {
        slog("unable to allocate owner and target in projectile");
        gf3d_entity_free(projectile);
        return NULL;
    }

    data = (Entity**)projectile->data;
    data[ PROJECTILE_OWNER ] = owner;
    data[ PROJECTILE_TARGET ] = target;

    projectile->update = gf3d_projectile_update;
    projectile->touch = gf3d_projectile_touch;

    vector3d_angle_vectors(owner->rotation, &forward, NULL, NULL);
    vector3d_scale(forward, forward, 3);
    vector3d_add(projectile->position, forward, owner->position);

    return projectile;
}

void gf3d_projectile_update(Entity *self)
{
    Entity **data = NULL;
    Entity *target = NULL;

    Vector3D dir;
    if(!self) return;

    data = (Entity**)self->data;
    if(!data) 
    {
        return;
    }

    target = data[ PROJECTILE_TARGET ];
    if(!target) 
    {
        return;
    }
    
    vector3d_sub(dir, target->position, self->position);
    vector3d_set_magnitude(&dir, PROJECTILE_MAX_SPEED);
    vector3d_copy(self->velocity, dir);

    self->rotation.x = vector2d_angle( vector2d(target->position.x, target->position.y) );
    if(self->rotation.x > 180) self->rotation.x -= 360;
    else if (self->rotation.x < -180) self->rotation.x += 360;

    gf3d_entity_general_update(self);
}

void gf3d_projectile_touch(Entity *self, Entity *other)
{
    Entity *owner = NULL;
    Entity **data = NULL;
    if(!self || !other) return;
    data = (Entity**)self->data;
    if(!data) return;
    
    owner = data[PROJECTILE_OWNER];
    if(owner && owner == other) return;

    slog("Projectile touch");

    /* health on a projectile is damage to deal, and chakra is the knockback */
    gf3d_combat_meele_attack(self, data[PROJECTILE_TARGET], self->health, self->chakra, self->hitstun);
    free(self->data);
    self->data = NULL;
    gf3d_entity_free(self);
}