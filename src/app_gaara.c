#include "simple_logger.h"
#include "app_gaara.h"

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
    gf3d_animation_load(ent->animationManager, "running", "gaara_running", 1, 20);
    gf3d_animation_play(ent->animationManager, "running", 1);
    ent->modelOffset.z = -6.5f;
    ent->scale = vector3d(2, 2, 2);
    gfc_matrix_identity(ent->modelMat);

    return ent;
}

void app_gaara_input_handler(Player *self, SDL_Event *event)
{

}

void app_gaara_think(Entity *self)
{

}

void app_gaara_update(Entity *self)
{
    if(self->think) self->think(self);
    gf3d_entity_general_update(self);
    gfc_matrix_rotate(self->modelMat, self->modelMat, (self->rotation.y + 90) * GFC_DEGTORAD, vector3d(1, 0, 0));
}

void app_gaara_touch(Entity *self)
{

}