#include "gf3d_gui.h"

#include "simple_logger.h"
#include "gf3d_vgraphics.h"
#include "gf3d_camera.h"

void gf3d_gui_update(GuiElement *gui);
void gf3d_gui_draw(GuiElement *gui, Uint32 bufferFrame, VkCommandBuffer commandBuffer);

typedef struct GuiManager_S
{
    Uint32 max_count;
    GuiElement *elements;
} GuiManager;

static GuiManager gf3d_gui_manager = {0};

void gf3d_gui_manager_close()
{
    int i;

    for(i = 0; i < gf3d_gui_manager.max_count; i++)
    {
        gf3d_gui_free( &gf3d_gui_manager.elements[i] );
    }

    free(gf3d_gui_manager.elements);
    memset(&gf3d_gui_manager, 0, sizeof(GuiManager));
}

int gf3d_gui_manager_init(Uint32 count)
{
    int i;
    GuiElement *gui = NULL;

    gf3d_gui_manager.elements = (GuiElement*)gfc_allocate_array(sizeof(GuiElement), count);
    if(!gf3d_gui_manager.elements)
    {
        slog("failed to initialize gui manager");
        return 1;
    }

    for(i = 0; i < count; i++)
    {
        memset(&gf3d_gui_manager.elements[i], 0, sizeof(GuiElement));
        // gui = &gf3d_gui_manager.elements[i];
    }

    gf3d_gui_manager.max_count = count;
    atexit(gf3d_gui_manager_close);
    return 0;
}

void gf3d_gui_manager_update()
{
    int i;
    GuiElement *gui = NULL;
    for(i = 0; i < gf3d_gui_manager.max_count; i++)
    {
        gui = &gf3d_gui_manager.elements[i];
        if( !gui->_inuse ) continue;

        gf3d_gui_update(gui);
    }
}

void gf3d_gui_manager_draw(Uint32 bufferFrame, VkCommandBuffer commandBuffer)
{
    int i;
    GuiElement *gui = NULL;

    for(i = 0; i < gf3d_gui_manager.max_count; i++)
    {
        gui = &gf3d_gui_manager.elements[i];
        if( !gui->_inuse ) continue;

        gf3d_gui_draw(gui, bufferFrame, commandBuffer);
    }
}

GuiElement *gf3d_gui_new()
{
    int i;
    GuiElement *gui = NULL;

    for(i = 0; i < gf3d_gui_manager.max_count; i++)
    {
        gui = &gf3d_gui_manager.elements[i];
        if(gui->_inuse) continue;
        gui->_inuse = 1;
        return gui;
    }

    return NULL;
}

void gf3d_gui_free(GuiElement *gui)
{
    if(!gui) return;
    memset(gui, 0, sizeof(GuiElement));
}

void gf3d_gui_update(GuiElement *gui)
{
    gf3d_camera_get_angles( &gui->shape.position, NULL, NULL);
    vector3d_add(gui->shape.position, gui->shape.position, gf3d_camera_get_position());
    vector3d_add(gui->shape.position, gui->shape.position, gui->offset);
    if(gui->val && gui->max)
    {
        gui->shape.extents.x = gui->size.x * (*gui->val / *gui->max);
    }
    gf3d_shape_update_mat(&gui->shape);
}

void gf3d_gui_draw(GuiElement *gui, Uint32 bufferFrame, VkCommandBuffer commandBuffer)
{
    if(!gui) return;
    if(!gui->shape.model) return;
    gf3d_model_draw(gui->shape.model, bufferFrame, commandBuffer, gui->shape.matrix, 0);
}