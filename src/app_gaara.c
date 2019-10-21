#include "app_gaara.h"

#include "simple_logger.h"
#include "gf3d_camera.h"

void app_gaara_punch(Entity *self);
void app_gaara_throw_sand(Entity *self);

Entity *app_gaara_new()
{
    Entity *ent = gf3d_entity_new();
    if(ent == NULL) 
    {
        slog("could not create a new gaara. not enough memory");
        return NULL;
    }

    ent->health = 100;
    ent->healthmax = 100;
    ent->think = app_gaara_think;
    ent->touch = app_gaara_touch;
    ent->update = app_gaara_update;
    ent->model = gf3d_model_new();
    ent->model->texture = gf3d_texture_load("images/gaara.png");
    ent->animationManager = gf3d_animation_manager_init(8, ent->model);
    gf3d_animation_load(ent->animationManager, "idle", "gaara_idle", 1, 60);
    gf3d_animation_play(ent->animationManager, "idle", 1);
    gf3d_animation_load(ent->animationManager, "running", "gaara_running", 1, 20);
    gf3d_animation_load(ent->animationManager, "swipe right", "gaara_swipe_right", 1, 68);
    gf3d_animation_set_speed(ent->animationManager, "swipe right", 1.5f);
    gf3d_animation_load(ent->animationManager, "swipe left", "gaara_swipe_left", 1, 68);
    gf3d_animation_set_speed(ent->animationManager, "swipe left", 1.5f);
    gf3d_animation_load(ent->animationManager, "swipe forward", "gaara_forward_attack", 1, 50);
    gf3d_animation_set_speed(ent->animationManager, "swipe forward", 1.5f);
    gf3d_animation_load(ent->animationManager, "throw sand", "gaara_throw_sand", 1, 51);
    gf3d_animation_load(ent->animationManager, "jump", "gaara_jump", 1, 58);
    ent->modelOffset.z = -6.5f;
    ent->scale = vector3d(1.7f, 1.7f, 1.7f);
    gfc_matrix_identity(ent->modelMat);

    return ent;
}

