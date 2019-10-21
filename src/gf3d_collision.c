#include "gf3d_collision.h"
#include "simple_logger.h"

CollisionArmor *gf3d_collision_armor_new( Uint32 count )
{
    CollisionArmor *armor = NULL;
    
    slog("creating armor");

    if (!count) 
    {
        slog("Why allocated 0 shapes for armor?");
        return NULL;
    }

    armor = (CollisionArmor*)malloc(sizeof(CollisionArmor));
    if (!armor)
    {
        slog("Not enough space to allocate collision armor");
    }

    armor->shapes = (Shape*)gfc_allocate_array( sizeof(Shape), count );
    if (!armor->shapes) 
    {
        slog("Could not allocate %d collision shapes", count);
        return NULL;
    }

    armor->offsets = (Vector3D*)gfc_allocate_array(sizeof(Vector3D), count);
    if (!armor->offsets) 
    {
        slog("Could not allocate %d collision offsets", count);
        return NULL;
    }

    armor->_inuse = (Uint8*)gfc_allocate_array(sizeof(Vector3D), count);
    if (!armor->_inuse) 
    {
        slog("Could not allocate %d collision _inuse", count);
        return NULL;
    }

    armor->shapeCount = count;
    return armor;
}

void gf3d_collision_armor_free( CollisionArmor *armor )
{
    free(armor->shapes);
    free(armor->offsets);
    free(armor->_inuse);
    free(armor);
}

int gf3d_collision_armor_add_shape( CollisionArmor *armor, Shape s, Vector3D offset )
{
    int i;

    if (!armor)
    {
        slog("Can't add shapes to nothing -.-");
    }

    for(i = 0; i < armor->shapeCount; i++)
    {
        if (armor->_inuse[i]) continue;
        armor->shapes[i] = s;
        armor->_inuse[i] = 1;
        armor->offsets[i] = offset;
        return 1;
    }
    return 0;
}

void gf3d_collision_armor_update( CollisionArmor *armor, Vector3D parentPosition, Vector3D parentRotation )
{
    Shape *s;
    Vector3D forward, right, offset;
    int i;

    if (!armor || !armor->shapes) return;

    vector3d_angle_vectors(parentRotation, &forward, &right, NULL);

    for (i = 0; i < armor->shapeCount; i++)
    {
        s = &armor->shapes[i];
        vector3d_scale(forward, forward, -armor->offsets[i].y);
        vector3d_scale(right, right, -armor->offsets[i].x);
        vector3d_add( s->position , parentPosition, forward );
        vector3d_add(s->position, s->position, right);
        s->position.z += armor->offsets[i].z;
    }
}

void gf3d_collision_armor_update_mat( CollisionArmor *armor )
{
    Shape *s;
    int i;

    if( !armor || !armor->shapes) return;

    for (i = 0; i < armor->shapeCount; i++)
    {
        s = &armor->shapes[i];
        gf3d_shape_update_mat(s);
    }
}

void gf3d_collision_armor_draw( CollisionArmor *armor, Uint32 bufferFrame, VkCommandBuffer commandBuffer )
{
    Shape *shape;
    Vector3D offset;
    int i;

    if ( !armor || !armor->shapes) return;

    for (i = 0; i < armor->shapeCount; i++)
    {
        // gf3d_shape_update_mat( &armor->shapes[i] );
        shape = &armor->shapes[i];
        vector3d_add(offset, shape->position, armor->offsets[i]);
        gfc_matrix_make_translation(shape->matrix, offset);
        gf3d_model_scale(shape->matrix, shape->extents);
        gf3d_model_draw(shape->model, bufferFrame, commandBuffer, shape->matrix, 0);
    }
}

int gf3d_collision_check( CollisionArmor *first, CollisionArmor *second )
{
    Shape f, s;
    int i, j;

    for(i = 0; i < first->shapeCount; i++)
    {
        if (!first->_inuse[i]) continue;
        f = first->shapes[i];
        
        for(j = 0; j < second->shapeCount; j++)
        {
            if (!second->_inuse[j]) continue;
            s = second->shapes[j];

            if ( gf3d_shape_intersect( f, s ) )
                return 1;
        }
    }

    return 0;
}