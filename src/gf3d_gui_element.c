#include "gf3d_gui_element.h"

#include "simple_logger.h"
#include "gf3d_vgraphics.h"
#include "gf3d_game_defines.h"
#include "gf3d_vec.h"

typedef struct 
{
    VkDevice device;
    Pipeline *pipe;
    VkDeviceSize bufferSize;
    uint16_t indices[6];
    
    TTF_Font *font;

    float screenWidth;
    float screenHeight;
} GuiElementManager;

static GuiElementManager gf3d_gui_element_manager = {0};

void gf3d_gui_element_update_vertex_buffer(GuiElement *e);
void gf3d_gui_element_create_index_buffer(GuiElement *e);
void gf3d_gui_element_update_descriptor_sets(GuiElement *e, const VkDescriptorSet *descriptorSet);

void gf3d_gui_element_manager_close()
{
    if(gf3d_gui_element_manager.font) TTF_CloseFont(gf3d_gui_element_manager.font);
    gf3d_gui_element_manager.font = NULL;
}

void gf3d_gui_element_manager_init(VkDevice device, Pipeline *pipe){
    gf3d_gui_element_manager.device = device;
    gf3d_gui_element_manager.pipe = pipe;
    gf3d_gui_element_manager.bufferSize = sizeof(GuiVertex) * 4;

    gf3d_gui_element_manager.indices[0] = 0;
    gf3d_gui_element_manager.indices[1] = 1;
    gf3d_gui_element_manager.indices[2] = 2;
    gf3d_gui_element_manager.indices[3] = 2;
    gf3d_gui_element_manager.indices[4] = 3;
    gf3d_gui_element_manager.indices[5] = 0;

    gf3d_gui_element_manager.screenWidth = (float)gf3d_vgraphics_get_view_extent().width;
    gf3d_gui_element_manager.screenHeight = (float)gf3d_vgraphics_get_view_extent().height;

    gf3d_gui_element_manager.font = TTF_OpenFont( "fonts/RobotoMono-Light.ttf", 32 );
    // gf3d_gui_element_manager.font = TTF_OpenFont( "fonts/lazy.ttf", 28);
    if(!gf3d_gui_element_manager.font)
    {
        slog("\033[0;33mWARNING:\033[0m font was not found");
    }

    atexit(gf3d_gui_element_manager_close);
}

GuiElement *gf3d_gui_element_create(Vector2D pos, Vector2D ext, Vector4D color)
{
    SDL_Surface *surface = NULL;
    SDL_Renderer *renderer = NULL;

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

    element->vertices[0].texel = vector2d(1.0f, 0.0f);
    element->vertices[1].texel = vector2d(0.0f, 0.0f);
    element->vertices[2].texel = vector2d(0.0f, 1.0f);
    element->vertices[3].texel = vector2d(1.0f, 1.0f);

    gf3d_vgraphics_create_buffer(
        gf3d_gui_element_manager.bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &element->stagingBuffer, &element->stagingBufferMemory
    );

    gf3d_vgraphics_create_buffer(
        gf3d_gui_element_manager.bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &element->vertexBuffer, &element->vertexBufferMemory
    );

    gf3d_gui_element_update_vertex_buffer(element);

    gf3d_gui_element_create_index_buffer(element);

    /* initialize blank texture so it is not empty */
    surface = SDL_CreateRGBSurface(
        0,
        gf3d_gui_element_manager.screenWidth,
        gf3d_gui_element_manager.screenHeight,
        32,
        rmask, gmask, bmask, amask
    );

    renderer = SDL_CreateSoftwareRenderer(surface);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    element->tex = gf3d_texture_from_surface(NULL, surface);

    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);

    return element;
}

