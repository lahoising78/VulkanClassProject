#ifndef _APP_GAARA_H_
#define _APP_GAARA_H_

#include "app_player.h"

#define SAND_FORWARD    8.0f
#define SAND_LEFT       15.0f
#define SAND_SPEED      120.0f

/* 
 * @brief create a new gaara as an entity
 * @return a pointer to the entity created
 */
Entity *app_gaara_new();

void app_gaara_input_handler(Player *self, SDL_Event *events);

void app_gaara_think(Entity *self);
void app_gaara_update(Entity *self);
void app_gaara_touch(Entity *self, Entity *other);

#endif