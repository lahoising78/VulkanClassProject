#include "app_editor_entity.h"

#include "simple_logger.h"

typedef struct
{
    uint32_t count;
    EditorEntity *entity_list;
} EditorEntityManager;

static EditorEntityManager app_editor_entity_manager = {0};

void app_editor_entity_update(EditorEntity *e, SDL_Event *keys, SDL_Event *mouse);

void app_editor_entity_manager_close()
{
    int i;
    EditorEntity *e = NULL;
    if(!app_editor_entity_manager.entity_list) return;
    for(i = 0; i < app_editor_entity_manager.count; i++)
    {
        e = &app_editor_entity_manager.entity_list[i];
        app_editor_entity_free(e);
    }
    free(app_editor_entity_manager.entity_list);
    app_editor_entity_manager.entity_list = NULL;
    app_editor_entity_manager.count = 0;
}

void app_editor_entity_manager_init(uint32_t count)
{
    app_editor_entity_manager.entity_list = (EditorEntity*)gfc_allocate_array(sizeof(EditorEntity), count);
    if(!app_editor_entity_manager.entity_list)
    {
        slog("unable to init editor manager");
        return;
    }
    memset(app_editor_entity_manager.entity_list, 0, sizeof(EditorEntity) * count);
    app_editor_entity_manager.count = count;
    atexit(app_editor_entity_manager_close);
}

void app_editor_entity_manager_update(SDL_Event *keys, SDL_Event *mouse)
{
    int i;
    EditorEntity *e = NULL;
    for(i = 0; i < app_editor_entity_manager.count; i++)
    {
        e = &app_editor_entity_manager.entity_list[i];
        if(!e->_inuse) continue;
        app_editor_entity_update(e, keys, mouse);
    }
}

void app_editor_entity_manager_draw(uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    int i;
    EditorEntity *e = NULL;
    for(i = 0; i < app_editor_entity_manager.count; i++)
    {
        e = &app_editor_entity_manager.entity_list[i];
        if(!e->_inuse) continue;
        gf3d_hud_element_draw(&e->ent, bufferFrame, commandBuffer);
    }
}

EditorEntity *app_editor_entity_create()
{
    int i;
    EditorEntity *e;
    for(i = 0; i < app_editor_entity_manager.count; i++)
    {
        if(app_editor_entity_manager.entity_list[i]._inuse) continue;
        e = &app_editor_entity_manager.entity_list[i];
        e->_inuse = 1;
        e->pos = vector2d(0.0f, 0.0f);
        e->ext = vector2d(50.0f, 50.0f);
        e->ent.type = GF3D_HUD_TYPE_GUI_ELEMENT;
        e->ent.element.guiElement = gf3d_gui_element_create(
            e->pos,
            e->ext,
            vector4d(255.0f, 255.0f, 255.0f, 255.0f)
        );
        return e;
    }
    return NULL;
}

void app_editor_entity_update(EditorEntity *e, SDL_Event *keys, SDL_Event *mouse)
{
    Vector2D pos;
    if(!e) return;
    // if(!e->parent)
    // {
    //     gf3d_hud_element_set_position(e->ent, e->pos);
    //     gf3d_hud_element_set_extents(e->ent, e->ext);
    // }
    // else
    // {
    //     vector2d_copy(pos, e->pos);
    //     vector2d_add(e->pos, e->pos, e->parent->bg->position);
    // }
    
    // gf3d_hud_element_update(&e->ent, keys, mouse);

    // if(e->parent)
    // {
    //     vector2d_copy(e->pos, pos);
    // }
}

void app_editor_entity_free(EditorEntity *e)
{
    if(!e) return;
    gf3d_hud_element_free(&e->ent);
    memset(e, 0, sizeof(EditorEntity));
}