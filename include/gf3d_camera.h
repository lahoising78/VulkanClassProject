#ifndef __GF3D_CAMERA_H__
#define __GF3D_CAMERA_H__

#include "gfc_matrix.h"

/* 
 * @brief get the forward vector of the camera
 * @param forward dest of forward vector
 * @param right dest of right vector
 * @param up dest of up vector
 */
void gf3d_camera_get_angles(Vector3D *forward, Vector3D *right, Vector3D *up);

/**
 * @brief get the current camera view
 * @param view output, the matrix provided will be populated with the current camera information
 */
void gf3d_camera_get_view(Matrix4 view);

/**
 * @brief set the current camera based on the matrix provided
 */
void gf3d_camera_set_view(Matrix4 view);

/**
 * @brief set the camera properties based on position and direction that the camera should be looking
 * @param position the location for the camera
 * @param target the point the camera should be looking at
 * @param up the direction considered to be "up"
 */
void gf3d_camera_look_at(
    Vector3D position,
    Vector3D target,
    Vector3D up
);

/* 
 * @brief make camera look at the center between two points
 * @param first first position
 * @param second second position
 */
void gf3d_camera_look_at_center(
    Vector3D first,
    Vector3D second
);

/**
 * @brief explicitely set the camera positon, holding all other parameters the same
 * @param position the new position for the camera
 */
void gf3d_camera_set_position(Vector3D position);

/**
 * @brief move the camera relatively based on the vector provided
 * @param move the ammount to move the camera
 */
void gf3d_camera_move(Vector3D move);

#endif
