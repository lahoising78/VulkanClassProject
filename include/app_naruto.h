#ifndef _APP_NARUTO_H_
#define _APP_NARUTO_H_

#include "gf3d_entity.h"
#include "app_player.h"
#include "gf3d_combat.h"

/* Animation defines */
#define ANIM_NARUTO_IDLE_START              1
#define ANIM_NARUTO_IDLE_END                67
#define ANIM_NARUTO_RUN_START               1
#define ANIM_NARUTO_RUN_END                 20
#define ANIM_NARUTO_JUMP_UP_START           1
#define ANIM_NARUTO_JUMP_UP_END             25
#define ANIM_NARUTO_JUMP_DOWN_START         1
#define ANIM_NARUTO_JUMP_DOWN_END           12
#define ANIM_NARUTO_PUNCH_START             1
#define ANIM_NARUTO_PUNCH_END               27
#define ANIM_NARUTO_KICK_START              1
#define ANIM_NARUTO_KICK_END                49
#define ANIM_NARUTO_THROW_START             1
#define ANIM_NARUTO_THROW_END               51
#define ANIM_NARUTO_CHARGE_START            1
#define ANIM_NARUTO_CHARGE_END              18 
#define ANIM_NARUTO_RASENSHURIKEN_START     1
#define ANIM_NARUTO_RASENSHURIKEN_END       54

/* Naruto Punch */
#define NARUTO_PUNCH_DMG                    5
#define NARUTO_PUNCH_KICK                   3.0f
#define NARUTO_PUNCH_ATK_FRAME              17.5f

/* Naruto Kick */
#define NARUTO_KICK_ATK_FRAME               30.0f

/* Clone Punch */
#define NARUTO_CLONE_PUNCH_KICK             30.0f

/* Rasenshuriken */
#define NARUTO_RASENSHURIKEN_LOCKED         100
#define NARUTO_RASENSHURIKEN_ATK_FRAME      15.0f
#define NARUTO_RASENSHURIKEN_UP_OFFSET      5.0f
#define NARUTO_RASENSHURIKEN_TIME           1.5f
#define NARUTO_RASENSHURIKEN_ROT_SPEED      2000.0f
#define NARUTO_RASENSHURIKEN_SPEED          80.0f
#define NARUTO_RASENSHURIKEN_DMG            20.0f
#define NARUTO_RASENSHURIKEN_KICK           10.0f

/* Shuriken Teleport */
#define NARUTO_ST_LOCKED                    200
#define NARUTO_ST_SPEED                     80.0f
#define NARUTO_ST_TIME                      1.0f
#define NARUTO_ST_ROT_SPEED                 2000.0f
#define NARUTO_ST_UP_OFFSET                 5.0f
#define NARUTO_ST_OFFSET                    3.0f

/* ****************************** 
 * ABILITIES
 *  -Rasengan
 *  o Rasen-shuriken
 *  -Jinchuriki cloak
 *  -Shuriken Teleport
 *  -Barrage
 * ******************************/

extern float worldTime;

/* 
@brief get an entity with naruto's moves, model, functions
*/
Entity *app_naruto_new();

void app_naruto_input_handler( struct Player_s *self, SDL_Event* events );

void app_naruto_think (struct Entity_S* self);
void app_naruto_update(struct Entity_S* self);
void app_naruto_touch (struct Entity_S* self, struct Entity_S* other);

#endif