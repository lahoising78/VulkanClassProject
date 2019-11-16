#include "gf3d_gui.h"

#include "simple_logger.h"

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

// extern Texture *ui_tex;
#define _UI_ATTRIBUTE_COUNT_ 3

typedef struct 
{
    Uint32 count;
    Gui *gui_list;

    Pipeline *pipe;
    VkDevice device;

    VkVertexInputAttributeDescription attributeDescriptions[_UI_ATTRIBUTE_COUNT_];
    VkVertexInputBindingDescription bindingDescription;
} GuiManager;

static GuiManager gf3d_gui = {0};
static const Uint32 indices[6] = {0, 1, 2, 2, 3, 0};

void gf3d_gui_init(Gui *gui);
void gf3d_gui_create_vertex_buffer(Gui *gui);
void gf3d_gui_create_index_buffer(Gui *gui);
void gf3d_gui_update_descriptor_set(Gui* gui, VkDescriptorSet *descriptorSet);

void gf3d_gui_manager_close()
{
    int i;
    Gui *gui = NULL;

    slog("gui manager close");

    if(!gf3d_gui.gui_list) return;

    for(i = 0; i < gf3d_gui.count; i++)
    {
        gui = &gf3d_gui.gui_list[i];
        gf3d_gui_free(gui);
        if(gui->surface) SDL_FreeSurface(gui->surface);
        if(gui->renderer) SDL_DestroyRenderer(gui->renderer);
        gui->surface = NULL;
        gui->renderer = NULL;
        gui->_inuse = 0;
    }

    free(gf3d_gui.gui_list);
    gf3d_gui.count = 0;
}

void gf3d_gui_manager_init(Uint32 count, Pipeline *pipe, VkDevice device)
{
    gf3d_gui.gui_list = (Gui*)gfc_allocate_array(sizeof(Gui), count);
    if(!gf3d_gui.gui_list)
    {
        slog("Could not allocate gui list");
        return;
    }

    gf3d_gui.count = count;

    gf3d_gui.device = device;
    gf3d_gui.pipe = pipe;

    gf3d_gui.bindingDescription.binding = 1;
    gf3d_gui.bindingDescription.stride = sizeof(GuiVertex);
    gf3d_gui.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    gf3d_gui.attributeDescriptions[0].binding = 1;
    gf3d_gui.attributeDescriptions[0].location = 0;
    gf3d_gui.attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    gf3d_gui.attributeDescriptions[0].offset = offsetof(GuiVertex, pos);
    
    gf3d_gui.attributeDescriptions[1].binding = 1;
    gf3d_gui.attributeDescriptions[1].location = 1;
    gf3d_gui.attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    gf3d_gui.attributeDescriptions[1].offset = offsetof(GuiVertex, color);
    
    gf3d_gui.attributeDescriptions[2].binding = 1;
    gf3d_gui.attributeDescriptions[2].location = 2;
    gf3d_gui.attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    gf3d_gui.attributeDescriptions[2].offset = offsetof(GuiVertex, texel);

    atexit(gf3d_gui_manager_close);
}

void gf3d_gui_manager_draw(Uint32 bufferFrame, VkCommandBuffer commandBuffer)
{
    int i;
    Gui *gui;
    VkDescriptorSet *descriptorSets = NULL;
    
    if(gf3d_gui.pipe->descriptorSets) 
    {
        descriptorSets = gf3d_gui.pipe->descriptorSets[bufferFrame];
    }

    // slog("draw gui");
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gf3d_gui.pipe->pipeline);


        for(i = 0; i < gf3d_gui.count; i++)
        {
            gui = &gf3d_gui.gui_list[i];
            if(!gui->_inuse) continue;

            // if(gui->ui_tex) gf3d_gui_update_descriptor_set(gui, descriptorSets);
            gf3d_gui_draw(gui, descriptorSets, commandBuffer);
        }

        // vkCmdBindDescriptorSets(
        //     commandBuffer, 
        //     VK_PIPELINE_BIND_POINT_GRAPHICS,
        //     gf3d_gui.pipe->pipelineLayout,
        //     0,
        //     1, 
        //     descriptorSets,
        //     0, NULL
        // );

        // vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gf3d_vgraphics_get_graphics_pipeline()->pipeline);
    // slog("draw end");
}

