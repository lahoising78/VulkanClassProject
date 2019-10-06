#ifndef _GF3D_SHAPE_H_
#define _GF3D_SHAPE_H_

#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gf3d_model.h"

typedef struct Shape_S
{
    Vector3D position;
    Vector3D extents;

    Model *model; /* to draw if needed */
    Matrix4 matrix; /* to draw if needed */
} Shape;

/* 
 * @brief get new shape with specified attributes
 * @param position position of the desired shaped
 * @param extents extents of the desired shape
 * @param mesh mesh of the desired shape
 * @return shape with the specified values
 */
Shape gf3d_shape( Vector3D position, Vector3D extents, Model *model );
// Shape gf3d_shape( Vector3D position, Vector3D extents, Mesh *mesh );

/* 
 * @brief get a new shape
 * @return empty shape
 */
Shape gf3d_shape_new();

/* 
 * @brief check if two shapes are intersecting
 * @param a first shape
 * @param b second shape
 */
int gf3d_shape_intersect(Shape a, Shape b);

/* 
 * @brief update the matrix of the shape
 * @param shape shape to update matrix of
 */
void gf3d_shape_update_mat( Shape *shape );

/* 
 * @brief draw render
 * @param shape the shape to render
 */
void gf3d_shape_draw( struct Shape_S shape );

#endif