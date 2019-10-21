#ifndef _GF3D_COLLISION_H_
#define _GF3D_COLLISION_H_

#include "gf3d_shape.h"

typedef struct CollisionArmor_S
{

    Shape *shapes;
    Uint8 *_inuse;
    Uint32 shapeCount;
    Vector3D *offsets;

} CollisionArmor;

CollisionArmor *gf3d_collision_armor_new( Uint32 count );

void gf3d_collision_armor_free( CollisionArmor *armor );

int gf3d_collision_armor_add_shape( CollisionArmor *armor, Shape s, Vector3D offset );

void gf3d_collision_armor_update( CollisionArmor *armor, Vector3D parentPosition, Vector3D parentRotation );

void gf3d_collision_armor_update_mat( CollisionArmor *armor );

void gf3d_collision_armor_draw( CollisionArmor *armor, Uint32 bufferFrame, VkCommandBuffer commandBuffer );

/* Actual Collision Stuff */
int gf3d_collision_check( CollisionArmor *first, CollisionArmor *second );

#endif