#include "gf3d_ui.h"
#include "simple_logger.h"


#define _GF3D_UI_MANAGER_INDEX_COUNT_ 6

typedef struct
{
    Vector2D        pos;
    Vector2D        ext;
    Vector2D        texCoord;
} UIVertex;


typedef struct
{
    uint32_t                    count;
    UILayer                     *layer_list;
    
    VkDevice                    device;
    Pipeline                    *pipe;
    
    int                         indices[_GF3D_UI_MANAGER_INDEX_COUNT_];
    VkBuffer                    indexBuffer;
    VkDeviceMemory              indexBufferMemory;

    VkDeviceSize                bufferSize;
} UIManager;

static UIManager ui_manager = {0};

void gf3d_ui_manager_close()
{
    int i;
    UILayer *layer = NULL;

    slog("ui manager close");

    if(ui_manager.layer_list)
    {
        for(i = 0; i < ui_manager.count; i++)
        {
            layer = &ui_manager.layer_list[i];
            gf3d_ui_layer_free(layer);
        }
        free(ui_manager.layer_list);
    }

    vkDestroyBuffer(ui_manager.device, ui_manager.indexBuffer, NULL);
    vkFreeMemory(ui_manager.device, ui_manager.indexBufferMemory, NULL);
}

void gf3d_ui_manager_init(uint32_t count)
{
    VkDeviceSize bufferSize = sizeof(int) * 6;
    VkBuffer    stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    void *data;

    slog("ui manager init");

    ui_manager.device = gf3d_vgraphics_get_default_logical_device();
    ui_manager.pipe = gf3d_vgraphics_get_graphics_pipeline2D();

    ui_manager.layer_list = (UILayer*) gfc_allocate_array(sizeof(UILayer), count);
    if(!ui_manager.layer_list) 
    {
        slog("unable to allocate layers");
        return;
    }
    memset(ui_manager.layer_list, 0, sizeof(UILayer) * count);
    ui_manager.count = count;

    ui_manager.bufferSize = sizeof(UIVertex) * 4;
    ui_manager.indices[0] = 0;
    ui_manager.indices[1] = 1;
    ui_manager.indices[2] = 2;
    ui_manager.indices[3] = 2;
    ui_manager.indices[4] = 3;
    ui_manager.indices[5] = 0;

    gf3d_vgraphics_create_buffer(
        bufferSize,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory
    );

    vkMapMemory(ui_manager.device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, ui_manager.indices, bufferSize);
    vkUnmapMemory(ui_manager.device, stagingBufferMemory);

    gf3d_vgraphics_create_buffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &ui_manager.indexBuffer, &ui_manager.indexBufferMemory
    );

    gf3d_vgraphics_copy_buffer(stagingBuffer, ui_manager.indexBuffer, bufferSize);

    vkDestroyBuffer(ui_manager.device, stagingBuffer, NULL);
    vkFreeMemory(ui_manager.device, stagingBufferMemory, NULL);

    atexit(gf3d_ui_manager_close);
}

UILayer *gf3d_ui_layer_new()
{
    int i;
    UILayer *layer = NULL;

    slog("ui new");

    for(i = 0; i < ui_manager.count; i++)
    {
        layer = &ui_manager.layer_list[i];
        if(layer->_inuse) continue;

        layer->_inuse = 1;
        layer->visible = 1;
        layer->active = 1;

        return layer;
    }

    return NULL;
}

void gf3d_ui_layer_free(UILayer *layer)
{
    slog("ui free");

    gf3d_texture_free(layer->texture);

    SDL_DestroyRenderer(layer->renderer);

    SDL_FreeSurface(layer->surface);
}