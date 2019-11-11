#include "gf3d_gui.h"

#include "simple_logger.h"
#include "gf3d_vgraphics.h"
#include "gf3d_swapchain.h"

#define GUI_ATTRIBUTE_COUNT 2

typedef struct
{
    Uint32 max_elements;
    GuiElement *elements;

    VkVertexInputAttributeDescription attributeDescriptions[GUI_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription bindingDescription;
    Uint32 chain_length;
    VkDevice device;
    Pipeline *pipe;
} GuiManager;

GuiManager gf3d_gui_manager = {0};

void gf3d_gui_delete( GuiElement *gui );

void gf3d_gui_create_index_buffer( GuiElement *gui );
void gf3d_gui_create_vertex_buffer( GuiElement *gui );
void gf3d_gui_create_image_view( GuiElement *gui );
void gf3d_gui_update_basic_gui_descriptor_set(GuiElement *gui, VkDescriptorSet descriptorSet, Uint32 bufferFrame);
void gf3d_gui_draw( GuiElement *gui, VkCommandBuffer commandBuffer, VkDescriptorSet *descriptorSet );

void gf3d_gui_manager_close()
{
    int i;

    slog("gui manager close");

    for(i = 0; i < gf3d_gui_manager.max_elements; i++)
    {
        gf3d_gui_delete(&gf3d_gui_manager.elements[i]);
    }

    if(gf3d_gui_manager.elements) free(gf3d_gui_manager.elements);
    gf3d_gui_manager.elements = NULL;
}

void gf3d_gui_manager_init(Uint32 count, Uint32 chain_length, VkDevice device)
{
    gf3d_gui_manager.elements = (GuiElement*)gfc_allocate_array(sizeof(GuiElement), count);
    if(!gf3d_gui_manager.elements)
    {
        slog("failed to allocate gui elements");
        return;
    }
    
    gf3d_gui_manager.max_elements = count;
    gf3d_gui_manager.chain_length = chain_length;
    gf3d_gui_manager.device = device;
    gf3d_gui_manager.pipe = gf3d_vgraphics_get_graphics_pipeline2D();
    atexit(gf3d_gui_manager_close);

    gf3d_gui_manager.bindingDescription.binding = 1;
    gf3d_gui_manager.bindingDescription.stride = sizeof(GuiVertex);
    gf3d_gui_manager.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    gf3d_gui_manager.attributeDescriptions[0].binding = 1;
    gf3d_gui_manager.attributeDescriptions[0].location = 0;
    gf3d_gui_manager.attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    gf3d_gui_manager.attributeDescriptions[0].offset = offsetof(GuiVertex, position);

    gf3d_gui_manager.attributeDescriptions[1].binding = 1;
    gf3d_gui_manager.attributeDescriptions[1].location = 1;
    gf3d_gui_manager.attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    gf3d_gui_manager.attributeDescriptions[1].offset = offsetof(GuiVertex, color);

}

void gf3d_gui_manager_draw(Uint32 bufferFrame, VkCommandBuffer commandBuffer)
{
    int i;
    GuiElement *element = NULL;
    VkDescriptorSet *descriptorSet = NULL;

    // descriptorSet = gf3d_pipeline_get_descriptor_set( gf3d_gui_manager.pipe, bufferFrame );
    // if(!descriptorSet)
    // {
    //     slog("failed to get a descriptor set for gui rendering");
    //     return;
    // }

    for(i = 0; i < gf3d_gui_manager.max_elements; i++)
    {
        element = &gf3d_gui_manager.elements[i];
        if(!element->_inuse) continue;

        // gf3d_gui_update_basic_gui_descriptor_set( element, *descriptorSet, bufferFrame );
        gf3d_gui_draw( element, commandBuffer, descriptorSet );
    }
}

GuiElement *gf3d_gui(Vector2D position, Vector2D extents, Color color)
{
    GuiElement *element = {0};
    int i;

    for(i = 0; i < gf3d_gui_manager.max_elements; i++)
    {
        if(gf3d_gui_manager.elements[i]._inuse) continue;
        element = &gf3d_gui_manager.elements[i];

        memset(element, 0, sizeof(GuiElement));
        element->_inuse = 1;
        element->transform = gf3d_shape(
            vector3d(position.x, position.y, 0),
            vector3d(extents.x, extents.y, 0),
            NULL
        );

        element->vertices[0].color = color;
        element->vertices[0].position = position;
        
        element->vertices[1].color = color;
        element->vertices[1].position = vector2d(position.x + extents.x, position.y);
        
        element->vertices[2].color = color;
        element->vertices[2].position = vector2d(position.x + extents.x, position.y + extents.y);
        
        element->vertices[3].color = color;
        element->vertices[3].position = vector2d(position.x, position.y + extents.y);

        element->indices[0] = 0;
        element->indices[1] = 1;
        element->indices[2] = 2;
        element->indices[3] = 2;
        element->indices[4] = 3;
        element->indices[5] = 0;
        
        element->color = color;

        // gf3d_gui_create_image_view(element);
        gf3d_gui_create_vertex_buffer(element);

        return element;
    }

    return NULL;
}

void gf3d_gui_free(GuiElement *gui)
{
    if(!gui) return;
    gui->_inuse = 0;
}

void gf3d_gui_delete( GuiElement *gui )
{
    if(!gui) return;
    if (gui->indexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(gf3d_vgraphics_get_default_logical_device(), gui->indexBuffer, NULL);
        slog("gui index buffer freed");
    }
    if (gui->indexBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf3d_vgraphics_get_default_logical_device(), gui->indexBufferMemory, NULL);
        slog("gui index buffer memory freed");
    }
    if (gui->buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(gf3d_vgraphics_get_default_logical_device(), gui->buffer, NULL);
        slog("gui vert buffer freed");
    }
    if (gui->bufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf3d_vgraphics_get_default_logical_device(), gui->bufferMemory, NULL);
        slog("gui vert buffer memory freed");
    }
    // if(gui->imageView != VK_NULL_HANDLE)
    // {
    //     vkDestroyImageView(gf3d_gui_manager.device, gui->imageView, NULL);
    // }
    // if(gui->texture)
    // {
    //     gf3d_texture_free(gui->texture);
    // }
}