GuiElement *gf3d_gui_element_load(SJson *json)
{
    SJson *arr = NULL;
    SJson *obj = NULL;
    GuiElement *e = NULL;

    Vector2D pos, ext;
    Vector4D color;
    char buf[GFCLINELEN];

    if(!json) return NULL;

    arr = sj_object_get_value(json, "position");
    obj = sj_array_get_nth(arr, 0);
    sj_get_float_value(obj, &pos.x);
    obj = sj_array_get_nth(arr, 1);
    sj_get_float_value(obj, &pos.y);
    
    arr = sj_object_get_value(json, "extents");
    obj = sj_array_get_nth(arr, 0);
    sj_get_float_value(obj, &ext.x);
    obj = sj_array_get_nth(arr, 1);
    sj_get_float_value(obj, &ext.y);

    arr = sj_object_get_value(json, "color");
    obj = sj_array_get_nth(arr, 0);
    sj_get_float_value(obj, &color.x);
    obj = sj_array_get_nth(arr, 1);
    sj_get_float_value(obj, &color.y);
    obj = sj_array_get_nth(arr, 2);
    sj_get_float_value(obj, &color.z);
    obj = sj_array_get_nth(arr, 3);
    sj_get_float_value(obj, &color.w);

    e = gf3d_gui_element_create(pos, ext, color);

    obj = sj_object_get_value(json, "texture");
    if(obj)
    {
        snprintf(buf, GFCLINELEN, "%s", sj_get_string_value(obj));
        gf3d_gui_element_attach_texture(e, buf);
    }

    return e;
}

SJson *gf3d_gui_element_to_json(GuiElement *e)
{
    SJson *obj = NULL;
    SJson *arr = NULL;
    if(!e) return NULL;

    obj = sj_object_new();
    if(!obj) return NULL;

    arr = gf3d_vec2_json(e->position);
    sj_object_insert(obj, "position", arr);
    
    arr = gf3d_vec2_json(e->extents);
    sj_object_insert(obj, "extents", arr);

    arr = gf3d_vec4_json(e->color);
    sj_object_insert(obj, "color", arr);
    
    return obj;
}

void gf3d_gui_element_attach_texture(GuiElement *gui, char *textureName)
{
    Texture *tex = NULL;
    if(!gui) return;

    tex = gui->tex;
    gui->tex = gf3d_texture_load(textureName);
    if(tex != gui->tex)
    {
        gf3d_texture_free(tex);
    }
}

void gf3d_gui_element_attach_texture_from_surface(GuiElement *gui, SDL_Surface *surface)
{
    Texture *tex = NULL;
    if(!gui) 
    {
        slog("no gui to attach surface");
        return;
    }
    if(!surface)
    {
        slog("no surface to attach to gui");
        return;
    }

    tex = gui->tex;
    gui->tex = gf3d_texture_from_surface(NULL, surface);
    if(tex != gui->tex)
    {
        gf3d_texture_free(tex);
    }
}

void gf3d_gui_element_free(GuiElement *e)
{
    if(!e) return;
    slog("free element");
    vkDestroyBuffer (gf3d_gui_element_manager.device, e->stagingBuffer, NULL);
    vkFreeMemory    (gf3d_gui_element_manager.device, e->stagingBufferMemory, NULL);
    vkDestroyBuffer (gf3d_gui_element_manager.device, e->vertexBuffer, NULL);
    vkFreeMemory    (gf3d_gui_element_manager.device, e->vertexBufferMemory, NULL);
    vkDestroyBuffer (gf3d_gui_element_manager.device, e->indexBuffer, NULL);
    vkFreeMemory    (gf3d_gui_element_manager.device, e->indexBufferMemory, NULL);
    if(e->tex) gf3d_texture_free(e->tex);
    // free(e);
}

void gf3d_gui_element_update(GuiElement *e)
{
    float screenWidth = gf3d_gui_element_manager.screenWidth;
    float screenHeight = gf3d_gui_element_manager.screenHeight;

    if(!e) return;

    vector4d_mul(e->vertices[0].color, e->color, (1/255.0f));
    e->vertices[0].pos.x = e->position.x * 2 / screenWidth - 1.0f;
    e->vertices[0].pos.y = e->position.y * 2 / screenHeight - 1.0f;

    vector4d_mul(e->vertices[1].color, e->color, (1/255.0f));
    e->vertices[1].pos.x = (e->position.x + e->extents.x) * 2 / screenWidth - 1.0f;
    e->vertices[1].pos.y = e->position.y * 2 / screenHeight - 1.0f;
    
    vector4d_mul(e->vertices[2].color, e->color, (1/255.0f));
    e->vertices[2].pos.x = (e->position.x + e->extents.x) * 2 / screenWidth - 1.0f;
    e->vertices[2].pos.y = (e->position.y + e->extents.y) * 2 / screenHeight - 1.0f;
    
    vector4d_mul(e->vertices[3].color, e->color, (1/255.0f));
    e->vertices[3].pos.x = e->position.x * 2 / screenWidth - 1.0f;
    e->vertices[3].pos.y = (e->position.y + e->extents.y) * 2 / screenHeight - 1.0f;
}

