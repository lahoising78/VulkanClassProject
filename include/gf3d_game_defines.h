#ifndef _GF3D_GAME_DEFINES_H_
#define _GF3D_GAME_DEFINES_H_

/* World Constants */
#define GRAVITY 230.0f

/* For Entity - used for update and stuff */
#define MAX_ACCELERATION 100.0f
#define START_ACCELERATION 50.0f
#define DAMP_ACCELERATION 1.0f
#define MAX_SPEED 70.0f
#define START_SPEED 10.0f
#define DAMP_SPEED 2.3f

#define COLLISION_SPEED 70.0f

/* For Stage */
#define MAX_STAGE_X 100.0f
#define MAX_STAGE_Y 100.0f
#define MAX_STAGE_Z -10.f
#define STAGE_SCALE_Z 1

/* Slog vector */
#define vector3d_slog(vec) (slog("x: %.3f, y: %.3f, z: %.3f", vec.x, vec.y, vec.z))

#define distance_to_floor(feetHeight) (feetHeight - (MAX_STAGE_Z + STAGE_SCALE_Z))
#define on_floor(distanceToFloor) (distanceToFloor < 0.5f)

#define within_stage(a) (a.x >= -MAX_STAGE_X && a.x <= MAX_STAGE_X && a.y >= -MAX_STAGE_Y && a.y <= MAX_STAGE_Y)

#endif