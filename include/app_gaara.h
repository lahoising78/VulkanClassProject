#ifndef _APP_GAARA_H_
#define _APP_GAARA_H_

#include "app_player.h"

// /* sand values */
// #define SAND_FORWARD_1              6.0f
// #define SAND_FORWARD_2              8.0f
// #define SAND_LEFT                   15.0f
#define SA_SPEED                  150.0f

/* Sand Attack Swipe */
#define GAARA_SA_SWIPE_DMG          5
#define GAARA_SA_SWIPE_KICK         50.0f
#define GAARA_SA_SWIPE_HITSTUN      0.3f
#define GAARA_SA_SWIPE_FWD1_OFFSET  6.0f
#define GAARA_SA_SWIPE_FWD2_OFFSET  8.0f
#define GAARA_SA_SWIPE_LEFT_OFFSET  15.0f

/* Sand Attack Forward */
#define GAARA_SA_FWD_DMG            5
#define GAARA_SA_FWD_KICK           300.0f
#define GAARA_SA_FWD_HITSTUN        0.7f

// /* Sand Attack */
// #define DMG_GAARA_SAND_ATK          5
// #define KICK_GAARA_SAND_ATK         5.5f
// #define KICK_GAARA_SAND_FWD_ATK     30.0f

/* Sand Burial */
#define GAARA_SB_FWD                4.0f
#define GAARA_SB_EXTENT_FWD         60.0f
#define GAARA_SB_EXTENT_UP          20.0f
#define GAARA_SB_FWD_SPEED          100.0f
#define GAARA_SB_UP_SPEED           30.0f
#define GAARA_SB_DMG                30.0f
#define GAARA_SB_HITSTUN            0.4f
#define GAARA_SB_END_FRAME          75.0f
#define GAARA_SB_TOUCH_SCALE        5
#define GAARA_SB_DATA_OWNER         0
#define GAARA_SB_DATA_TOUCHED       1

/* Sand Storm */
#define GAARA_SS_NUM                4
#define GAARA_SS_SCALE              0.5f
#define GAARA_SS_DISTANCE_TO_ENT    3.0f
#define GAARA_SS_TIME               4.0f
#define GAARA_SS_SPEED              60.0f
#define GAARA_SS_DMG                10.0f
#define GAARA_SS_KICK               50.0f
#define GAARA_SS_HITSTUN            0.1f

/* Knock Up */
#define GAARA_KU_DMG                10
#define GAARA_KU_KICK               100.0f
#define GAARA_KU_HITSTUN            0.5f
#define GAARA_KU_SPEED              100.0f /* knock up speed */
#define GAARA_KU_MAX_Z              100.0f

/* Sand Tsunami */
#define GAARA_ST_NUM                3
#define GAARA_ST_UP_OFFSET          5.0f
#define GAARA_ST_FWD_OFFSET         15.0f
#define GAARA_ST_FWD_OFFSET_INC     10
#define GAARA_ST_SPEED              80.0f
#define GAARA_ST_DMG                30.0f
#define GAARA_ST_KICK               10.0f
#define GAARA_ST_HITSTUN            0.1f

// extern float worldTime;

enum GaaraSandAttackType
{
    ATK_RIGHT = 0,
    ATK_LEFT = 1,
    ATK_FORWARD = 2
};

/* *********************
 * ABILITIES
 *  o Sand Burial
 *  -Sand Protection 
 *  o Sand Storm
 *  o Knock Up
 *  o Sand Tsunami
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