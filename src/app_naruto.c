#include "simple_logger.h"
#include "app_naruto.h"
#include "gf3d_camera.h"
#include "gf3d_timer.h"
#include "gf3d_common.h"

void app_naruto_add_hitbox(struct Entity_S *ent, char *name); /* predefined hitboxes for attacks */

/* basic */
void app_naruto_punch(struct Entity_S *e);
void app_naruto_throw_knife(struct Entity_S *e);
void app_naruto_charge(struct Entity_S *e);

/* clones */
void app_naruto_punch_create_shadow_clones(struct Entity_S *self);
void app_naruto_clone_update( struct Entity_S *e );
void app_naruto_clone_touch(struct Entity_S *self, struct Entity_S *other);

/* rasenshuriken */
void app_naruto_rasenshuriken(Entity *ent);
void app_naruto_rasenshuriken_update(Entity *self);
void app_naruto_rasenshuriken_touch(Entity *self, Entity *other);

Entity *app_naruto_new()
{
    Entity *e = NULL;

    e = gf3d_entity_new();
    e->health = 100;
    e->healthmax = 100;
    e->chakra = 100;
    e->chakraMax = 100;
    e->think = app_naruto_think;
    e->touch = app_naruto_touch;
    e->update = app_naruto_update;
    e->model = gf3d_model_new();
    e->model->texture = gf3d_texture_load("images/naruto.png");
    e->animationManager = gf3d_animation_manager_init(10, e->model);
    gf3d_animation_load(        e->animationManager, "idle",            "naruto_idle",          ANIM_NARUTO_IDLE_START,             ANIM_NARUTO_IDLE_END);
    gf3d_animation_play(        e->animationManager, "idle",                                    ANIM_NARUTO_IDLE_START);
    gf3d_animation_load(        e->animationManager, "running",         "naruto_running",       ANIM_NARUTO_RUN_START,              ANIM_NARUTO_RUN_END);
    gf3d_animation_load(        e->animationManager, "jump up",         "naruto_jump_up",       ANIM_NARUTO_JUMP_UP_START,          ANIM_NARUTO_JUMP_UP_END);
    gf3d_animation_load(        e->animationManager, "jump down",       "naruto_jump_down",     ANIM_NARUTO_JUMP_DOWN_START,        ANIM_NARUTO_JUMP_DOWN_END);
    gf3d_animation_load(        e->animationManager, "punch",           "naruto_punch",         ANIM_NARUTO_PUNCH_START,            ANIM_NARUTO_PUNCH_END);
    gf3d_animation_load(        e->animationManager, "punch mirror",    "naruto_punch_mirror",  ANIM_NARUTO_PUNCH_START,            ANIM_NARUTO_PUNCH_END);
    gf3d_animation_load(        e->animationManager, "kick",            "naruto_kick",          ANIM_NARUTO_KICK_START,             ANIM_NARUTO_KICK_END);
    gf3d_animation_load(        e->animationManager, "throw",           "naruto_throw",         ANIM_NARUTO_THROW_START,            ANIM_NARUTO_THROW_END);
    gf3d_animation_load(        e->animationManager, "charge",          "naruto_charge",        ANIM_NARUTO_CHARGE_START,           ANIM_NARUTO_CHARGE_END);
    gf3d_animation_load(        e->animationManager, "rasenshuriken",   "naruto_rasenshuriken", ANIM_NARUTO_RASENSHURIKEN_START,    ANIM_NARUTO_RASENSHURIKEN_END);
    gf3d_animation_set_speed(   e->animationManager, "rasenshuriken",   0.5f);
    e->modelOffset.z = -4.9f;
    e->scale = vector3d(5, 5, 5);
    // e->scale = vector3d(1, 1, 1);
    // e->scale = vector3d(0.1, 0.1, 0.1);
    gfc_matrix_identity(e->modelMat);

    return e;
}

