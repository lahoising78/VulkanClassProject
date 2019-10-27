#include "gfc_matrix.h"
#include "gf3d_vgraphics.h"

#include <string.h>

// extern float worldTime;
Matrix4 gf3d_camera = {0};
Vector3D gf3d_camera_forward = {0};
Vector3D gf3d_camera_right = {0};

void gf3d_camera_get_angles(Vector3D *forward, Vector3D *right, Vector3D *up)
{
    if (forward)
        *forward = vector3d(0, 1, 0);

    if (right)
        *right = vector3d(-1, 0, 0);

    if (up)
        *up = vector3d(0, 0, 1);
}

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
    Vector3D middle, camera_position;
    float offsetY = 0.0f;

    vector3d_sub(middle, first, second);

    /* find center between positions */
    vector3d_scale(middle, middle, 0.5f);
    vector3d_add(middle, middle, second);

    /* find proper camera position */
    offsetY = -abs(first.x - second.x);
    if (offsetY > -20.0f)
    {
        offsetY = -20.0f;
    }
    offsetY += (first.y < second.y)? first.y : second.y;
    
    camera_position.y = offsetY;
    camera_position.x = middle.x;
    camera_position.z = middle.z + 5.0f;

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
