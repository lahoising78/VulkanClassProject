#ifndef _APP_PLAYER_H_
#define _APP_PLAYER_H_

#include "simple_logger.h"
#include "app_player.h"

typedef struct
{
    Player *player_list;
    Uint32 player_max;
} PlayerManager;

static PlayerManager app_player_manager = {0}; /* this is only available on this file */

void app_player_manager_close()
{
    if(app_player_manager.player_list != NULL)
    {
        free(app_player_manager.player_list);
    }
    memset(&app_player_manager, 0, sizeof(PlayerManager));
}

void app_player_manager_init( Uint32 player_max )
{
    app_player_manager.player_list = (Player*)gfc_allocate_array(sizeof(Player), player_max);
    if(!app_player_manager.player_list)
    {
        slog("failed to allocate entity list");
        return;
    }
    app_player_manager.player_max = player_max;
    slog("manager init");
    atexit(app_player_manager_close);
}

void app_player_manager_update( SDL_Event *events )
{
    int i;
    Player *p = NULL;

    for (i = 0; i < app_player_manager.player_max; i++)
    {
        p = &app_player_manager.player_list[i];
        if (p->_inuse)
        {
            p->input_handler(p, events );
        }
    }
}

Player *app_player_new()
{
    Player *ent = NULL;
    int i;
    for(i = 0; i <app_player_manager.player_max; i++)
    {
        if (app_player_manager.player_list[i]._inuse) continue;

        memset(&app_player_manager.player_list[i], 0, sizeof(Player));
        app_player_manager.player_list[i]._inuse = 1;
        slog("created player");
        return &app_player_manager.player_list[i];
    }
    slog("request for entity failed: all full up");
    return NULL;
}

void    app_player_free(Player *self)
{
    if (!self)
    {
        slog("self pointer is not valid");
        return;
    }

    self->_inuse = 0;

    if (self->entity->data != NULL)
    {
        slog("warning: data not freed at entity free");
    }
}

#endif