void app_naruto_input_handler( struct Player_s *self, SDL_Event* events )
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

    if(e->locked < 0)
    {
        return;
    }

    if (self->entity->modelBox)
    {
        distanceToFloor = distance_to_floor( e->modelBox->shapes[0].position.z - e->modelBox->shapes[0].extents.z );
        onFloor = on_floor( distanceToFloor );
    }

    /* Jumping */
    if (events[SDL_SCANCODE_SPACE].type == SDL_KEYDOWN && (!e->locked || e->locked % 100 == 0))
    {
        if (distanceToFloor < 0.7f)
        {
            e->state |= ES_Jumping;
            e->acceleration.z = GRAVITY * 40.0f;
            gf3d_animation_play(e->animationManager, "jump up", 1);
        }
    }
    /* Punching */
    else if (events[SDL_SCANCODE_J].type == SDL_KEYDOWN)
    {
        app_naruto_punch(e);
    }
    /* Throw Knife */
    else if (events[SDL_SCANCODE_K].type == SDL_KEYDOWN)
    {
        app_naruto_throw_knife(e);
    }
    /* Charge Chakra */
    else if (events[SDL_SCANCODE_L].type == SDL_KEYDOWN)
    {
        app_naruto_charge(e);
    }
    else if (events[SDL_SCANCODE_L].type == SDL_KEYUP)
    {
        if(gf3d_animation_is_playing(e->animationManager, "charge"))
        {
            gf3d_animation_play(e->animationManager, "idle", 1);
            e->locked = 0;
        }
    }
    /* Start Jutsu */
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

    if (e->locked % 100 == 0 ) 
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
            /* Cap speed */
            if ( abs(e->velocity.y) > MAX_SPEED )
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
            /* Cap speed */
            if ( abs(e->velocity.y) > MAX_SPEED )
            {
                e->velocity.y = -MAX_SPEED;
            }

            if(!gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
            {
                gf3d_animation_play(e->animationManager, "running", 1);
            }
        }
        else if (events[SDL_SCANCODE_W].type == SDL_KEYUP)
        {
            vector3d_clear(e->velocity);

            if( gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
            {
                gf3d_animation_play(e->animationManager, "idle", 1);
            }
        }
        else if (events[SDL_SCANCODE_S].type == SDL_KEYUP)
        {
            vector3d_clear(e->velocity);

            if( gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
            {
                gf3d_animation_play(e->animationManager, "idle", 1);
            }
        }

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
            if ( abs(e->velocity.x) > MAX_SPEED )
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
            if ( abs(e->velocity.x) > MAX_SPEED )
            {
                e->velocity.x = -MAX_SPEED;
            }

            if(!gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
            {
                gf3d_animation_play(e->animationManager, "running", 1);
            }
        }
        else if (events[SDL_SCANCODE_D].type == SDL_KEYUP)
        {
            vector3d_clear(e->velocity);
            if( gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
            {
                gf3d_animation_play(e->animationManager, "idle", 1);
            }
        }
        else if (events[SDL_SCANCODE_A].type == SDL_KEYUP)
        {
            vector3d_clear(e->velocity);
            if( gf3d_animation_is_playing(e->animationManager, "running") && onFloor)
            {
                gf3d_animation_play(e->animationManager, "idle", 1);
            }
        }
    }

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


void app_naruto_charge(struct Entity_S *e)
{
    if(!e) return;
    if(e->state & ES_Idle)
    {
        if(!gf3d_animation_is_playing(e->animationManager, "charge"))
        {
            gf3d_animation_play(e->animationManager, "charge", 1);
            e->velocity.x = e->velocity.y = 0.0f;
            e->locked = 1;
        }
        
    }
}

void app_naruto_throw_knife(struct Entity_S* e)
{
    Entity *projectile = NULL;

    if(!e) return;

    if (e->state & ES_Idle)
    {
        if ( !gf3d_animation_is_playing(e->animationManager, "throw") )
        {
            gf3d_animation_play(e->animationManager, "throw", 1);
            e->state &= ~ES_Idle;
            e->state |= ES_Attacking;
            e->velocity.x = e->velocity.y = 0.0f;
            e->locked = 1;

            /* throw kunai */
            projectile = gf3d_combat_projectile_new(e, e->enemy);
            if(!projectile)
            {
                return;
            }
            projectile->model = gf3d_model_load("dino", "dino");
            projectile->hitboxes = gf3d_collision_armor_new(1);
            gf3d_collision_armor_add_shape(
                projectile->hitboxes,
                gf3d_shape(projectile->position, vector3d(1, 1, 1), gf3d_model_load("cube", "cube")),
                vector3d(0, 0, 0),
                "body"
            );

            projectile->health = 2;
            projectile->chakra = 3;
        }
    }
}

void app_naruto_punch_create_shadow_clones(struct Entity_S *self)
{
    Entity *firstShadow = NULL;
    Entity *secondShadow = NULL;
    Vector3D forward, right, finalPosition;
    Entity *e = NULL;
    int i;

    vector3d_angle_vectors(self->rotation, &forward, &right, NULL);
    vector3d_scale(forward, forward, 3.0f);
    vector3d_add(forward, forward, self->position);

    firstShadow = gf3d_entity_new();
    secondShadow = gf3d_entity_new();

    /* 
    only iterates with i=1 and i=-1
    doing this so that we can calculate the direction easily when scaling right vector
    */
    for(i = -1; i < 2; i+=2)
    {
        if (i == -1) e = firstShadow;
        else e = secondShadow;
        
        vector3d_scale(finalPosition, right, 2*i);
        vector3d_add(finalPosition, finalPosition, forward);
        vector3d_copy(e->position, finalPosition);
        vector3d_clear(finalPosition);
        vector3d_copy(e->rotation, self->rotation);
        vector3d_copy(e->modelOffset, self->modelOffset);
        vector3d_copy(e->scale, self->scale);

        e->hitboxes = gf3d_collision_armor_new(1);

        e->model = gf3d_model_new();
        e->model->texture = gf3d_texture_load("images/naruto.png");
        e->animationManager = gf3d_animation_manager_init(1, e->model);
        if(i == 1) gf3d_animation_load(e->animationManager, "punch", "naruto_punch_mirror", 1, 27);
        else gf3d_animation_load(e->animationManager, "punch", "naruto_punch", 1, 27);
        gf3d_animation_play(e->animationManager, "punch", 1);

        e->think = app_naruto_think;
        e->update = app_naruto_clone_update;
        e->touch = app_naruto_clone_touch;

        /* data for shadow clones will be the entity that created them */
        e->data = self; 
    }
}

void app_naruto_punch(struct Entity_S* e)
{
    // int punchCount = e->locked;
    float fcount = 0.0f;
    float currf  = 0.0f;

    if(!e) return;

    if( e->locked >= 100 )
    {
        if(e->locked == NARUTO_RASENSHURIKEN_LOCKED)
        {
            e->locked = 101;
            gf3d_animation_play(e->animationManager, "rasenshuriken", ANIM_NARUTO_RASENSHURIKEN_START);
            // app_naruto_rasenshuriken(e);
        }

        e->state &= ~ES_Idle;
        e->state |= ES_Attacking;
        e->velocity.x = e->velocity.y = 0.0f;
        return;
    }

    if(e->state & ES_Jumping)
    {

    }
    else if(e->state & ES_Idle)
    {

        if ( !gf3d_animation_is_playing(e->animationManager, "punch") )
        {
            /* start punching animation, set attacking flag, and clean velocity */
            gf3d_animation_play(e->animationManager, "punch", 1);
            e->state &= ~ES_Idle;
            e->state |= ES_Attacking;
            e->velocity.x = e->velocity.y = 0.0f;
            e->locked = 1;
        }
    }
    else if (e->state & ES_Attacking)
    {
        if ( gf3d_animation_is_playing(e->animationManager, "punch") )
        {
            fcount = gf3d_animation_get_frame_count(e->animationManager, "punch");
            currf = gf3d_animation_get_current_frame(e->animationManager);
            if ( (fcount - currf) * 3 <= 8.0f )
            {
                e->locked = 2;
            }
        }
        else if ( gf3d_animation_is_playing(e->animationManager, "kick") )
        {
            fcount = gf3d_animation_get_frame_count(e->animationManager, "kick");
            currf = gf3d_animation_get_current_frame(e->animationManager);
            if( fcount - currf <= 8.0f )
            {
                e->locked = 3;
            }
        }
    }
}

void app_naruto_think (struct Entity_S* self)
{
    float fcount = 0.0f, currf = 0.0f;
    float distanceToFloor = 0.0f;
    Uint32 onFloor = 0;

    if(!self->animationManager || !self->modelBox)
    {
        return;
    }

    distanceToFloor = distance_to_floor( self->modelBox->shapes[0].position.z - self->modelBox->shapes[0].extents.z );
    onFloor = on_floor( distanceToFloor );
    
    if ( gfc_line_cmp( self->animationManager->animationNames[ self->animationManager->currentAnimation ], "jump up" ) == 0 )
    {
        if( !(self->state & ES_Jumping) )
        {
            if( onFloor )
            {
                gf3d_animation_play(self->animationManager, "jump down", 1);
            }
        }

        if ( gf3d_animation_is_playing(self->animationManager, "jump up") )
        {
            if( gf3d_animation_get_frame_count(self->animationManager, "jump up") - gf3d_animation_get_current_frame(self->animationManager) <= 0.5f )
            {
                gf3d_animation_pause(self->animationManager, "jump up");
            }
        }
    }
    else if ( gfc_line_cmp( self->animationManager->animationNames[ self->animationManager->currentAnimation ], "jump down" ) == 0 )
    {
        if ( gf3d_animation_is_playing(self->animationManager, "jump down") )
        {
            if( gf3d_animation_get_frame_count(self->animationManager, "jump down") - gf3d_animation_get_current_frame(self->animationManager) <= 0.5f )
            {
                gf3d_animation_play(self->animationManager, "idle", 1);
            }
        }
    }
    else if ( gf3d_animation_is_playing(self->animationManager, "punch") )
    {
        fcount = gf3d_animation_get_frame_count(self->animationManager, "punch");
        currf = gf3d_animation_get_current_frame(self->animationManager);
        if(fcount - currf <= 0.5f)
        {
            gf3d_collision_armor_remove_shape(self->hitboxes, "punch");
            if(self->locked == 2)
            {
                gf3d_animation_play(self->animationManager, "kick", 1);
            }
            else
            {
                self->state &= ~ES_Attacking;
                self->state |= ES_Idle;
                gf3d_animation_play(self->animationManager, "idle", 1);
                self->locked = 0;
            }
            
        }
        else if ( currf * 3 <= NARUTO_PUNCH_ATK_FRAME )
        {
            if( !gf3d_collision_armor_contains(self->hitboxes, "punch") )
            {
                app_naruto_add_hitbox(self, "punch");
            }
        }
    }
    else if ( gf3d_animation_is_playing(self->animationManager, "kick") )
    {
        fcount = gf3d_animation_get_frame_count(self->animationManager, "kick");
        currf = gf3d_animation_get_current_frame(self->animationManager);
        if (fcount - currf <= 0.5f)
        {
            gf3d_collision_armor_remove_shape(self->hitboxes, "kick");
            if(self->locked == 3)
            {
                gf3d_animation_play(self->animationManager, "idle", 1);
                app_naruto_punch_create_shadow_clones(self);
            }
            else
            {
                self->state &= ~ES_Attacking;
                self->state |= ES_Idle;
                gf3d_animation_play(self->animationManager, "idle", 1);
                self->locked = 0;
            }
        }
        else if ( currf * 3 <= NARUTO_KICK_ATK_FRAME )
        {
            if( !gf3d_collision_armor_contains(self->hitboxes, "kick") )
            {
                app_naruto_add_hitbox(self, "kick");
            }
        }
    }
    else if ( self->locked == 3 && gf3d_animation_is_playing(self->animationManager, "idle") )
    {
        fcount = gf3d_animation_get_frame_count(self->animationManager, "idle");
        currf = gf3d_animation_get_current_frame(self->animationManager);
        if(fcount - currf <= 0.5f)
        {
            self->state &= ~ES_Attacking;
            self->state |= ES_Idle;
            self->locked = 0;
        }
    }
    else if ( gf3d_animation_is_playing(self->animationManager, "throw") )
    {
        fcount = gf3d_animation_get_frame_count(self->animationManager, "throw");
        currf = gf3d_animation_get_current_frame(self->animationManager);
        if(fcount - currf <= 0.5f)
        {
            self->state &= ~ES_Attacking;
            self->state |= ES_Idle;
            gf3d_animation_play(self->animationManager, "idle", 1);
            self->locked = 0;
        }
    }
    else if ( gf3d_animation_is_playing(self->animationManager, "rasenshuriken") )
    {
        fcount = gf3d_animation_get_frame_count(self->animationManager, "rasenshuriken");
        currf = gf3d_animation_get_current_frame(self->animationManager);
        if( fcount - currf <= 0.5f )
        {
            gf3d_common_init_state(self);
        }
        else if ( self->locked == NARUTO_RASENSHURIKEN_LOCKED + 1)
        {
            if( currf * 3 >= NARUTO_RASENSHURIKEN_ATK_FRAME )
            {
                app_naruto_rasenshuriken(self);
                self->locked++;
            }
        }
    }
}

void app_naruto_update(struct Entity_S* self)
{
    self->think(self);
    gf3d_entity_general_update(self);
    gfc_matrix_rotate(self->modelMat, self->modelMat, ( self->rotation.y + 90 ) * GFC_DEGTORAD, vector3d(1, 0, 0));
}

void app_naruto_touch (struct Entity_S* self, struct Entity_S* other)
{
    gf3d_combat_meele_attack(self, other, NARUTO_PUNCH_DMG, NARUTO_PUNCH_KICK);
}

void app_naruto_clone_update(struct Entity_S *e)
{
    Entity *owner;
    float fcount, currf;

    app_naruto_update(e);

    fcount = gf3d_animation_get_frame_count(e->animationManager, "punch");
    currf = gf3d_animation_get_current_frame(e->animationManager);

    if( fcount - currf < 0.5f)
    {
        gf3d_animation_manager_free(e->animationManager);
        gf3d_collision_armor_remove_shape(e->hitboxes, "punch");
        
        /* release entity from lock */
        owner = (Entity*)e->data;
        owner->state &= ~ES_Attacking;
        owner->state |= ES_Idle;
        owner->locked = 0;
        
        gf3d_entity_free(e);
    }
    else if ( currf * 3 <= NARUTO_PUNCH_ATK_FRAME )
    {
        if( !gf3d_collision_armor_contains(e->hitboxes, "punch") )
        {
            app_naruto_add_hitbox(e, "punch");
        }
    }
}

void app_naruto_clone_touch(struct Entity_S *self, struct Entity_S *other)
{
    if(self && other && (Entity*)self->data != other)
        gf3d_combat_meele_attack(self, other, NARUTO_PUNCH_DMG, NARUTO_CLONE_PUNCH_KICK);
}

void app_naruto_add_hitbox(struct Entity_S *ent, char *name)
{
    Vector3D forward;

    if(!ent) return;
    if(!ent->hitboxes) return;

    vector3d_angle_vectors(ent->rotation, &forward, NULL, NULL);

    if( gfc_line_cmp(name, "punch") == 0 )
    {
        gf3d_collision_armor_add_shape(
            ent->hitboxes,
            gf3d_shape(ent->position, vector3d(abs(forward.x * 2 + forward.y * 1.5), abs(forward.y * 2 + forward.x * 1.5), 1), gf3d_model_load("cube", "cube")),
            vector3d(0, 2.5, 0),
            "punch"
        );
    }
    else if ( gfc_line_cmp(name, "kick") == 0 )
    {
        gf3d_collision_armor_add_shape(
            ent->hitboxes,
            gf3d_shape(ent->position, vector3d(abs(forward.x * 3 + forward.y * 1.5), abs(forward.y * 3 + forward.x * 1.5), 1), gf3d_model_load("cube", "cube")),
            vector3d(0, 3, 0),
            "kick"
        );   
    }
    
}

/* ******************************
 * RASENSHURIKEN
 * ******************************/
void app_naruto_rasenshuriken(Entity *ent)
{
    Entity *proj = NULL;
    
    if(!ent) return;

    proj = gf3d_entity_new();
    if(!proj) return;

    vector3d_copy(proj->position, ent->position);
    proj->position.z += NARUTO_RASENSHURIKEN_UP_OFFSET;

    proj->model = gf3d_model_load("shuriken", "rasenshuriken");
    proj->hitboxes = gf3d_collision_armor_new(1);
    gf3d_collision_armor_add_shape(
        proj->hitboxes,
        gf3d_shape(proj->position, proj->scale, gf3d_model_load("cube", "cube")),
        vector3d(0, 0, 0),
        "body"
    );

    proj->update = app_naruto_rasenshuriken_update;
    proj->touch = app_naruto_rasenshuriken_touch;

    proj->data = ent;
}

void app_naruto_rasenshuriken_update(Entity *self)
{
    Entity *owner = NULL;
    if( self->chakra >= NARUTO_RASENSHURIKEN_TIME )
    {
        self->data = NULL;
        gf3d_entity_free(self);
        return;
    }

    self->rotation.x += worldTime * NARUTO_RASENSHURIKEN_ROT_SPEED;

    owner = (Entity*)self->data;
    if(owner && owner->enemy)
    {
        vector3d_sub(self->velocity, owner->enemy->position, self->position);
        vector3d_set_magnitude(&self->velocity, NARUTO_RASENSHURIKEN_SPEED);
    }

    gf3d_entity_general_update(self);
    gfc_matrix_rotate(self->modelMat, self->modelMat, self->rotation.y, vector3d(1, 0, 0));

    self->chakra += worldTime;
}

void app_naruto_rasenshuriken_touch(Entity *self, Entity *other)
{
    Entity *owner = NULL;
    Vector3D dir;
    
    if(!self || !other) return;

    owner = (Entity*)self->data;
    if(!owner || owner == other) return;

    vector3d_sub(dir, other->position, self->position);

    gf3d_combat_attack(owner, other, NARUTO_RASENSHURIKEN_DMG, NARUTO_RASENSHURIKEN_KICK, dir);
}