#include "gf3d_ui_layer.h"
#include "simple_logger.h"

#include <SDL2/SDL.h>
#include "gf3d_vgraphics.h"
#include "gf3d_gui_element.h"

// #if SDL_BYTEORDER == SDL_BIG_ENDIAN
//     static Uint32 rmask = 0xff000000;
//     static Uint32 gmask = 0x00ff0000;
//     static Uint32 bmask = 0x0000ff00;
//     static Uint32 amask = 0x000000ff;
// #else
//     static Uint32 rmask = 0x000000ff;
//     static Uint32 gmask = 0x0000ff00;
//     static Uint32 bmask = 0x00ff0000;
//     static Uint32 amask = 0xff000000;
// #endif

#define UI_MANAGER_INDEX_COUNT 6
#define UI_MANAGER_VERTEX_COUNT 4

typedef struct
{
    UILayer                 *layer_list;
    uint32_t                count;

    GuiElement              *display;

    Pipeline                *pipe;
    VkDevice                device;
} UIManager;

static UIManager gf3d_ui_manager = {0};

// void gf3d_ui_manager_create_index_buffer();
// void gf3d_ui_manager_create_vertex_buffer();
void gf3d_ui_manager_create_display();
void gf3d_ui_manager_initialize_list();

void gf3d_ui_draw(UILayer *layer, uint32_t bufferFrame, VkCommandBuffer commandBuffer);

void gf3d_ui_manager_close()
{
    int i;

    slog("close ui system");

    if(gf3d_ui_manager.layer_list) 
    {
        for(i = 0; i < gf3d_ui_manager.count; i++)
        {
            /* TODO: free each layer */
        }

        free(gf3d_ui_manager.layer_list);
        gf3d_ui_manager.layer_list = NULL;
    }

    gf3d_ui_manager.count = 0;
    gf3d_gui_element_free(gf3d_ui_manager.display);

    // gf3d_texture_free(gf3d_ui_manager.texture);
    // vkDestroyBuffer(gf3d_ui_manager.device, gf3d_ui_manager.indexBuffer, NULL);
    // vkFreeMemory(gf3d_ui_manager.device, gf3d_ui_manager.indexBufferMemory, NULL);

}

// void gf3d_ui_manager_create_index_buffer()
// {
//     void *data;
//     VkBuffer stagingBuffer;
//     VkDeviceMemory stagingBufferMemory;
//     VkDeviceSize buffsize;

//     buffsize = sizeof(uint16_t) * 6;

//     gf3d_vgraphics_create_buffer(
//         buffsize,
//         VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
//         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//         &stagingBuffer, &stagingBufferMemory
//     );

//     vkMapMemory(gf3d_ui_manager.device, stagingBufferMemory, 0, buffsize, 0, &data);
//         memcpy(data, gf3d_ui_manager.indices, buffsize);
//     vkUnmapMemory(gf3d_ui_manager.device, stagingBufferMemory);

//     gf3d_vgraphics_create_buffer(
//         buffsize,
//         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
//         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//         &gf3d_ui_manager.indexBuffer, &gf3d_ui_manager.indexBufferMemory
//     );

//     gf3d_vgraphics_copy_buffer(stagingBuffer, gf3d_ui_manager.indexBuffer, buffsize);

//     vkDestroyBuffer(gf3d_ui_manager.device, stagingBuffer, NULL);
//     vkFreeMemory(gf3d_ui_manager.device, stagingBufferMemory, NULL);
// }

void gf3d_ui_manager_initialize_list()
{
    int i;
    UILayer *layer = NULL;
    SDL_Surface *surface = NULL;
    SDL_Renderer *renderer = NULL;
    VkExtent2D ext = gf3d_vgraphics_get_view_extent();
    SDL_Rect rect = {0};

    slog("ui system init list");

    rect.x = rect.y = rect.h = rect.w = 100;
    
    surface = SDL_CreateRGBSurface(
        0,
        ext.width, ext.height,
        32,
        rmask, gmask, bmask, amask
    );

    renderer = SDL_CreateSoftwareRenderer(surface);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);

    for(i = 0; i < gf3d_ui_manager.count; i++)
    {
        layer = &gf3d_ui_manager.layer_list[i];
        memset(layer, 0, sizeof(UILayer));

        layer->display = gf3d_texture_from_surface(layer->display, surface);
        snprintf(layer->display->filename, GFCLINELEN, "ui_layer_%d", i);

        layer->visible = layer->active = layer->_inuse = 1;
        slog("inuse %d, visible %d, texture %d", layer->_inuse, layer->visible, layer->display);
    }

    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
}

