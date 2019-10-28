#ifndef _GF3D_COLLISION_H_
#define _GF3D_COLLISION_H_

#include "gf3d_shape.h"

typedef struct CollisionArmor_S
{

    Shape *shapes;
    Uint8 *_inuse;
    Uint32 shapeCount;
    Vector3D *offsets;
    char **shapeNames;

} CollisionArmor;

/* 
 * @brief create a new collision armor
 * @param count : the max amount of shapes the armor can contain
 */
CollisionArmor *gf3d_collision_armor_new( Uint32 count );

/* 
 * @brief free the armor
 * @param armor : the armor to free
 */
void gf3d_collision_armor_free( CollisionArmor *armor );

/* 
 * @brief add a new shape to the armor
 * @param armor : the armor to add the shape to
 * @param shape : the shape to add
 * @param offset : the offset of the shape according to the origin of the entity that holds the armor
 * @param name : the name of the shape, to refer to it later
 */
int gf3d_collision_armor_add_shape( CollisionArmor *armor, Shape s, Vector3D offset, char *name );

/*  
 * @brief remove a shape from the armor
 * @param armor : the armor to remove shape from
 * @param name : the name of the shape to remove
 */
int gf3d_collision_armor_remove_shape( CollisionArmor *armor, char *name );

/* 
 * @brief remove all shapes from armor
 * @param armor : armor to wipe
 */
int gf3d_collision_armor_remove_all( CollisionArmor *armor );

/* 
 * @brief update the position of the shapes in the armor
 * @param armor : the armor to update
 * @param parentPosition : the origin to calculate offsets from, usually the the origin of the entity that holds the armor
 * @param parentRotation : the rotation of the entity that holds the armor; serves to define what forward is
 */
void gf3d_collision_armor_update( CollisionArmor *armor, Vector3D parentPosition, Vector3D parentRotation );

/* 
 * @brief update the matrix of the armor's shapes
 * @note not necessary to call if you display for shapes is unimportant
 * @param armor the armor that contains the shapes
 */
void gf3d_collision_armor_update_mat( CollisionArmor *armor );

/* 
 * @brief draw the shapes of the collision armor
 * @param armor : the armor to draw
 * @param bufferFrame : the bufferFrame
 * @param commandBuffer : the command buffer
 */
void gf3d_collision_armor_draw( CollisionArmor *armor, Uint32 bufferFrame, VkCommandBuffer commandBuffer );

/* 
 * @brief checks if specified shape is in armor
 * @param armor : armor to check
 * @param name : name of the shape
 */
int gf3d_collision_armor_contains(CollisionArmor *armor, char* name);

/* Actual Collision Stuff */
/* 
 * @brief check for collision between two armors
 * @param first : the first armor
 * @param second : the second armor
 */
int gf3d_collision_check( CollisionArmor *first, CollisionArmor *second );

#endif