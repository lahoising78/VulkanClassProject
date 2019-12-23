#include "gf3d_ui.h"
#include "gf3d_vgraphics.h"
#include "simple_logger.h"

typedef struct 
{
    uint32_t count;
    uiLayer *layer_list;
    
    VkDevice device;
    Pipeline *pipe;
} uiManager;

static uiManager gf3d_ui_manager = {0};

void gf3d_ui_manager_init(uint32_t count)
{
    gf3d_ui_manager.layer_list = (uiLayer*)gfc_allocate_array(sizeof(uiLayer), count);
    if(!gf3d_ui_manager.layer_list) 
    {
        slog("WARNING: unable to initialize ui manager");
        return;
    }
    memset(gf3d_ui_manager.layer_list, 0, sizeof(uiLayer) * count);

    gf3d_ui_manager.count = count;

    gf3d_ui_manager.device = gf3d_vgraphics_get_default_logical_device();
    gf3d_ui_manager.pipe = gf3d_vgraphics_get_graphics_pipeline();
}

uiLayer *gf3d_ui_new()
{
    
}

void gf3d_ui_free(uiLayer *layer)
{

}