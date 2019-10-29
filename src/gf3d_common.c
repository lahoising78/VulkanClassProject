#include "gf3d_common.h"
#include "simple_logger.h"

// extern float worldTime;

void gf3d_common_init_state(Entity *ent)
{
    if(!ent || !ent->animationManager) return;
    ent->state &= ~ES_Attacking;
    ent->state |= ES_Idle;
    gf3d_animation_play(ent->animationManager, "idle", 1);
    ent->locked = 0;
}

Entity *gf3d_common_chakra_new(Entity *owner)
{
    Entity *ent = NULL;

    ent = gf3d_entity_new();
    if(!ent)
    {
        return NULL;
    }

    ent->update = gf3d_common_chakra_update;
    ent->data = owner;
    slog("creating chakra");

    return ent;
}

void gf3d_common_chakra_free(Entity *ent)
{
    if(!ent) return;
    slog("chakra free");
    ent->data = NULL;
    gf3d_entity_free(ent);
}

void gf3d_common_chakra_update(Entity *ent)
{
    Entity *owner = NULL;

    if(!ent) return;

    owner = (Entity*)ent->data;
    if(!owner) return;

    if( owner->animationManager && !gf3d_animation_is_playing(owner->animationManager, "charge") )
    {
        gf3d_common_chakra_free(ent);
    }
    else
    {
        owner->chakra += worldTime * 10;
        if(owner->chakra > owner->chakraMax) owner->chakra = owner->chakraMax;
    }
    
}