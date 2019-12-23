#include "gf3d_ui.h"
#include "gf3d_vgraphics.h"
#include "simple_logger.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    static const uint32_t rmask = 0xff000000;
    static const uint32_t gmask = 0x00ff0000;
    static const uint32_t bmask = 0x0000ff00;
    static const uint32_t amask = 0x000000ff;
#else
    static const uint32_t rmask = 0x000000ff;
    static const uint32_t gmask = 0x0000ff00;
    static const uint32_t bmask = 0x00ff0000;
    static const uint32_t amask = 0xff000000;
#endif

typedef struct 
{
    uint32_t count;
    uiLayer *layer_list;
    
    VkExtent2D extents;

    VkDevice device;
    Pipeline *pipe;
} uiManager;

static uiManager gf3d_ui_manager = {0};

void gf3d_ui_manager_close()
{
    int i;
    uiLayer *ui = NULL;

    for(i = 0; i < gf3d_ui_manager.count; i++)
    {
        ui = &gf3d_ui_manager.layer_list[i];
        gf3d_ui_free(ui);
        
        vkDestroyBuffer(gf3d_ui_manager.device, ui->stagingBuffer, NULL);
        vkFreeMemory(gf3d_ui_manager.device, ui->stagingBufferMemory, NULL);
        
        if(ui->renderer) SDL_DestroyRenderer(ui->renderer);
        ui->renderer = NULL;
        if(ui->surface) SDL_FreeSurface(ui->surface);
        ui->surface = NULL;
    }

    free(gf3d_ui_manager.layer_list);
}

void gf3d_ui_manager_init(uint32_t count)
{
    int i;
    uiLayer *ui = NULL;
    VkDeviceSize imageSize;

    slog("ui manager init");

    if(gf3d_ui_manager.layer_list) return;

    gf3d_ui_manager.layer_list = (uiLayer*)gfc_allocate_array(sizeof(uiLayer), count);
    if(!gf3d_ui_manager.layer_list) 
    {
        slog("WARNING: unable to initialize ui manager");
        return;
    }
    memset(gf3d_ui_manager.layer_list, 0, sizeof(uiLayer) * count);

    gf3d_ui_manager.extents = gf3d_vgraphics_get_view_extent();
    for(i = 0; i < count; i++)
    {
        void *data;
        ui =  &gf3d_ui_manager.layer_list[i];
        ui->surface = SDL_CreateRGBSurface(
            0,
            gf3d_ui_manager.extents.width, gf3d_ui_manager.extents.height,
            32,
            rmask, gmask, bmask, amask
        );
        if(!ui->surface) continue;

        ui->renderer = SDL_CreateSoftwareRenderer(ui->surface);
        if(!ui->renderer) continue;

        // imageSize = ui->surface->w * ui->surface->h * 4;
        // gf3d_vgraphics_create_buffer(
        //     imageSize,
        //     VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        //     &ui->stagingBuffer, &ui->stagingBufferMemory
        // );

        // SDL_LockSurface(ui->surface);
        //     vkMapMemory(gf3d_ui_manager.device, ui->stagingBufferMemory, 0, imageSize, 0, &data);
        //         memcpy(data, ui->surface->pixels, imageSize);
        //     vkUnmapMemory(gf3d_ui_manager.device, ui->stagingBufferMemory);
        // SDL_UnlockSurface(ui->surface);
    }

    gf3d_ui_manager.count = count;
    atexit(gf3d_ui_manager_close);

    gf3d_ui_manager.device = gf3d_vgraphics_get_default_logical_device();
    gf3d_ui_manager.pipe = gf3d_vgraphics_get_graphics_pipeline();

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
        return ui;
    }

    return NULL;
}

void gf3d_ui_free(uiLayer *layer)
{
    if(!layer) return;

    layer->_inuse = 0;
}