void app_gaara_input_handler(Player *self, SDL_Event *events)
{
    Entity *e = self->entity;
    Vector3D camera_f, camera_r;
    Uint8 onFloor = 0;
    float distanceToFloor = 0.0f;
    int i;

    const int usedScancodes[] = {
        SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_A, 
        SDL_SCANCODE_SPACE,
        SDL_SCANCODE_J, SDL_SCANCODE_K
    };

    if (self->entity->modelBox)
    {
        distanceToFloor = distance_to_floor( e->modelBox->shapes[0].position.z - e->modelBox->shapes[0].extents.z );
        onFloor = on_floor( distanceToFloor );
    }

    if(events[SDL_SCANCODE_SPACE].type == SDL_KEYDOWN && !e->locked)
    {
        if(distanceToFloor < 0.7f)
        {
            e->state |= ES_Jumping;
            e->acceleration.z = GRAVITY * 40.0f;
            gf3d_animation_play(e->animationManager, "jump", 1);
        }
    }
    else if(events[SDL_SCANCODE_J].type == SDL_KEYDOWN)
    {
        app_gaara_punch(e);
    }
    else if (events[SDL_SCANCODE_K].type == SDL_KEYDOWN)
    {
        app_gaara_throw_sand(e);
    }
    
    /* Get camera angles */
    gf3d_camera_get_angles(&camera_f, &camera_r, NULL);
    vector3d_normalize(&camera_f);
    vector3d_normalize(&camera_r);

    if (e->locked) return;
    /* Forward and Backwards */
    if (events[SDL_SCANCODE_W].type == SDL_KEYDOWN)
    {
        e->rotation.x = 90.0f;
        vector3d_set_magnitude(&camera_f, MAX_SPEED);
        vector3d_add(e->velocity, e->velocity, camera_f);
        if(!gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
        {
            gf3d_animation_play(e->animationManager, "running", 1);
        }
    }
    else if (events[SDL_SCANCODE_S].type == SDL_KEYDOWN)
    {
        e->rotation.x = 270.0f;
        vector3d_set_magnitude(&camera_f, MAX_SPEED);
        vector3d_sub(e->velocity, e->velocity, camera_f);
        if(!gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
        {
            gf3d_animation_play(e->animationManager, "running", 1);
        }
    }
    else if (events[SDL_SCANCODE_W].type == SDL_KEYUP || 
             events[SDL_SCANCODE_S].type == SDL_KEYUP)
    {
        vector3d_clear(e->velocity);

        if( gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
        {
            gf3d_animation_play(e->animationManager, "idle", 1);
        }
    }
    // else if (events[SDL_SCANCODE_S].type == SDL_KEYUP)
    // {
    //     vector3d_clear(e->velocity);

    //     if( gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
    //     {
    //         gf3d_animation_play(e->animationManager, "idle", 1);
    //     }
    // }

    /* Right and Left */
    if (events[SDL_SCANCODE_D].type == SDL_KEYDOWN)
    {
        /* To rotate according to direction */
        if (e->velocity.y > 0)
        {
            e->rotation.x = 45.0f;
        }
        else if (e->velocity.y < 0)
        {
            e->rotation.x = -45.0f;
        }
        else
        {
            e->rotation.x = 0.0f;
        }
        
        vector3d_set_magnitude(&camera_r, MAX_SPEED);
        vector3d_sub(e->velocity, e->velocity, camera_r);
        if(!gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
        {
            gf3d_animation_play(e->animationManager, "running", 1);
        }
    }
    else if (events[SDL_SCANCODE_A].type == SDL_KEYDOWN)
    {
        /* To rotate according to direction */
        if (e->velocity.y > 0)
        {
            e->rotation.x = 135.0f;
        }
        else if (e->velocity.y < 0)
        {
            e->rotation.x = 225.0f;
        }
        else
        {
            e->rotation.x = 180.0f;
        }
        
        vector3d_set_magnitude(&camera_r, MAX_SPEED);
        vector3d_add(e->velocity, e->velocity, camera_r);
        if(!gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
        {
            gf3d_animation_play(e->animationManager, "running", 1);
        }
    }
    else if (events[SDL_SCANCODE_D].type == SDL_KEYUP || 
             events[SDL_SCANCODE_A].type == SDL_KEYUP)
    {
        vector3d_clear(e->velocity);
        if( gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
        {
            gf3d_animation_play(e->animationManager, "idle", 1);
        }
    }
    // else if (events[SDL_SCANCODE_A].type == SDL_KEYUP)
    // {
    //     vector3d_clear(e->velocity);
    //     if( gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
    //     {
    //         gf3d_animation_play(e->animationManager, "idle", 1);
    //     }
    // }

    /* 
    since we set the events in an array, even if we stop using
    a key, we get the key as input after we stop pressing it.
    To ignore these, we set the type to something else
    */
    for(i = 0; i < sizeof(usedScancodes) / sizeof(int); i++)
    {
        if ( events[ usedScancodes[i] ].type == SDL_KEYUP )
            events[ usedScancodes[i] ].type = -SDL_KEYUP;
    }
}

void app_gaara_punch(Entity *self)
{
    float fcount = 0.0f;
    float currf = 0.0f;

    if(!self) return;

    if(self->state & ES_Idle)
    {
        if( !gf3d_animation_is_playing(self->animationManager, "swipe right") )
        {
            gf3d_animation_play(self->animationManager, "swipe right", 1);
            self->state &= ~ES_Idle;
            self->state |= ES_Attacking;
            self->velocity.x = self->velocity.y = 0.0f;
            self->locked = 1;
        }
    }
    else if (self->state & ES_Attacking)
    {
        if( gf3d_animation_is_playing(self->animationManager, "swipe right") )
        {
            fcount = gf3d_animation_get_frame_count(self->animationManager, "swipe right");
            currf = gf3d_animation_get_current_frame(self->animationManager);
            if ( fcount - currf <= 8.0f )
            {
                self->locked = 2;
            }
        }
        else if ( gf3d_animation_is_playing(self->animationManager, "swipe left") )
        {
            fcount = gf3d_animation_get_frame_count(self->animationManager, "swipe left");
            currf = gf3d_animation_get_current_frame(self->animationManager);
            if( fcount - currf <= 8.0f )
            {
                self->locked = 3;
            }
        }
    }
    
}

void app_gaara_throw_sand(Entity *self)
{
    float fcount = 0.0f;
    float currf = 0.0f;
    if(!self) return;

    if(self->state & ES_Idle)
    {
        if(!gf3d_animation_is_playing(self->animationManager, "throw sand"))
        {
            gf3d_animation_play(self->animationManager, "throw sand", 1);
            self->state &= ~ES_Idle;
            self->state |= ES_Attacking;
            self->velocity.x = self->velocity.y = 0.0f;
            self->locked = 1;
        }
    }
}

void app_gaara_think(Entity *self)
{
    float fcount = 0.0f; /* frame count */
    float currf = 0.0f; /* current frame */
    float distanceToFloor = 0.0f;
    Uint8 onFloor = 0;

    if(!self->animationManager || !self->modelBox)
    {
        return;
    }

    distanceToFloor = distance_to_floor( self->modelBox->shapes[0].position.z - self->modelBox->shapes[0].extents.z );
    onFloor = on_floor( distanceToFloor );

    if ( gfc_line_cmp("jump", gf3d_animation_get_current_animation_name(self->animationManager)) == 0 )
    {
        fcount = gf3d_animation_get_frame_count(self->animationManager, "jump");
        currf = gf3d_animation_get_current_frame(self->animationManager);
        if(gf3d_animation_is_playing(self->animationManager, "jump"))
        {
            if( fcount - currf <= 0.5f )
            {
                gf3d_animation_play(self->animationManager, "idle", 1);
            }
            else if ( fcount - currf <= 32.5f )
            {
                gf3d_animation_pause(self->animationManager, "jump");
            }
        }
        else
        {
            if(onFloor)
            {
                gf3d_animation_play(self->animationManager, "jump", gf3d_animation_get_current_frame(self->animationManager));
            }
        }
        
    }
    else if( gf3d_animation_is_playing(self->animationManager, "swipe right") )
    {
        fcount = gf3d_animation_get_frame_count(self->animationManager, "swipe right");
        currf = gf3d_animation_get_current_frame(self->animationManager);
        if( fcount - currf <= 0.5f )
        {
            if(self->locked == 2)
            {
                gf3d_animation_play(self->animationManager, "swipe left", 1);
            } 
            else 
            {
                self->state &= ~ES_Attacking;
                self->state |= ES_Idle;
                gf3d_animation_play(self->animationManager, "idle", 1);
                self->locked = 0;
            }
        }
    }
    else if ( gf3d_animation_is_playing(self->animationManager, "swipe left") )
    {
        fcount = gf3d_animation_get_frame_count(self->animationManager, "swipe left");
        currf = gf3d_animation_get_current_frame(self->animationManager);
        if( fcount - currf <= 0.5f )
        {
            if( self->locked == 3 )
            {
                gf3d_animation_play(self->animationManager, "swipe forward", 1);
            }
            else
            {
                self->state &= ~ES_Attacking;
                self->state |= ES_Idle;
                gf3d_animation_play(self->animationManager, "idle", 1);
                self->locked = 0;
            }
        }
    }
    else if( gf3d_animation_is_playing(self->animationManager, "swipe forward") )
    {
        fcount = gf3d_animation_get_frame_count(self->animationManager, "swipe forward");
        currf = gf3d_animation_get_current_frame(self->animationManager);
        if( fcount - currf <= 0.5f )
        {
            self->state &= ~ES_Attacking;
            self->state |= ES_Idle;
            self->locked = 0;
            gf3d_animation_play(self->animationManager, "idle", 1);
        }
    }
    else if ( gf3d_animation_is_playing(self->animationManager, "throw sand") )
    {
        fcount = gf3d_animation_get_frame_count(self->animationManager, "throw sand");
        currf = gf3d_animation_get_current_frame(self->animationManager);
        if(fcount - currf <= 0.5f)
        {
            self->state &= ~ES_Attacking;
            self->state |= ES_Idle;
            gf3d_animation_play(self->animationManager, "idle", 1);
            self->locked = 0;
        }
    }
}

void app_gaara_update(Entity *self)
{
    if(!self) return;
    if(self->think) self->think(self);
    gf3d_entity_general_update(self);
    gfc_matrix_rotate(self->modelMat, self->modelMat, (self->rotation.y + 90) * GFC_DEGTORAD, vector3d(1, 0, 0));
}

void app_gaara_touch(Entity *self, Entity *other)
{

}