// void gf3d_gui_element_draw(GuiElement *element, SDL_Renderer *renderer)
void gf3d_gui_element_draw(GuiElement *element, uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    VkBuffer vertexBuffers[] = {element->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    const VkDescriptorSet *descriptorSet = gf3d_pipeline_get_descriptor_set(gf3d_gui_element_manager.pipe, bufferFrame);

    if(!element) return;

    gf3d_gui_element_update_descriptor_sets(element, descriptorSet);

    gf3d_gui_element_update_vertex_buffer(element);

    vkCmdBindVertexBuffers(commandBuffer, 1, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, element->indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdBindDescriptorSets(
        commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        gf3d_gui_element_manager.pipe->pipelineLayout,
        0, 1, descriptorSet,
        0, NULL
    );
    vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
}

/* ======VULKAN STUFF===== */
void gf3d_gui_element_update_vertex_buffer(GuiElement *e)
{
    void *data;

    if(!e) return;

    // vector4d_copy(e->vertices[0].color, e->color);
    // vector2d_copy(e->vertices[0].pos, e->position);

    // vector4d_copy(e->vertices[1].color, e->color);
    // e->vertices[1].pos = vector2d(e->position.x + e->extents.x, e->position.y);
    
    // vector4d_copy(e->vertices[2].color, e->color);
    // e->vertices[2].pos = vector2d(e->position.x + e->extents.x, e->position.y + e->extents.y);
    
    // vector4d_copy(e->vertices[3].color, e->color);
    // e->vertices[3].pos = vector2d(e->position.x, e->position.y + e->extents.y);

    vkMapMemory(gf3d_gui_element_manager.device, e->stagingBufferMemory, 0, gf3d_gui_element_manager.bufferSize, 0, &data);
        memcpy(data, e->vertices, gf3d_gui_element_manager.bufferSize);
    vkUnmapMemory(gf3d_gui_element_manager.device, e->stagingBufferMemory);

    gf3d_vgraphics_copy_buffer(e->stagingBuffer, e->vertexBuffer, gf3d_gui_element_manager.bufferSize);
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

    vkMapMemory(gf3d_gui_element_manager.device, stagingBufferMemory, 0, buffsize, 0, &data);
        memcpy(data, gf3d_gui_element_manager.indices, buffsize);
    vkUnmapMemory(gf3d_gui_element_manager.device, stagingBufferMemory);

    gf3d_vgraphics_create_buffer(
        buffsize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &e->indexBuffer, &e->indexBufferMemory
    );

    gf3d_vgraphics_copy_buffer(stagingBuffer, e->indexBuffer, buffsize);

    vkDestroyBuffer(gf3d_gui_element_manager.device, stagingBuffer, NULL);
    vkFreeMemory(gf3d_gui_element_manager.device, stagingBufferMemory, NULL);
}

void gf3d_gui_element_update_descriptor_sets(GuiElement *e, const VkDescriptorSet *descriptorSet)
{
    VkDescriptorImageInfo imageInfo = {0};
    VkWriteDescriptorSet descriptorWrite[1] = {0};
    // const VkDescriptorSet *descriptorSet = gf3d_pipeline_get_descriptor_set(gf3d_gui_element_manager.pipe, bufferFrame);

    if(!e)
    {
        slog("element is null");
        return;
    }
    if(!e->tex)
    {
        slog("texture is null");
        return;
    }

    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = e->tex->textureImageView;
    imageInfo.sampler = e->tex->textureSampler;

    descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[0].dstSet = *descriptorSet;
    descriptorWrite[0].dstBinding = 1;
    descriptorWrite[0].dstArrayElement = 0;
    descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite[0].descriptorCount = 1;
    descriptorWrite[0].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(gf3d_gui_element_manager.device, 1, descriptorWrite, 0, NULL);
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

TTF_Font *gf3d_gui_element_get_font()
{
    return gf3d_gui_element_manager.font;
}