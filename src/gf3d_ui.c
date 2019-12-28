#include "gf3d_ui.h"
#include "gf3d_vgraphics.h"
#include "simple_logger.h"

// #if SDL_BYTEORDER == SDL_BIG_ENDIAN
//     static const uint32_t rmask = 0xff000000;
//     static const uint32_t gmask = 0x00ff0000;
//     static const uint32_t bmask = 0x0000ff00;
//     static const uint32_t amask = 0x000000ff;
// #else
//     static const uint32_t rmask = 0x000000ff;
//     static const uint32_t gmask = 0x0000ff00;
//     static const uint32_t bmask = 0x00ff0000;
//     static const uint32_t amask = 0xff000000;
// #endif

typedef struct 
{
    uint32_t count;
    uiLayer *layer_list;

    uint32_t component_list_count;
    
    VkExtent2D extents;

    VkDevice device;
    Pipeline *pipe;
} uiManager;

static uiManager gf3d_ui_manager = {0};

void gf3d_ui_draw( uiLayer *layer, uint32_t bufferFrame, VkCommandBuffer commandBuffer );

void gf3d_ui_manager_close()
{
    int i;
    uiLayer *ui = NULL;

    for(i = 0; i < gf3d_ui_manager.count; i++)
    {
        ui = &gf3d_ui_manager.layer_list[i];
        gf3d_ui_free(ui);
        if(ui->components) free(ui->components);
        ui->components = NULL;
    }

    free(gf3d_ui_manager.layer_list);
}

void gf3d_ui_manager_init(uint32_t count)
{
    slog("ui manager init");

    if(gf3d_ui_manager.layer_list) return;

    gf3d_ui_manager.layer_list = (uiLayer*)gfc_allocate_array(sizeof(uiLayer), count);
    if(!gf3d_ui_manager.layer_list) 
    {
        slog("WARNING: unable to initialize ui manager");
        return;
    }
    memset(gf3d_ui_manager.layer_list, 0, sizeof(uiLayer) * count);

    gf3d_ui_manager.count = count;
    atexit(gf3d_ui_manager_close);

    gf3d_ui_manager.component_list_count = 128;
    gf3d_ui_manager.extents = gf3d_vgraphics_get_view_extent();
    gf3d_ui_manager.device = gf3d_vgraphics_get_default_logical_device();
    gf3d_ui_manager.pipe = gf3d_vgraphics_get_graphics_pipeline();
    
    gf3d_ui_component_manager_init();
}

void gf3d_ui_manager_draw( uint32_t bufferFrame, VkCommandBuffer commandBuffer )
{
    int i;
    uiLayer *layer = NULL;

    // slog("draw ui manager");

    for(i = 0; i < gf3d_ui_manager.count; i++)
    {
        layer = &gf3d_ui_manager.layer_list[i];
        if(!layer->_inuse || !layer->visible) continue;

        gf3d_ui_draw(layer, bufferFrame, commandBuffer);
    }
}

uiLayer *gf3d_ui_new()
{
    int i;
    uiLayer *ui = NULL;

    for(i = 0; i < gf3d_ui_manager.count; i++)
    {
        ui = &gf3d_ui_manager.layer_list[i];
        if(ui->_inuse) continue;

        ui->_inuse = 1;
        ui->active = 1;
        ui->visible = 1;
        ui->count = gf3d_ui_manager.component_list_count;

        if(!ui->components)
        {
            ui->components = (uiComponent*)gfc_allocate_array(sizeof(uiComponent), ui->count);
        }

        return ui;
    }

    return NULL;
}

uiComponent *gf3d_ui_get_component(uiLayer *layer)
{
    uint32_t i;
    uiComponent *ui = NULL;
    if(!layer || !layer->components) return NULL;

    for(i = 0; i < layer->count; i++)
    {
        ui = &layer->components[i];
        if(ui->_inuse) continue;
        gf3d_ui_component_init(ui);
        return ui;
    }

    return NULL;
}

void gf3d_ui_free(uiLayer *layer)
{
    int i;
    uiComponent *components = NULL;

    slog("free ui layer");
    if(!layer) return;

    components = layer->components;
    if(components)
    {
        for(i = 0; i < layer->count; i++)
        {
            gf3d_ui_component_free(&components[i]);
        }
    }

    memset(layer, 0, sizeof(uiLayer));

    layer->components = components;
}

void gf3d_ui_draw( uiLayer *layer, uint32_t bufferFrame, VkCommandBuffer commandBuffer )
{
    int i;
    uiComponent *ui = NULL;

    // slog("draw ui layer");

    for(i = 0; i < layer->count; i++)
    {
        ui = &layer->components[i];
        if(!ui->_inuse || !ui->visible) continue;

        gf3d_ui_component_render(ui, bufferFrame, commandBuffer);
    }
}