#ifndef _APP_GAARA_H_
#define _APP_GAARA_H_

#include "app_player.h"

/* 
 * @brief create a new gaara as an entity
 * @return a pointer to the entity created
 */
Entity *app_gaara_new();

void app_gaara_input_handler(Player *self, SDL_Event *events);

void app_gaara_think(Entity *self);
void app_gaara_update(Entity *self);
void app_gaara_touch(Entity *self);

#endif