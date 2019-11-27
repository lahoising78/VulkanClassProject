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
    // int i;
    // EditorEntity *e = NULL;
    // for(i = 0; i < app_editor_entity_manager.count; i++)
    // {
    //     e = &app_editor_entity_manager.entity_list[i];
    //     if(!e->_inuse) continue;
    //     gf3d_hud_element_draw(&e->ent, bufferFrame, commandBuffer);
    // }
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
        return e;
    }
    return NULL;
}

void app_editor_entity_update(EditorEntity *e, SDL_Event *keys, SDL_Event *mouse)
{
    SDL_Event evt = {0};
    SDL_Point p = {0};
    SDL_Rect r = {0};
    Vector2D pos = {0};
    
    if(!e) return;
    
    evt = keys[SDL_SCANCODE_DELETE];
    if( evt.key.type == SDL_KEYDOWN && e->selected )
    {
        app_editor_entity_free(e);
        return;
    }

    evt = mouse[SDL_BUTTON_LEFT];
    if( evt.button.type == SDL_MOUSEBUTTONDOWN )
    {
        vector2d_copy( p, evt.button );
        vector2d_add(pos, e->pos, e->parent->bg->position);
        vector2d_copy(r, pos);
        r.w = e->ext.x, r.h = e->ext.y;
        if( SDL_PointInRect(&p, &r) )
        {
            e->selected = 1;
            e->dragging = 1;
        }



    }
    else if (evt.button.type == SDL_MOUSEBUTTONUP)
    {
        e->dragging = 0;
    }

    /* motion is recorded in the button being pressed */
    if(e->dragging && evt.type == SDL_MOUSEMOTION)
    {
        e->pos.x += evt.motion.xrel;
        e->pos.y += evt.motion.yrel;
        if(e->pos.x < 0.0f) e->pos.x = 0.0f;
        else if (e->pos.x + e->ext.x > e->parent->bg->extents.x) e->pos.x = e->parent->bg->extents.x - e->ext.x;
        if(e->pos.y < 0.0f) e->pos.y = 0.0f;
        else if (e->pos.y + e->ext.y > e->parent->bg->extents.y) e->pos.y = e->parent->bg->extents.y - e->ext.y;
        gf3d_hud_element_set_position(e->ent, e->pos);
    }
}

void app_editor_entity_free(EditorEntity *e)
{
    if(!e) return;
    if(e->parent) 
    {
        gf3d_hud_window_remove_element(e->parent, e->ent);
    }

    memset(e, 0, sizeof(EditorEntity));
}