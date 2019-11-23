#include "gf3d_gui_element.h"

#include "simple_logger.h"
#include "gf3d_vgraphics.h"

static VkDevice gf3d_gui_element_device;
static VkDeviceSize bufferSize = sizeof(GuiVertex) * 4;
static const uint16_t indices[] = {0, 1, 2, 2, 3, 0};

void gf3d_gui_element_update_vertex_buffer(GuiElement *e);
void gf3d_gui_element_create_index_buffer(GuiElement *e);

GuiElement *gf3d_gui_element_create(Vector2D pos, Vector2D ext, Vector4D color)
{
    int i;

    GuiElement *element;
    element = (GuiElement*) malloc(sizeof(GuiElement));
    if(!element) 
    {
        slog("unnable to allocate element");
        return NULL;
    }

    element->position = pos;
    element->extents = ext;
    element->color = color;
    element->tex = NULL;

    for(i = 0; i < 4; i++)
    {
        vector2d_clear(element->vertices[i].texel);
    }

    gf3d_vgraphics_create_buffer(
        bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &element->stagingBuffer, &element->stagingBufferMemory
    );

    gf3d_vgraphics_create_buffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &element->vertexBuffer, &element->vertexBufferMemory
    );

    gf3d_gui_element_update_vertex_buffer(element);

    gf3d_gui_element_create_index_buffer(element);

    return element;
}

void gf3d_gui_element_free(GuiElement *e)
{
    if(!e) return;
    slog("free element");
    vkDestroyBuffer (gf3d_gui_element_device, e->stagingBuffer, NULL);
    vkFreeMemory    (gf3d_gui_element_device, e->stagingBufferMemory, NULL);
    vkDestroyBuffer (gf3d_gui_element_device, e->vertexBuffer, NULL);
    vkFreeMemory    (gf3d_gui_element_device, e->vertexBufferMemory, NULL);
    vkDestroyBuffer (gf3d_gui_element_device, e->indexBuffer, NULL);
    vkFreeMemory    (gf3d_gui_element_device, e->indexBufferMemory, NULL);
    if(e->tex) gf3d_texture_free(e->tex);
    // free(e);
}

// void gf3d_gui_element_draw(GuiElement *element, SDL_Renderer *renderer)
void gf3d_gui_element_draw(GuiElement *element, VkCommandBuffer commandBuffer)
{
    VkBuffer vertexBuffers[] = {element->vertexBuffer};
    VkDeviceSize offsets[] = {0};

    if(!element) return;

    gf3d_gui_element_update_vertex_buffer(element);

    vkCmdBindVertexBuffers(commandBuffer, 1, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, element->indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);

    // SDL_Rect rect;

    // if(!renderer) return;

    // rect.x = element.position.x;
    // rect.y = element.position.y;
    // rect.w = element.extents.x;
    // rect.h = element.extents.y;

    // SDL_SetRenderDrawColor(renderer, element.color.r * 255, element.color.g * 255, element.color.b * 255, element.color.a * 255);
    // SDL_RenderFillRect(renderer, &rect);
}

/* ======VULKAN STUFF===== */
void gf3d_gui_element_set_vk_device(VkDevice device)
{
    gf3d_gui_element_device = device;
}

void gf3d_gui_element_update_vertex_buffer(GuiElement *e)
{
    void *data;

    if(!e) return;

    vector4d_copy(e->vertices[0].color, e->color);
    vector2d_copy(e->vertices[0].pos, e->position);

    vector4d_copy(e->vertices[1].color, e->color);
    e->vertices[1].pos = vector2d(e->position.x + e->extents.x, e->position.y);
    
    vector4d_copy(e->vertices[2].color, e->color);
    e->vertices[2].pos = vector2d(e->position.x + e->extents.x, e->position.y + e->extents.y);
    
    vector4d_copy(e->vertices[3].color, e->color);
    e->vertices[3].pos = vector2d(e->position.x, e->position.y + e->extents.y);

    vkMapMemory(gf3d_gui_element_device, e->stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, e->vertices, bufferSize);
    vkUnmapMemory(gf3d_gui_element_device, e->stagingBufferMemory);

    gf3d_vgraphics_copy_buffer(e->stagingBuffer, e->vertexBuffer, bufferSize);
}

void gf3d_gui_element_create_index_buffer(GuiElement *e)
{
    void *data;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDeviceSize buffsize;

    buffsize = sizeof(uint16_t) * 6;

    gf3d_vgraphics_create_buffer(
        buffsize,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory
    );

    vkMapMemory(gf3d_gui_element_device, stagingBufferMemory, 0, buffsize, 0, &data);
        memcpy(data, indices, buffsize);
    vkUnmapMemory(gf3d_gui_element_device, stagingBufferMemory);

    gf3d_vgraphics_create_buffer(
        buffsize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &e->indexBuffer, &e->indexBufferMemory
    );

    gf3d_vgraphics_copy_buffer(stagingBuffer, e->indexBuffer, buffsize);

    vkDestroyBuffer(gf3d_gui_element_device, stagingBuffer, NULL);
    vkFreeMemory(gf3d_gui_element_device, stagingBufferMemory, NULL);
}

/* ==========COLOR======== */
Color gfc_color(float r,float g,float b,float a)
{
    Color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}