void gf3d_ui_manager_init(uint32_t count)
{
    slog("initialize ui system");

    gf3d_ui_manager.device = gf3d_vgraphics_get_default_logical_device();
    gf3d_ui_manager.pipe = gf3d_vgraphics_get_graphics_pipeline2D();
    gf3d_ui_manager_create_display();

    gf3d_ui_manager.layer_list = (UILayer*)gfc_allocate_array(sizeof(UILayer), count);
    if(!gf3d_ui_manager.layer_list)
    {
        slog("unable to initialize ui layers");
        return;
    }
    gf3d_ui_manager.count = count;

    gf3d_ui_manager_initialize_list();
    
    atexit(gf3d_ui_manager_close);
}

void gf3d_ui_manager_create_display()
{
    SDL_Surface *surface = NULL;
    SDL_Renderer *renderer = NULL;

    VkExtent2D ext = {0};
    ext = gf3d_vgraphics_get_view_extent();

    surface = SDL_CreateRGBSurface(
        0,
        ext.width, ext.height,
        32,
        rmask, gmask, bmask, amask
    );

    renderer = SDL_CreateSoftwareRenderer(surface);
    SDL_SetRenderDrawColor(renderer, 0, 0, 210, 255);
    SDL_RenderClear(renderer);

    gf3d_ui_manager.display =  gf3d_gui_element_create(vector2d(0,0), vector2d((float)ext.width, (float)ext.height), vector4d(255, 255, 255, 255));
    gf3d_gui_element_attach_texture_from_surface(gf3d_ui_manager.display, surface);
    gfc_line_cpy(gf3d_ui_manager.display->tex->filename, "ui_manager_texture");
    // gf3d_gui_element_attach_texture(gf3d_ui_manager.display, "images/bg_flat.png");

    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
}

void gf3d_ui_manager_draw(uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    int i;
    UILayer *layer = NULL;
    
    for(i = 0; i < gf3d_ui_manager.count; i++)
    {
        layer = &gf3d_ui_manager.layer_list[i];
        if(!layer->_inuse || !layer->visible) continue;

        gf3d_ui_draw(layer, bufferFrame, commandBuffer);
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gf3d_ui_manager.pipe->pipeline);
    gf3d_gui_element_update(gf3d_ui_manager.display);
    gf3d_gui_element_draw(gf3d_ui_manager.display, bufferFrame, commandBuffer);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gf3d_vgraphics_get_graphics_pipeline()->pipeline);
}

void gf3d_ui_draw(UILayer *layer, uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    VkImageBlit imageBlit;
    VkExtent2D view = gf3d_vgraphics_get_view_extent();

    if(!layer) return;

    imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.srcSubresource.layerCount = 1;
    imageBlit.srcSubresource.mipLevel = 1;
    vector3d_clear(imageBlit.srcOffsets[0]);
    imageBlit.srcOffsets[1].x = view.width;
    imageBlit.srcOffsets[1].y = view.height;
    imageBlit.srcOffsets[1].z = 1;

    imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.dstSubresource.layerCount = 1;
    imageBlit.dstSubresource.mipLevel = 2;
    vector3d_clear(imageBlit.dstOffsets[0]);
    imageBlit.dstOffsets[1].x = view.width;
    imageBlit.dstOffsets[1].y = view.height;
    imageBlit.dstOffsets[1].z = 1;

    vkCmdBlitImage(
        commandBuffer,
        layer->display->textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        gf3d_ui_manager.display->tex->textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &imageBlit, VK_FILTER_LINEAR
    );
    // slog("ui draw layer");

    // vkCmdBindVertexBuffers(commandBuffer, 0, 1, )
}