#include "gf3d_shape.h"
#include "simple_logger.h"

Shape gf3d_shape( Vector3D position, Vector3D extents, Model *model )
{
    Shape s;

    s.position = position;
    s.extents = extents;
    if (model == NULL)
    {
        s.model = gf3d_model_load("cube", NULL);
    }
    else
    {
        s.model = model;
    }

    return s;
}

Shape gf3d_shape_new()
{
    Shape s;
    s.position = vector3d(0, 0, 0);
    s.extents = vector3d(0, 0, 0);
    s.model = NULL;
    return s;
}

int gf3d_shape_intersect(Shape a, Shape b)
{
    Vector3D aMax, aMin;
    Vector3D bMax, bMin;

    aMax = vector3d( a.position.x + a.extents.x, a.position.y + a.extents.y, a.position.z + a.extents.z );
    aMin = vector3d( a.position.x - a.extents.x, a.position.y - a.extents.y, a.position.z - a.extents.z );
    
    bMax = vector3d( b.position.x + b.extents.x, b.position.y + b.extents.y, b.position.z + b.extents.z );
    bMin = vector3d( b.position.x - b.extents.x, b.position.y - b.extents.y, b.position.z - b.extents.z );

    return  ( aMin.x <= bMax.x && aMax.x >= bMin.x ) && 
            ( aMin.y <= bMax.y && aMax.y >= bMin.y ) && 
            ( aMin.z <= bMax.z && aMax.z >= bMin.z );
}

void gf3d_shape_update_mat( Shape *shape )
{
    gfc_matrix_make_translation(shape->matrix, shape->position);
    gf3d_model_scale(shape->matrix, shape->extents);
}

void gf3d_shape_draw( struct Shape_S *shape, Uint32 bufferFrame, VkCommandBuffer commandBuffer )
{
    gf3d_model_draw(shape->model, bufferFrame, commandBuffer, shape->matrix, 0);
}