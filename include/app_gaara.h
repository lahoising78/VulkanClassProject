#ifndef _APP_GAARA_H_
#define _APP_GAARA_H_

#include "app_player.h"

/* sand values */
#define SAND_FORWARD                6.0f
#define SAND_LEFT                   15.0f
#define SAND_SPEED                  150.0f

/* attack damaage */
#define DMG_GAARA_SAND_ATK          5

/* attack kick */
#define KICK_GAARA_SAND_ATK         3.0f
#define KICK_GAARA_SAND_FWD_ATK     30.0f

enum GaaraSandAttackType
{
    ATK_RIGHT = 0,
    ATK_LEFT = 1,
    ATK_FORWARD = 2
};

/* *********************
 * ABILITIES
 *  -Sand Burial
 *  -Sand Protection 
 *  -Sand Storm
 *  -Knock Up
 *  -Sand Tsunami
 * ***********************/

/* 
 * @brief create a new gaara as an entity
 * @return a pointer to the entity created
 */
Entity *app_gaara_new();

void app_gaara_input_handler(Player *self, SDL_Event *events);

void app_gaara_think(Entity *self);
void app_gaara_update(Entity *self);

/* 
 * @brief what to do when gaara attacks something
 * @note all of gaara's attack are going to be through saand
 * @param self : the sand 
 * @param other : the entity that made contact
 */
void app_gaara_touch(Entity *self, Entity *other);

#endif