Gui *gf3d_gui_new(Uint32 count, int depth)
{
    int i;
    Gui *gui = NULL;

    for(i = 0; i < gf3d_gui.count; i++)
    {
        gui = &gf3d_gui.gui_list[i];
        if(gui->_inuse) continue;

        gf3d_gui_init(gui);
        gui->elements = (GuiElement**)gfc_allocate_array(sizeof(GuiElement*), count);
        if(!gui->elements)
        {
            slog("unable to allocate enough elements");
            return NULL;
        }
        gui->elementCount = count;

        /* top left */
        gui->vertices[0].color =    gfc_color(1.0f, 0.0f, 0.0f, 1.0f);
        gui->vertices[0].pos =      vector2d(-1.0f, -1.0f);
        gui->vertices[0].texel =    vector2d(0.0f, 0.0f);
        
        /* top right */
        gui->vertices[1].color =    gfc_color(0.0f, 1.0f, 0.0f, 1.0f);
        gui->vertices[1].pos =      vector2d( 1.0f, -1.0f);
        gui->vertices[1].texel =    vector2d(1.0f, 0.0f);

        /* bottom right */
        gui->vertices[2].color =    gfc_color(0.0f, 0.0f, 1.0f, 1.0f);
        gui->vertices[2].pos =      vector2d( 1.0f,  1.0f);
        gui->vertices[2].texel =    vector2d(1.0f, 1.0f);

        /* bottom left */
        gui->vertices[3].color =    gfc_color(1.0f, 1.0f, 1.0f, 1.0f);
        gui->vertices[3].pos =      vector2d(-1.0f,  1.0f);
        gui->vertices[3].texel =    vector2d(0.0f, 1.0f);

        gf3d_gui_create_vertex_buffer(gui);
        gf3d_gui_create_index_buffer(gui);

        gui->_inuse = 1;
        return gui;
    }

    return NULL;
}

void gf3d_gui_free(Gui *gui)
{
    int i;
    GuiElement *element = NULL;
    SDL_Surface *surface;
    SDL_Renderer *renderer;

    slog("gui free");
    if(!gui) return;

    if(gui->elements) free(gui->elements);

    surface = gui->surface;
    renderer = gui->renderer;

    vkDestroyBuffer(gf3d_gui.device, gui->vertexBuffer, NULL);
    vkFreeMemory(gf3d_gui.device, gui->vertexBufferMemory, NULL);
    vkDestroyBuffer(gf3d_gui.device, gui->indexBuffer, NULL);
    vkFreeMemory(gf3d_gui.device, gui->indexBufferMemory, NULL);

    memset(gui, 0, sizeof(Gui));

    /* Do this to save the surface and renderer */
    gui->surface = surface;
    gui->renderer = renderer;
}

void gf3d_gui_add_element(Gui *gui, GuiElement *element)
{
    int i;

    if(!gui) return;

    for(i = 0; i < gui->elementCount; i++)
    {
        if(gui->elements[i]) continue;
        gui->elements[i] = element;
        return;
    }
}

void gf3d_gui_init(Gui *gui)
{
    gui->surface = SDL_CreateRGBSurface(
        0,
        gf3d_vgraphics_get_view_extent().width,
        gf3d_vgraphics_get_view_extent().height,
        32,
        rmask, gmask, bmask, amask
    );

    gui->renderer = SDL_CreateSoftwareRenderer(gui->surface);
}

