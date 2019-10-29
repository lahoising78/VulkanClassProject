#include "app_gaara.h"

#include "simple_logger.h"
#include "gf3d_camera.h"
#include "gf3d_combat.h"
#include "gf3d_common.h"

// extern float worldTime;

/* Regular stuff */
void app_gaara_punch(Entity *self);
void app_gaara_throw_sand(Entity *self);
void app_gaara_charge(Entity *self);

/* Sand attacks */
void app_gaara_sand_attack(Entity *owner, enum GaaraSandAttackType type);
void app_gaara_sand_attack_update(Entity *self);

/* sand burial */
void app_gaara_sand_burial(Entity *ent);
void app_gaara_sand_burial_update(Entity *self);
void app_gaara_sand_burial_touch(Entity *self, Entity *other);

/* sand storm */
void app_gaara_sand_storm(Entity *ent);
void app_gaara_sand_storm_update(Entity *self);
void app_gaara_sand_storm_touch(Entity *self, Entity *other);

/* knock up */
void app_gaara_knockup(Entity *self);
void app_gaara_knockup_update(Entity *self);
void app_gaara_knockup_touch(Entity *self, Entity *other);

/* sand tsunami */
void app_gaara_sand_tsunami(Entity *ent);
void app_gaara_sand_tsunami_update(Entity *self);
void app_gaara_sand_tsunami_touch(Entity *self, Entity *other);

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
    ent->animationManager = gf3d_animation_manager_init(16, ent->model);
    gf3d_animation_load(ent->animationManager, "idle",              "gaara_idle",               1, 60);
    gf3d_animation_play(ent->animationManager, "idle",                                          1);
    gf3d_animation_load(ent->animationManager, "running",           "gaara_running",            1, 20);
    gf3d_animation_load(ent->animationManager, "swipe right",       "gaara_swipe_right",        1, 68);
    gf3d_animation_set_speed(ent->animationManager,                 "swipe right",              1.8f);
    gf3d_animation_load(ent->animationManager, "swipe left",        "gaara_swipe_left",         1, 68);
    gf3d_animation_set_speed(ent->animationManager,                 "swipe left",               1.8f);
    gf3d_animation_load(ent->animationManager, "swipe forward",     "gaara_forward_attack",     1, 50);
    gf3d_animation_set_speed(ent->animationManager,                 "swipe forward",            1.8f);
    gf3d_animation_load(ent->animationManager, "throw sand",        "gaara_throw_sand",         1, 51);
    gf3d_animation_load(ent->animationManager, "jump",              "gaara_jump",               1, 58);
    gf3d_animation_load(ent->animationManager, "charge",            "gaara_charge",             1, 36);
    gf3d_animation_load(ent->animationManager, "knock up",          "gaara_knock_up",           1, 70);
    gf3d_animation_load(ent->animationManager, "choke",             "gaara_choke",              1, 115);
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
        SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_L, 
        SDL_SCANCODE_O, SDL_SCANCODE_P
    };

    /* dont take input if being attacked */
    if(e->locked < 0)
    {
        return;
    }
    
    if (self->entity->modelBox)
    {
        distanceToFloor = distance_to_floor( e->modelBox->shapes[0].position.z - e->modelBox->shapes[0].extents.z );
        onFloor = on_floor( distanceToFloor );
    }

    if(events[SDL_SCANCODE_SPACE].type == SDL_KEYDOWN && (!e->locked || e->locked >= 100))
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
    else if (events[SDL_SCANCODE_O].type == SDL_KEYUP)
    {
        if(e->locked >= 100) 
        {
            e->locked+=100;
        }
        else
        {
            e->locked = 100;
        }
        slog("locked: %d", e->locked);
    }
    
    if ( e->locked % 100 == 0 || e->locked == 301 )
    {
        if(events[SDL_SCANCODE_P].type == SDL_KEYUP)
        {
            /* Stop Expecting a Jutsu */
            e->locked = 0;
        }

        /* Get camera angles */
        gf3d_camera_get_angles(&camera_f, &camera_r, NULL);
        vector3d_normalize(&camera_f);
        vector3d_normalize(&camera_r);

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
    }

    /* 
    since we set the events in an array, even if we stop using
    a key, we get the key as input after we stop pressing it.
    To ignore these, we set the type to something else
    */
    for(i = 0; i < sizeof(usedScancodes) / sizeof(int); i++)
    {
        if ( events[ usedScancodes[i] ].type == SDL_KEYUP )
        {
            events[ usedScancodes[i] ].type = -SDL_KEYUP;
        }
    }
}

