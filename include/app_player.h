#ifndef _APP_ENTITY_H_
#define _APP_ENTITY_H_

#include "gf3d_entity.h"

typedef struct Player_s
{
    Entity *entity;
    int id;
    int _inuse;
    void (*input_handler)( struct Player_s *self, SDL_Event *events);
} Player;

void app_player_manager_init( Uint32 player_max );

void app_player_manager_update( SDL_Event *events );

Player *app_player_new();

void    app_player_free(Player *self);

#endif