/* Vulkan Stuff For GUI */
void gf3d_gui_create_index_buffer( GuiElement *gui )
{
    void *data = NULL;

    size_t bufferSize;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    bufferSize = sizeof(Uint16) * 6;

    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(gf3d_gui_manager.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, &gui->indices, bufferSize);
    vkUnmapMemory(gf3d_gui_manager.device, stagingBufferMemory);

    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &gui->indexBuffer, &gui->indexBufferMemory);

    gf3d_vgraphics_copy_buffer(stagingBuffer, gui->indexBuffer, bufferSize);

    vkDestroyBuffer(gf3d_gui_manager.device, stagingBuffer, NULL);
    vkFreeMemory(gf3d_gui_manager.device, stagingBufferMemory, NULL);
}

void gf3d_gui_create_vertex_buffer( GuiElement *gui )
{
    void *data = NULL;
    VkDevice device;
    size_t bufferSize;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    bufferSize = sizeof(GuiVertex) * 4;

    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    device = gf3d_gui_manager.device;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, gui->vertices, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &gui->buffer, &gui->bufferMemory);

    gf3d_vgraphics_copy_buffer(stagingBuffer, gui->buffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    gf3d_gui_create_index_buffer(gui);

}

void gf3d_gui_create_image_view(GuiElement *gui)
{
    // gui->texture = gf3d_texture_load("images/red.png"); 
    // gui->image = tex->textureImage;
    // gui->imageView = gf3d_vgraphics_create_image_view(gui->image, VK_FORMAT_R8G8B8A8_UNORM);
    // gf3d_texture_free(tex);
}

void gf3d_gui_update_basic_gui_descriptor_set(GuiElement *gui, VkDescriptorSet descriptorSet, Uint32 bufferFrame)
{
    // VkDescriptorImageInfo imageInfo = {0};
    // VkWriteDescriptorSet descriptorWrite[2] = {0};
    // VkDescriptorBufferInfo bufferInfo = {0};

    // slog("updating descriptor set");

    // if (!gui)
    // {
    //     slog("no model provided for descriptor set update");
    //     return;
    // }
    // if (descriptorSet == VK_NULL_HANDLE)
    // {
    //     slog("null handle provided for descriptorSet");
    //     return;
    // }

    // imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // imageInfo.imageView = gui->texture->textureImageView;
    // imageInfo.sampler = gui->texture->textureSampler;

    // gf3d_model_update_uniform_buffer(model,chainIndex,modelMat);
    // bufferInfo.buffer = model->uniformBuffers[chainIndex];
    // bufferInfo.offset = 0;
    // bufferInfo.range = sizeof(UniformBufferObject);        
    
    // descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // descriptorWrite[0].dstSet = descriptorSet;
    // descriptorWrite[0].dstBinding = 0;
    // descriptorWrite[0].dstArrayElement = 0;
    // descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // descriptorWrite[0].descriptorCount = 1;
    // descriptorWrite[0].pBufferInfo = &bufferInfo;

    // descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // descriptorWrite[1].dstSet = descriptorSet;
    // descriptorWrite[1].dstBinding = 1;
    // descriptorWrite[1].dstArrayElement = 0;
    // descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // descriptorWrite[1].descriptorCount = 1;                        
    // descriptorWrite[1].pImageInfo = &imageInfo;
    // descriptorWrite[1].pTexelBufferView = NULL; // Optional

    // vkUpdateDescriptorSets(gf3d_model.device, 2, descriptorWrite, 0, NULL);
}

void gf3d_gui_draw( GuiElement *gui, VkCommandBuffer commandBuffer, VkDescriptorSet *descriptorSet )
{
    VkDeviceSize offsets[] = {0};
    Pipeline *pipe = NULL;

    slog("draw gui");
    
    if(!gui)
    {
        slog("cannot render NULL gui");
        return;
    }

    pipe = gf3d_gui_manager.pipe;

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &gui->buffer, offsets);

    vkCmdBindIndexBuffer(commandBuffer, gui->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    // vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe->pipelineLayout, 0, 1, descriptorSet, 0, NULL);

    vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
}

/* stuff that wasn't defined on gfc */
Color gfc_color(float r, float g, float b, float a)
{
    Color color = {0};

    if(r > 1) r = 1;
    else if (r < 0) r = 0;

    if(g > 1) g = 1;
    else if (g < 0) g = 0;
    
    if(b > 1) b = 1;
    else if (b < 0) b = 0;
    
    if(a > 1) a = 1;
    else if (a < 0) a = 0;
    
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;

    return color;
}