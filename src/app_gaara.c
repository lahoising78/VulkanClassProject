#include "app_gaara.h"

#include "simple_logger.h"
#include "gf3d_camera.h"
#include "gf3d_combat.h"

void app_gaara_punch(Entity *self);
void app_gaara_throw_sand(Entity *self);
void app_gaara_charge(Entity *self);

void app_gaara_sand_attack(Entity *owner, enum GaaraSandAttackType type);
void app_gaara_sand_attack_update(Entity *self);

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
    gf3d_animation_load(ent->animationManager, "charge", "gaara_charge", 1, 36);
    ent->modelOffset.z = -4.9f;
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
    else if (events[SDL_SCANCODE_L].type == SDL_KEYDOWN)
    {
        app_gaara_charge(e);
    }
    else if (events[SDL_SCANCODE_L].type == SDL_KEYUP)
    {
        if(gf3d_animation_is_playing(e->animationManager, "charge"))
        {
            gf3d_animation_play(e->animationManager, "idle", 1);
            e->locked = 0;
        }
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
        if ( e->velocity.y > MAX_SPEED )
        {
            e->velocity.y = MAX_SPEED;
        }

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
        if ( e->velocity.y < -MAX_SPEED )
        {
            e->velocity.y = -MAX_SPEED;
        }

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
        /* Cap speed */
        if ( e->velocity.x > MAX_SPEED )
        {
            e->velocity.x = MAX_SPEED;
        }

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
        /* Cap speed */
        if ( e->velocity.x < -MAX_SPEED )
        {
            e->velocity.x = -MAX_SPEED;
        }

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
            app_gaara_sand_attack(self, ATK_RIGHT);
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

void app_gaara_charge(Entity *self)
{
    if(!self) return;
    if(self->state & ES_Idle)
    {
        if(!gf3d_animation_is_playing(self->animationManager, "charge"))
        {
            gf3d_animation_play(self->animationManager, "charge", 1);
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
    Vector3D forward, right;

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
            else if ( fcount - currf <= fcount/2 && (distanceToFloor >= 0.7f) )
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
                app_gaara_sand_attack(self, ATK_LEFT);
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
                app_gaara_sand_attack(self, ATK_FORWARD);
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
    Entity *owner = NULL;
    
    if(!self || !other) return;
    owner = self->data;
    if(!owner) return;

    gf3d_combat_meele_attack(owner, other, self->health, self->chakra);
}

/* *********************************************
 * Gaara sand 
 * ********************************************/
void app_gaara_sand_attack(Entity *owner, enum GaaraSandAttackType type)
{
    Entity *sand = NULL;
    Vector3D forward, right;
    // Vector3D start, dst;
    Vector3D buff;

    if(!owner) return;

    sand = gf3d_entity_new();
    if(!sand) return;
        
    vector3d_angle_vectors(owner->rotation, &forward, &right, NULL);

    switch (type)
    {
    case ATK_RIGHT:
            vector3d_scale(sand->position, forward, SAND_FORWARD); /* scale forward and save to start */
            vector3d_add(sand->position, sand->position, owner->position); /* add start to position of the creating entity */
            vector3d_scale(right, right, -SAND_LEFT); /* right becomes the left, already scaled */
            vector3d_sub(sand->rotation, sand->position, right); /* add right to start to get dest */
            vector3d_add(sand->position, sand->position, right); /* add left to start to get start */
            sand->health = DMG_GAARA_SAND_ATK; /* health of sand will be damage */
            sand->chakra = KICK_GAARA_SAND_ATK; /* chakra of sand will be kick */
        break;

    case ATK_LEFT:
            vector3d_scale(sand->position, forward, SAND_FORWARD);
            vector3d_add(sand->position, sand->position, owner->position);
            vector3d_scale(right, right, -SAND_LEFT);
            vector3d_add(sand->rotation, sand->position, right);
            vector3d_sub(sand->position, sand->position, right);
            sand->health = DMG_GAARA_SAND_ATK;
            sand->chakra = KICK_GAARA_SAND_ATK;
        break;

    case ATK_FORWARD:
            vector3d_scale(sand->position, forward, SAND_FORWARD / 2);
            vector3d_add(sand->position, sand->position, owner->position);
            vector3d_scale(sand->rotation, forward, SAND_FORWARD);
            vector3d_add(sand->rotation, sand->position, sand->rotation);
            sand->health = DMG_GAARA_SAND_ATK;
            sand->chakra = KICK_GAARA_SAND_FWD_ATK;
        break;
    
    default:
        break;
    }

    vector3d_sub(sand->velocity, sand->rotation, sand->position);
    vector3d_set_magnitude(&sand->velocity, SAND_SPEED);

    sand->model = gf3d_model_load("dino", "dino");

    sand->hitboxes = gf3d_collision_armor_new(1);
    gf3d_collision_armor_add_shape(
        sand->hitboxes,
        gf3d_shape(sand->position, vector3d(1, 1, 1), gf3d_model_load("cube", "cube")),
        vector3d(0, 0, 0),
        "body"
    );

    sand->update = app_gaara_sand_attack_update;
    sand->touch = app_gaara_touch;

    sand->data = owner;
}

void app_gaara_sand_attack_update(Entity *self)
{
    Vector3D buff;
    gf3d_entity_general_update(self);

    // calculate how far we are from destination
    vector3d_sub(buff, self->rotation, self->position);
    if( vector3d_magnitude_squared(buff) <= 0.25f )
    {
        /* the entity has reached the limit */
        self->data = NULL;
        gf3d_entity_free(self);
    }

    vector3d_set_magnitude(&self->velocity, SAND_SPEED);
}