void app_gaara_punch(Entity *self)
{
    float fcount = 0.0f;
    float currf = 0.0f;

    if(!self) return;

    if( self->locked >= 100)
    {
        if( self->locked == 100 )
        {
            gf3d_animation_play(self->animationManager, "knock up", 1);
            self->locked = 101;
        }
        else if( self->locked == 200 )
        {
            gf3d_animation_play(self->animationManager, "choke", 1);
            self->locked = 201;
            app_gaara_sand_burial(self);
        }
        else if( self->locked == 300 )
        {
            self->locked = 301;
            app_gaara_sand_storm(self);
        }
        else if ( self->locked == 400 )
        {
            self->locked = 401;
            app_gaara_sand_tsunami(self);
        }
        self->state &= ~ES_Idle;
        self->state |= ES_Attacking;
        self->velocity.x = self->velocity.y = 0.0f;
        return;
    }

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
    // float fcount = 0.0f;
    // float currf = 0.0f;
    Entity *projectile = NULL;
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

            projectile = gf3d_combat_projectile_new(self, self->enemy);
            if(!projectile) return;

            projectile->model = gf3d_model_load("sand", "sand");
            projectile->hitboxes = gf3d_collision_armor_new(1);
            gf3d_collision_armor_add_shape(
                projectile->hitboxes,
                gf3d_shape(projectile->position, vector3d(1, 1, 1), gf3d_model_load("cube", "cube")),
                vector3d(0, 0, 0),
                "body"
            );

            projectile->health = 2;
            projectile->chakra = 3;
            projectile->hitstun = 0.1f;
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
            gf3d_common_chakra_new(self);
        }
    }
}

