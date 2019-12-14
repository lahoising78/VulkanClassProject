#ifndef _APP_EDITOR_ENTITY_H_
#define _APP_EDITOR_ENTITY_H_

#include "gf3d_gui.h"

typedef struct
{
    HudElement ent;
    Vector2D pos;
    Vector2D ext;
    Window *parent;
    uint8_t _inuse;
    uint8_t dragging;
} EditorEntity;

extern float worldTime;

void app_editor_entity_manager_init(uint32_t count);
void app_editor_entity_manager_clean();
void app_editor_entity_manager_update(SDL_Event *keys, SDL_Event *mouse);
void app_editor_entity_manager_draw(uint32_t bufferFrame, VkCommandBuffer commandBuffer);

EditorEntity *app_editor_entity_manager_get_selected();

EditorEntity *app_editor_entity_create();
void app_editor_entity_free(EditorEntity *e);

void app_editor_entity_fix_pos(EditorEntity *e);

#endif