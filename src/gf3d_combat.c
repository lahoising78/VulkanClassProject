#include "gf3d_combat.h"

void gf3d_combat_meele_attack(Entity *attacker, Entity *target, float damage, float kick)
{
    Vector3D forward;
    // Vector3D buff;

    if(!attacker || !target) return;

    /* add kick */
    vector3d_angle_vectors(attacker->rotation, &forward, NULL, NULL);
    vector3d_scale(forward, forward, kick);
    vector3d_add(target->velocity, target->velocity, forward);

    /* do damage */
    target->health -= damage;

    /* make target look at attacker */
    target->rotation.x = 180 - attacker->rotation.x;


}