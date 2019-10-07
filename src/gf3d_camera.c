#include "gfc_matrix.h"
#include "gf3d_vgraphics.h"

#include <string.h>

Matrix4 gf3d_camera = {0};

void gf3d_camera_get_view(Matrix4 *view)
{
    if (!view)return;
    memcpy(view,gf3d_camera,sizeof(Matrix4));
}

void gf3d_camera_set_view(Matrix4 *view)
{
    if (!view)return;
    memcpy(gf3d_camera,view,sizeof(Matrix4));
}

void gf3d_camera_look_at(
    Vector3D position,
    Vector3D target,
    Vector3D up
)
{
    gfc_matrix_view(
        gf3d_camera,
        position,
        target,
        up
    );
}

void gf3d_camera_look_at_center(
    Vector3D first,
    Vector3D second
)
{
    Vector3D middle, camera_position, normal;
    const float normalScale = 1.0f;
    int mag_sqrd;

    /* find center between positions */
    vector3d_sub(middle, first, second);
    vector3d_cross_product(&normal, middle, vector3d(0, 0, 1));
    vector3d_scale(middle, middle, 0.5);
    vector3d_add(middle, middle, second);

    /* find proper camera position */
    vector3d_scale(camera_position, normal, normalScale);
    vector3d_add(camera_position, camera_position, middle);

    /* Making sure that we don't get too close to the entities */
    mag_sqrd = vector3d_magnitude_squared(normal);
    if (mag_sqrd == 0) 
    {
        return;
    }
    
    if ( mag_sqrd < 400.0f )
    {
        vector3d_normalize(&normal);
        vector3d_scale(normal, normal, normalScale * 20);
        vector3d_add(camera_position, middle, normal);
    }
    
    gf3d_camera_look_at(camera_position, middle, vector3d(0, 0, 1));
    gf3d_vgraphics_set_camera_view( gf3d_camera );
}

void gf3d_camera_set_position(Vector3D position)
{
    gf3d_camera[0][3] = position.x;
    gf3d_camera[1][3] = position.y;
    gf3d_camera[2][3] = position.z;
}

void gf3d_camera_move(Vector3D move)
{
    gf3d_camera[0][3] += move.x;
    gf3d_camera[1][3] += move.y;
    gf3d_camera[2][3] += move.z;
}

/*eol@eof*/
