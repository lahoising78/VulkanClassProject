#include "gf3d_gui.h"

#include "simple_logger.h"
#include "gf3d_vgraphics.h"

// extern float worldTime;
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

GuiElement *gf3d_gui_new()
{
    int i;
    GuiElement *gui = NULL;

    for(i = 0; i < gf3d_gui_manager.max_count; i++)
    {
        gui = &gf3d_gui_manager.elements[i];
        if(gui->_inuse) continue;
        return gui;
    }

    return NULL;
}

void gf3d_gui_free(GuiElement *gui)
{
    if(gui->shape.model) gf3d_model_free(gui->shape.model);
    memset(gui, 0, sizeof(GuiElement));
}