void gf3d_gui_draw(Gui *gui, VkDescriptorSet *descriptorSet, VkCommandBuffer commandBuffer)
{
    int i;
    VkDeviceSize offsets[] = {0};

    if(!gui || !gui->renderer) return;

    SDL_RenderClear(gui->renderer);

    for(i = 0; i < gui->elementCount; i++)
    {
        if(gui->elements[i]) gf3d_gui_element_draw(*gui->elements[i], gui->renderer);
    }

    SDL_RenderPresent(gui->renderer);

    gf3d_texture_from_surface(gui->ui_tex, gui->surface);
    gf3d_gui_update_descriptor_set(gui, descriptorSet);

    vkCmdBindVertexBuffers(commandBuffer, 1, 1, &gui->vertexBuffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, gui->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(
        commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        gf3d_gui.pipe->pipelineLayout,
        0,
        1, 
        descriptorSet,
        0, NULL
    );
    vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
}

void gf3d_gui_create_vertex_buffer(Gui *gui)
{
    void *data;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDeviceSize bufferSize;

    slog("creating gui buffers");

    bufferSize = sizeof(GuiVertex) * 4;

    gf3d_vgraphics_create_buffer(
        bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory
    );

    vkMapMemory(gf3d_gui.device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, gui->vertices, bufferSize);
    vkUnmapMemory(gf3d_gui.device, stagingBufferMemory);

    gf3d_vgraphics_create_buffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &gui->vertexBuffer, &gui->vertexBufferMemory
    );

    gf3d_vgraphics_copy_buffer(stagingBuffer, gui->vertexBuffer, bufferSize);

    vkDestroyBuffer(gf3d_gui.device, stagingBuffer, NULL);
    vkFreeMemory(gf3d_gui.device, stagingBufferMemory, NULL);
}

void gf3d_gui_create_index_buffer(Gui *gui)
{
    void *data;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDeviceSize bufferSize;

    slog("creating gui buffers");

    bufferSize = sizeof(int) * 6;

    gf3d_vgraphics_create_buffer(
        bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory
    );

    vkMapMemory(gf3d_gui.device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices, bufferSize);
    vkUnmapMemory(gf3d_gui.device, stagingBufferMemory);

    gf3d_vgraphics_create_buffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &gui->indexBuffer, &gui->indexBufferMemory
    );

    gf3d_vgraphics_copy_buffer(stagingBuffer, gui->indexBuffer, bufferSize);

    vkDestroyBuffer(gf3d_gui.device, stagingBuffer, NULL);
    vkFreeMemory(gf3d_gui.device, stagingBufferMemory, NULL);
}

void gf3d_gui_update_descriptor_set(Gui* gui, VkDescriptorSet *descriptorSet)
{
    int i;
    VkWriteDescriptorSet descriptorWrite[1] = {0};
    VkDescriptorImageInfo imageInfo = {0};

    slog("updating descriptor sets");
    for(i = 0; i < gf3d_gui.pipe->descriptorSetCount; i++)
    {
        slog("%d", i);
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = gui->ui_tex->textureImageView;
        imageInfo.sampler = gui->ui_tex->textureSampler;

        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[0].dstSet = *descriptorSet;
        descriptorWrite[0].dstBinding = 1;
        descriptorWrite[0].dstArrayElement = 0;
        descriptorWrite[0].descriptorCount = 1;
        descriptorWrite[0].pImageInfo = &imageInfo;
        descriptorWrite[0].pTexelBufferView = NULL;
        descriptorWrite[0].pNext = NULL;

        vkUpdateDescriptorSets(gf3d_gui.device, 1, descriptorWrite, 0, NULL);
    }
}

VkVertexInputBindingDescription *gf3d_gui_get_bind_description()
{
    return &gf3d_gui.bindingDescription;
}

VkVertexInputAttributeDescription *gf3d_gui_get_attribute_descriptions(Uint32 *count)
{
    if(count) *count = _UI_ATTRIBUTE_COUNT_;
    return gf3d_gui.attributeDescriptions;
}