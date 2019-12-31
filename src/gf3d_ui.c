#include "gf3d_ui.h"
#include "simple_logger.h"

#define _GF3D_UI_MANAGER_INDEX_COUNT_ 6

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    static Uint32 rmask = 0xff000000;
    static Uint32 gmask = 0x00ff0000;
    static Uint32 bmask = 0x0000ff00;
    static Uint32 amask = 0x000000ff;
#else
    static Uint32 rmask = 0x000000ff;
    static Uint32 gmask = 0x0000ff00;
    static Uint32 bmask = 0x00ff0000;
    static Uint32 amask = 0xff000000;
#endif

typedef struct ui_vertex_t
{
    Vector2D        pos;
    Vector4D        color;
    Vector2D        texCoord;
} UIVertex;

typedef struct
{
    uint32_t                    count;
    UILayer                     *layer_list;
    
    VkDevice                    device;
    Pipeline                    *pipe;
    
    uint16_t                         indices[_GF3D_UI_MANAGER_INDEX_COUNT_];
    VkBuffer                    indexBuffer;
    VkDeviceMemory              indexBufferMemory;

    VkDeviceSize                bufferSize;
} UIManager;

static UIManager ui_manager = {0};

void gf3d_ui_manager_init_layers();

void gf3d_ui_render(UILayer *layer, uint32_t bufferFrame, VkCommandBuffer commandBuffer);

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
    gf3d_ui_manager_init_layers();

    ui_manager.bufferSize = sizeof(uint16_t) * 6;
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

void gf3d_ui_manager_init_layers()
{
    int i;
    UILayer *layer = NULL;
    VkExtent2D extents = gf3d_vgraphics_get_view_extent();
    SDL_Rect rect;
    VkDeviceSize imageSize;
    void *data;
    UIVertex vertices[] = 
    {
        {
            {-1, -1},
            {255, 255, 255, 255},
            {0, 0}
        },
        {
            {1, -1},
            {255, 255, 255, 255},
            {1, 0}
        },
        {
            {1, 1},
            {255, 255, 255, 255},
            {1, 1}
        },
        {
            {-1, 1},
            {255, 255, 255, 255},
            {0, 1}
        }
    };

    for(i = 0; i < ui_manager.count; i++)
    {
        layer = &ui_manager.layer_list[i];

        layer->surface = SDL_CreateRGBSurface(
            0,
            extents.width, extents.height, 
            32,
            rmask, gmask, bmask, amask
        );

        layer->renderer = SDL_CreateSoftwareRenderer(layer->surface);

        rect.x = rect.y = rect.w = rect.h = 100;
        SDL_SetRenderDrawColor(layer->renderer, 255, 255, 255, 0);
        SDL_RenderClear(layer->renderer);
        SDL_SetRenderDrawColor(layer->renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(layer->renderer, &rect);

        imageSize = sizeof(UIVertex) * 4;
        gf3d_vgraphics_create_buffer(
            imageSize, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            &layer->stagingBuffer, &layer->stagingBufferMemory
        );

        vkMapMemory(ui_manager.device, layer->stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, vertices, imageSize);
        vkUnmapMemory(ui_manager.device, layer->stagingBufferMemory);

        gf3d_vgraphics_create_buffer(
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &layer->vertexBuffer, &layer->vertexBufferMemory
        );
    
        gf3d_vgraphics_copy_buffer(layer->stagingBuffer, layer->vertexBuffer, imageSize);

        layer->texture = gf3d_texture_from_surface( NULL, layer->surface );
    }
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

void gf3d_ui_manger_render(uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    int i; 
    UILayer *layer = NULL;

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ui_manager.pipe->pipeline);

    for(i = 0; i < ui_manager.count; i++)
    {
        layer = &ui_manager.layer_list[i];
        if(!layer->_inuse || !layer->visible) continue;

        gf3d_ui_render(layer, bufferFrame, commandBuffer);
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gf3d_vgraphics_get_graphics_pipeline()->pipeline);
}

void gf3d_ui_layer_free(UILayer *layer)
{
    slog("ui free");

    vkDestroyBuffer(ui_manager.device, layer->stagingBuffer, NULL);
    vkFreeMemory(ui_manager.device, layer->stagingBufferMemory, NULL);
    vkDestroyBuffer(ui_manager.device, layer->vertexBuffer, NULL);
    vkFreeMemory(ui_manager.device, layer->vertexBufferMemory, NULL);

    gf3d_texture_free(layer->texture);

    SDL_DestroyRenderer(layer->renderer);

    SDL_FreeSurface(layer->surface);
}

void gf3d_ui_update_descriptor_set(UILayer *layer, const VkDescriptorSet *descriptorSet)
{
    VkDescriptorImageInfo imageInfo = {0};
    VkWriteDescriptorSet descriptorWrite[1] = {0};
    // const VkDescriptorSet *descriptorSet = gf3d_pipeline_get_descriptor_set(gf3d_gui_element_manager.pipe, bufferFrame);

    if(!layer)
    {
        slog("element is null");
        return;
    }
    if(!layer->texture)
    {
        slog("texture is null");
        return;
    }

    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = layer->texture->textureImageView;
    imageInfo.sampler = layer->texture->textureSampler;

    descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[0].dstSet = *descriptorSet;
    descriptorWrite[0].dstBinding = 1;
    descriptorWrite[0].dstArrayElement = 0;
    descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite[0].descriptorCount = 1;
    descriptorWrite[0].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(ui_manager.device, 1, descriptorWrite, 0, NULL);
}

void gf3d_ui_render(UILayer *layer, uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    VkBuffer vertexBuffers[] = {layer->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    const VkDescriptorSet *descriptorSet = gf3d_pipeline_get_descriptor_set(ui_manager.pipe, bufferFrame);

    if(!layer) return;

    gf3d_ui_update_descriptor_set(layer, descriptorSet);

    vkCmdBindVertexBuffers(commandBuffer, 1, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, ui_manager.indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdBindDescriptorSets(
        commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        ui_manager.pipe->pipelineLayout,
        0, 1, descriptorSet,
        0, NULL
    );
    vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
}