void app_gaara_think(Entity *self)
{
    float fcount = 0.0f; /* frame count */
    float currf = 0.0f; /* current frame */
    float distanceToFloor = 0.0f;
    Uint8 onFloor = 0;
    // Vector3D forward, right;

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
    else if ( gf3d_animation_is_playing(self->animationManager, "knock up") )
    {
        fcount = gf3d_animation_get_frame_count(self->animationManager, "knock up");
        currf = gf3d_animation_get_current_frame(self->animationManager);
        if(fcount - currf <= 0.5f)
        {
            self->state &= ~ES_Attacking;
            self->state |= ES_Idle;
            gf3d_animation_play(self->animationManager, "idle", 1);
            self->locked = 0;
        }
        else if ( (fcount - currf) <= 20.0f && self->locked == 101 )
        {
            app_gaara_knockup(self);
        }
    }
    else if ( gf3d_animation_is_playing(self->animationManager, "choke") )
    {
        fcount = gf3d_animation_get_frame_count(self->animationManager, "choke");
        currf = gf3d_animation_get_current_frame(self->animationManager);
        if( fcount - currf <= 0.5f )
        {
            self->state &= ~ES_Attacking;
            self->state |= ES_Idle;
            gf3d_animation_play(self->animationManager, "idle", 1);
            self->locked = 0;
        }
        else if ( (self->locked == 201) && (currf * 3 >= 35.0f) )
        {
            gf3d_animation_pause(self->animationManager, "choke");
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

    gf3d_combat_meele_attack(owner, other, self->health, self->chakra, self->hitstun);
}

/* *********************************************
 * Gaara sand 
 * ********************************************/
void app_gaara_sand_attack(Entity *owner, enum GaaraSandAttackType type)
{
    Entity *sand = NULL;
    Vector3D forward, right;
    // Vector3D start, dst;
    // Vector3D buff;

    if(!owner) return;

    sand = gf3d_entity_new();
    if(!sand) return;
        
    vector3d_angle_vectors(owner->rotation, &forward, &right, NULL);

    switch (type)
    {
    case ATK_RIGHT:
            vector3d_scale(sand->position, forward, GAARA_SA_SWIPE_FWD1_OFFSET);        /* scale forward and save to start */
            vector3d_add(sand->position, sand->position, owner->position);  /* add start to position of the creating entity */
            vector3d_scale(right, right, -GAARA_SA_SWIPE_LEFT_OFFSET);                       /* right becomes the left, already scaled */
            vector3d_sub(sand->rotation, sand->position, right);            /* add right to start to get dest */
            vector3d_add(sand->position, sand->position, right);            /* add left to start to get start */
            sand->health = GAARA_SA_SWIPE_DMG;                              /* health of sand will be damage */
            sand->chakra = GAARA_SA_SWIPE_KICK;                             /* chakra of sand will be kick */
            sand->hitstun  = GAARA_SA_SWIPE_HITSTUN;                        /* hitstun of sand will be hitstun duration */
        break;

    case ATK_LEFT:
            vector3d_scale(sand->position, forward, GAARA_SA_SWIPE_FWD2_OFFSET);
            vector3d_add(sand->position, sand->position, owner->position);
            vector3d_scale(right, right, -GAARA_SA_SWIPE_LEFT_OFFSET);
            vector3d_add(sand->rotation, sand->position, right);
            vector3d_sub(sand->position, sand->position, right);
            sand->health = GAARA_SA_FWD_DMG;
            sand->chakra = GAARA_SA_FWD_KICK;
            sand->hitstun = GAARA_SA_SWIPE_HITSTUN;
        break;

    case ATK_FORWARD:
            vector3d_scale(sand->position, forward, GAARA_SA_SWIPE_FWD1_OFFSET / 2);
            vector3d_add(sand->position, sand->position, owner->position);
            vector3d_scale(sand->rotation, forward, GAARA_SA_SWIPE_FWD2_OFFSET * 1.3);
            vector3d_add(sand->rotation, sand->position, sand->rotation);
            sand->health = GAARA_SA_FWD_DMG;
            sand->chakra = GAARA_SA_FWD_KICK;
            sand->hitstun = GAARA_SA_SWIPE_HITSTUN;
        break;
    
    default:
        break;
    }

    vector3d_sub(sand->velocity, sand->rotation, sand->position);
    vector3d_set_magnitude(&sand->velocity, SA_SPEED);

    sand->model = gf3d_model_load("sand", "sand");

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

    vector3d_set_magnitude(&self->velocity, SA_SPEED);
}

/* *****************************************
 * SAND BURIAL
 * *****************************************/
void app_gaara_sand_burial(Entity *ent)
{
    Entity *proj = NULL;
    Entity **ents = NULL;
    Vector3D forward;

    if(!ent) return;

    proj = gf3d_entity_new();
    if(!proj) return;

    vector3d_angle_vectors(ent->rotation, &forward, NULL, NULL);
    vector3d_scale(proj->position, forward, GAARA_SB_FWD);
    vector3d_add(proj->position, proj->position, ent->position);
    vector3d_scale(proj->rotation, forward, GAARA_SB_EXTENT_FWD);
    vector3d_add(proj->rotation, proj->rotation, proj->position); /* once again, rotation is the destination of the attack */

    vector3d_sub(proj->velocity, proj->rotation, proj->position);
    vector3d_set_magnitude(&proj->velocity, GAARA_SB_FWD_SPEED);

    proj->model = gf3d_model_load("sand", "sand");
    proj->hitboxes = gf3d_collision_armor_new(1);
    gf3d_collision_armor_add_shape(
        proj->hitboxes,
        gf3d_shape(proj->position, vector3d(1, 1, 1), gf3d_model_load("cube", "cube")),
        vector3d(0, 0, 0),
        "body"
    );

    proj->update = app_gaara_sand_burial_update;
    proj->touch  = app_gaara_sand_burial_touch;

    /* the first entity in data will be owner; the second will be the entity we touched */
    proj->data = (Entity**)gfc_allocate_array(sizeof(Entity*), 2);
    if(!proj->data) return;
    ents = (Entity**)proj->data;
    ents[ GAARA_SB_DATA_OWNER ] = ent;
    ents[ GAARA_SB_DATA_TOUCHED ] = NULL;
}

void app_gaara_sand_burial_update(Entity *self)
{
    Entity *owner = NULL;
    Entity *touched = NULL;
    Entity **ents = NULL;
    Vector3D distanceToDst;

    if(!self) return;

    ents = (Entity**)self->data;
    if(!ents) return;
    owner = ents[ GAARA_SB_DATA_OWNER ];
    if(!owner) return;
    
    touched = ents[ GAARA_SB_DATA_TOUCHED ];
    if(touched)
    {
        vector3d_set_magnitude(&self->velocity, GAARA_SB_UP_SPEED);
    }
    else
    {
        vector3d_set_magnitude(&self->velocity, GAARA_SB_FWD_SPEED);
    }
    gf3d_entity_general_update(self);

    vector3d_sub(distanceToDst, self->position, self->rotation);
    if( vector3d_magnitude_squared(distanceToDst) <= 2.0f || self->state & ES_Walking_Out)
    {
        if( !gf3d_animation_is_playing(owner->animationManager, "choke") ||
            gf3d_animation_get_current_frame(owner->animationManager) * 3 >= GAARA_SB_END_FRAME)
        {
            if(touched) gf3d_combat_meele_attack(self, touched, GAARA_SB_DMG, 0, GAARA_SB_HITSTUN);
            if(self->data) free(self->data);
            self->data = NULL;
            gf3d_entity_free(self);
        }
        if(owner && !touched) gf3d_common_init_state(owner);
    }
}

void app_gaara_sand_burial_touch(Entity *self, Entity *other)
{
    Entity *owner = NULL;
    Entity *touched = NULL;
    Entity **ents = NULL;

    if(!self || !other) return;

    ents = (Entity**)self->data;
    if(!ents) return;

    owner = ents[ GAARA_SB_DATA_OWNER ];
    if(!owner || owner == other) return;
    touched = ents[ GAARA_SB_DATA_TOUCHED ];

    if(touched)
    {
        vector3d_copy(touched->position, self->position);
        vector3d_clear(touched->velocity);
        vector3d_clear(touched->acceleration);

        if( self->position.z >= self->rotation.z )
        {
            self->position.z = self->rotation.z;
        }
    }
    else
    {
        ents[ GAARA_SB_DATA_TOUCHED ] = other;
        
        owner->locked = 202;
        gf3d_animation_unpause(owner->animationManager, "choke");
        
        vector3d_copy(self->position, other->position);

        vector3d_clear(self->velocity);
        self->velocity.z = GAARA_SB_UP_SPEED;

        vector3d_add(self->rotation, other->position, vector3d(0, 0, GAARA_SB_EXTENT_UP));
        vector3d_scale(self->scale, self->scale, GAARA_SB_TOUCH_SCALE);
    }
}

/* ******************************
 * SAND STORM
 * 
 * -note that chakra will be the time the thing has been alive
 * ******************************/
void app_gaara_sand_storm(Entity *ent)
{
    int i;
    Entity *proj = NULL;
    Vector3D forward, right;
    
    if(!ent) return;
    vector3d_angle_vectors(ent->rotation, &forward, &right, NULL);

    for( i = 0; i < GAARA_SS_NUM; i++)
    {
        proj = gf3d_entity_new();
        if(!proj) return;

        proj->data = ent;

        if(i == 0)
        {
            vector3d_scale(proj->position, forward, GAARA_SS_DISTANCE_TO_ENT);
        }
        else if (i == 1)
        {
            vector3d_scale(proj->position, forward, -GAARA_SS_DISTANCE_TO_ENT);
        }
        else if (i == 2)
        {
            vector3d_scale(proj->position, right, GAARA_SS_DISTANCE_TO_ENT);
        }
        else if (i == 3)
        {
            vector3d_scale(proj->position, right, -GAARA_SS_DISTANCE_TO_ENT);
        }
        vector3d_add(proj->position, ent->position, proj->position);
        proj->position.z += (i - 1.5f) * 2;
        proj->scale = vector3d(GAARA_SS_SCALE, GAARA_SS_SCALE, GAARA_SS_SCALE);

        proj->model = gf3d_model_load("sand", "sand");
        proj->hitboxes = gf3d_collision_armor_new(1);
        gf3d_collision_armor_add_shape(
            proj->hitboxes,
            gf3d_shape(proj->position, proj->scale, gf3d_model_load("cube", "cube")),
            vector3d(0, 0, 0),
            "body"
        );

        proj->update = app_gaara_sand_storm_update;
        proj->touch = app_gaara_sand_storm_touch;
    }
}

void app_gaara_sand_storm_update(Entity *self)
{
    Vector3D in;
    Entity *owner = NULL;
    float z;
    if(!self) return;

    owner = (Entity*)self->data;
    if(!owner) return;

    // if(owner->locked < GAARA_SS_LOCKED_MIN || owner->locked > GAARA_SS_LOCKED_MAX)
    if(self->chakra >= GAARA_SS_TIME)
    {
        gf3d_common_init_state(owner);
        self->data = NULL;
        gf3d_entity_free(self);
        return;
    }

    vector3d_sub(in, self->position, owner->position);
    vector3d_normalize(&in);
    vector3d_cross_product(&self->velocity, in, vector3d(0, 0, 1));
    vector3d_scale(self->velocity, self->velocity, GAARA_SS_SPEED);

    gf3d_entity_general_update(self);

    vector3d_sub(in, self->position, owner->position);
    in.z = 0;
    z = self->position.z;
    vector3d_set_magnitude(&in, GAARA_SS_DISTANCE_TO_ENT);
    vector3d_add(self->position, owner->position, in);
    self->position.z = z;

    self->chakra += worldTime;
}

void app_gaara_sand_storm_touch(Entity *self, Entity *other)
{
    Entity *owner = NULL;
    Vector3D dir;

    if(!self || !other) return;

    owner = (Entity*)self->data;
    if(!owner || owner == other) return;

    vector3d_sub(dir, other->position, owner->position);
    gf3d_combat_attack(owner, other, GAARA_SS_DMG, GAARA_SS_KICK, dir, GAARA_SS_HITSTUN);
}

/* *****************************************
 * KNOCK UP
 * *****************************************/
void app_gaara_knockup(Entity *self)
{
    Entity *projectile = NULL;
    if(!self) return;
    self->locked = 102;

    projectile = gf3d_entity_new();
    if(!projectile) return;

    if(self->enemy) vector3d_copy(projectile->position, self->enemy->position);
    else vector3d_copy(projectile->position, self->position);
    projectile->position.z = MAX_STAGE_Z - 10;
    
    projectile->velocity.z = GAARA_KU_SPEED;
    projectile->rotation.y = -90.0f;

    projectile->model = gf3d_model_load("sand", "sand");
    projectile->hitboxes = gf3d_collision_armor_new(1);
    gf3d_collision_armor_add_shape(
        projectile->hitboxes,
        gf3d_shape(projectile->position, vector3d(1, 1, 1), gf3d_model_load("cube", "cube")),
        vector3d(0, 0, 0),
        "body"
    );

    projectile->data = self;

    projectile->update = app_gaara_knockup_update;
    projectile->touch = app_gaara_knockup_touch;
}

void app_gaara_knockup_update(Entity *self)
{
    if(!self) return;
    gf3d_entity_general_update(self);

    if( self->position.z > GAARA_KU_MAX_Z )
    {
        self->data = NULL;
        gf3d_entity_free(self);
    }
}

void app_gaara_knockup_touch(Entity *self, Entity *other)
{
    Entity *owner = NULL;

    if(!self || !other) return;
    owner = (Entity*)self->data;

    if(!owner || owner == other) return;

    gf3d_combat_meele_attack(self, other, GAARA_KU_DMG, GAARA_KU_KICK, GAARA_KU_HITSTUN);
    other->state |= ES_Jumping;
    
    self->data = NULL;
    gf3d_entity_free(self);
}

/* *****************************
 * SAND TSUNAMI
 * *****************************/
void app_gaara_sand_tsunami(Entity *ent)
{
    int i;
    Entity *proj = NULL;
    Vector3D forward;

    if(!ent) return;
    vector3d_angle_vectors(ent->rotation, &forward, NULL, NULL);

    for(i = 0; i < GAARA_ST_NUM; i++)
    {
        proj = gf3d_entity_new();
        if(!proj) continue;

        vector3d_copy(proj->position, ent->position);
        proj->position.z += GAARA_ST_UP_OFFSET;

        vector3d_scale(proj->rotation, forward, GAARA_ST_FWD_OFFSET + GAARA_ST_FWD_OFFSET_INC * i); /* rotation is the destination */
        vector3d_add(proj->rotation, proj->rotation, ent->position);
        proj->rotation.z = MAX_STAGE_Z + STAGE_SCALE_Z;

        proj->model = gf3d_model_load("sand", "sand");
        proj->hitboxes = gf3d_collision_armor_new(1);
        gf3d_collision_armor_add_shape(
            proj->hitboxes, 
            gf3d_shape(proj->position, proj->scale, gf3d_model_load("cube", "cube")),
            vector3d(0, 0, 0),
            "body"
        );

        proj->update = app_gaara_sand_tsunami_update;
        proj->touch = app_gaara_sand_tsunami_touch;

        proj->locked = i;
        proj->data = ent;
    }
}

void app_gaara_sand_tsunami_update(Entity *self)
{
    Entity *owner = NULL;
    Vector3D distanceToDest;
    if(!self) return;

    vector3d_sub(self->velocity, self->rotation, self->position);
    vector3d_set_magnitude(&self->velocity, GAARA_ST_SPEED);

    gf3d_entity_general_update(self);

    vector3d_sub(distanceToDest, self->rotation, self->position);
    if( vector3d_magnitude_squared(distanceToDest) <= 2.0f || self->state & ES_Walking_Out)
    {
        owner = (Entity*)self->data;
        if( owner && self->locked == GAARA_ST_NUM - 1 )
        {
            gf3d_common_init_state(owner);
        }

        self->data = NULL;
        gf3d_entity_free(self);
    }
}

void app_gaara_sand_tsunami_touch(Entity *self, Entity *other)
{
    Entity *owner = NULL;

    if(!self || !other) return;

    owner = (Entity*)self->data;
    if(!owner || owner == other) return;

    gf3d_combat_attack(owner, other, GAARA_ST_DMG, GAARA_ST_KICK, self->velocity, GAARA_ST_HITSTUN);
}