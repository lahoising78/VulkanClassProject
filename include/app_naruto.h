#ifndef _APP_NARUTO_H_
#define _APP_NARUTO_H_

#include "gf3d_entity.h"
#include "app_player.h"

/* Animation defines */
#define ANIM_NARUTO_IDLE_START      1
#define ANIM_NARUTO_IDLE_END        67
#define ANIM_NARUTO_RUN_START       1
#define ANIM_NARUTO_RUN_END         20
#define ANIM_NARUTO_JUMP_UP_START   1
#define ANIM_NARUTO_JUMP_UP_END     25
#define ANIM_NARUTO_JUMP_DOWN_START 1
#define ANIM_NARUTO_JUMP_DOWN_END   12
#define ANIM_NARUTO_PUNCH_START     1
#define ANIM_NARUTO_PUNCH_END       27
#define ANIM_NARUTO_KICK_START      1
#define ANIM_NARUTO_KICK_END        49
#define ANIM_NARUTO_THROW_START     1
#define ANIM_NARUTO_THROW_END       51
#define ANIM_NARUTO_CHARGE_START    1
#define ANIM_NARUTO_CHARGE_END      18 

/* attack frames */
#define ATK_FRAME_NARUTO_PUNCH      17.5f
#define ATK_FRAME_NARUTO_KICK       30.0f

/* 
@brief get an entity with naruto's moves, model, functions
*/
Entity *app_naruto_new();

void app_naruto_input_handler( struct Player_s *self, SDL_Event* events );

void app_naruto_think (struct Entity_S* self);
void app_naruto_update(struct Entity_S* self);
void app_naruto_touch (struct Entity_S* self, struct Entity_S* other);

#endif