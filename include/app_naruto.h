#ifndef _APP_NARUTO_H_
#define _APP_NARUTO_H_

#include "gf3d_entity.h"
#include "app_player.h"

/* 
@brief get an entity with naruto's moves, model, functions
*/
Entity *app_naruto_new();

void app_naruto_input_handler( struct Player_s *self, const Uint8* keys );

void app_naruto_think (struct Entity_S* self);
void app_naruto_update(struct Entity_S* self);
void app_naruto_touch (struct Entity_S* self, struct Entity_S* other);

#endif