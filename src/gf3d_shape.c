#include "gf3d_shape.h"

Shape gf3d_shape( Vector3D position, Vector3D extents, Model *model )
{
    Shape s;

    s.position = position;
    s.extents = extents;
    s.model = model;

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
    return 0;
}

void gf3d_shape_get_mesh(Shape *shape)
{
    Vertex vertices[8];
    Face faces[6];
    int i;

    vertices[0].vertex = vector3d( 1.0f,  1.0f,  1.0f);
    vertices[0].normal = vector3d( 0.0f, -1.0f,  0.0f);

    vertices[1].vertex = vector3d( 1.0f,  1.0f, -1.0f);
    vertices[1].normal = vector3d( 0.0f, -1.0f,  0.0f);

    vertices[2].vertex = vector3d(-1.0f,  1.0f,  1.0f);
    vertices[2].normal = vector3d( 0.0f, -1.0f,  0.0f);

    vertices[3].vertex = vector3d(-1.0f,  1.0f, -1.0f);
    vertices[3].normal = vector3d( 0.0f, -1.0f,  0.0f);

    vertices[4].vertex = vector3d( 1.0f,  1.0f,  1.0f);
    vertices[4].normal = vector3d( 0.0f, -1.0f,  0.0f);

    vertices[5].vertex = vector3d( 1.0f,  1.0f,  1.0f);
    vertices[5].normal = vector3d( 0.0f, -1.0f,  0.0f);

    vertices[6].vertex = vector3d( 1.0f,  1.0f,  1.0f);
    vertices[6].normal = vector3d( 0.0f, -1.0f,  0.0f);

    vertices[7].vertex = vector3d( 1.0f,  1.0f,  1.0f);
    vertices[7].normal = vector3d( 0.0f, -1.0f,  0.0f);

    for (i = 0; i < 8; i++)
    {
        vertices[i].texel = vector2d(0, 0);
    }
    
}

void gf3d_shape_update_mat( Shape *shape )
{
    gfc_matrix_make_translation(shape->matrix, shape->position);
    gf3d_model_scale(shape->matrix, shape->extents);
}

void gf3d_shape_draw( struct Shape_S shape )
{
    
}