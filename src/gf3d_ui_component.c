#include "gf3d_ui_component.h"
#include "simple_logger.h"
#include "gf3d_mesh.h"
#include "gf3d_vgraphics.h"

typedef struct 
{
    uint32_t vcount;
    uint32_t fcount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkBuffer faceBuffer;
    VkDeviceMemory faceBufferMemory;
    
    Vector2D ext;
    UniformBufferObject *ubo;
    VkDevice device;
} uiComponentManager;

static uiComponentManager gf3d_ui_component_manager = {0};

void gf3d_ui_component_create_vertex_buffer(uiComponent *ui)
{
    void *data = NULL;
    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    size_t bufferSize;    
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    Vertex vertices[4] = {
        {
            {-1,  0, 1},
            {0, -1, 0},
            {0, 1}
        },
        {
            { 1,  0, 1},
            {0, -1, 0},
            {1, 1}
        },
        {
            { 1,  0, -1},
            {0, -1, 0},
            {1, 0}
        },
        {
            {-1,  0, -1},
            {0, -1, 0},
            {0, 0}
        }
    };

    if(!ui) return;

    bufferSize = sizeof(Vertex) * gf3d_ui_component_manager.vcount;
    
    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
    
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &ui->vertexBuffer, &ui->vertexBufferMemory);

    gf3d_vgraphics_copy_buffer(stagingBuffer, ui->vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);
}

void gf3d_ui_component_manager_close()
{
    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    vkDestroyBuffer(device, gf3d_ui_component_manager.stagingBuffer, NULL);
    vkFreeMemory(device, gf3d_ui_component_manager.stagingBufferMemory, NULL);
    vkDestroyBuffer(device, gf3d_ui_component_manager.faceBuffer, NULL);
    vkFreeMemory(device, gf3d_ui_component_manager.faceBufferMemory, NULL);
}

void gf3d_ui_component_manager_init()
{
    void* data;
    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    VkDeviceSize bufferSize = sizeof(Face) * 2;
    VkExtent2D ext = gf3d_vgraphics_get_view_extent();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    Face faces[bufferSize / sizeof(Face)];
    int i,j;

    gf3d_ui_component_manager.fcount = bufferSize / sizeof(Face);
    faces[0].verts[0] = 0;
    faces[0].verts[1] = 1;
    faces[0].verts[2] = 2;
    faces[1].verts[0] = 2;
    faces[1].verts[1] = 3;
    faces[1].verts[2] = 0;

    gf3d_ui_component_manager.vcount = 4;
    gf3d_ui_component_manager.device = gf3d_vgraphics_get_default_logical_device();
    
    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, faces, (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &gf3d_ui_component_manager.faceBuffer, &gf3d_ui_component_manager.faceBufferMemory);

    gf3d_vgraphics_copy_buffer(stagingBuffer, gf3d_ui_component_manager.faceBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    gf3d_ui_component_manager.ext.x = (float)ext.width;
    gf3d_ui_component_manager.ext.y = (float)ext.height;

    gf3d_ui_component_manager.ubo = (UniformBufferObject*)malloc(sizeof(UniformBufferObject));
    if(gf3d_ui_component_manager.ubo)
    {
        slog("ui system set up ubo");

        gfc_matrix_identity(gf3d_ui_component_manager.ubo->proj);
        gfc_matrix_perspective(
            gf3d_ui_component_manager.ubo->proj,
            atanf(gf3d_ui_component_manager.ext.y/2.0f),
            // 45 * GFC_DEGTORAD,
            gf3d_ui_component_manager.ext.x/gf3d_ui_component_manager.ext.y,
            0.1f,
            1.1
        );

        gfc_matrix_identity(gf3d_ui_component_manager.ubo->view);
        gfc_matrix_view(
            gf3d_ui_component_manager.ubo->view,
            vector3d( 0, -1,  0),
            vector3d( 0,  0,  0),
            vector3d( 0,  0, 1)
        );
    }

    atexit(gf3d_ui_component_manager_close);
}

void gf3d_ui_component_init( uiComponent *ui )
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    VkExtent2D ext = gf3d_vgraphics_get_view_extent();

    if(!ui) return;
    ui->_inuse = 1;
    ui->visible = 1;
    ui->active = 1;
    gfc_matrix_identity(ui->mat);
    gfc_matrix_make_translation(ui->mat, vector3d(ui->position.x, 0,ui->position.y));
    ui->mat[0][0] = 1.0f;
    ui->mat[1][1] = 1.0f;
    ui->mat[2][2] = 1.0f;

    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &ui->uniformBuffer, &ui->uniformBufferMemory);
}

void gf3d_ui_component_update( uiComponent *ui )
{
    static uiComponent *first = NULL;
    static int updated = 0;
    void *data;
    VkDeviceSize imageSize;
    
    SDL_Rect rect = {0};

    if(!ui) return;
    if(first && ui != first) return;
    if(updated > 2) return;

    // updated++;
    first = ui;

    slog("updating the first");

    if(!ui->surface) ui->surface = SDL_CreateRGBSurface(
        0,
        gf3d_ui_component_manager.ext.x, gf3d_ui_component_manager.ext.y,
        32,
        rmask, gmask, bmask, amask
    );

    if(!ui->renderer) ui->renderer = SDL_CreateSoftwareRenderer(ui->surface);

    rect.w = rect.h = rect.y = 100;
    rect.x = updated * 200;

    imageSize = ui->surface->w * ui->surface->h * 4;

    if(updated == 0)
    {
        gf3d_vgraphics_create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &gf3d_ui_component_manager.stagingBuffer, &gf3d_ui_component_manager.stagingBufferMemory);
    }

    SDL_LockSurface(ui->surface);
        SDL_SetRenderDrawColor(ui->renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(ui->renderer, &rect);

        vkMapMemory(gf3d_ui_component_manager.device, gf3d_ui_component_manager.stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, ui->surface->pixels, imageSize);
        vkUnmapMemory(gf3d_ui_component_manager.device, gf3d_ui_component_manager.stagingBufferMemory);
    SDL_UnlockSurface(ui->surface);

    gf3d_swapchain_transition_image_layout(ui->texture->textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    gf3d_texture_copy_buffer_to_image(gf3d_ui_component_manager.stagingBuffer, ui->texture->textureImage, ui->texture->w, ui->texture->h);
    gf3d_swapchain_transition_image_layout(ui->texture->textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void gf3d_ui_component_free( uiComponent *ui )
{
    VkDevice device = gf3d_vgraphics_get_default_logical_device();

    slog("free ui component");
    if(!ui) return;

    if(ui->texture) gf3d_texture_free(ui->texture);

    vkDestroyBuffer(device, ui->vertexBuffer, NULL);
    vkFreeMemory(device, ui->vertexBufferMemory, NULL); 
    vkDestroyBuffer(device, ui->uniformBuffer, NULL);
    vkFreeMemory(device, ui->uniformBufferMemory, NULL); 

    SDL_DestroyRenderer(ui->renderer);
    SDL_FreeSurface(ui->surface);

    memset(ui, 0, sizeof(uiComponent));
}

void gf3d_ui_component_attach_texture_from_file( uiComponent *ui, const char *filename )
{
    TextLine assetname;

    if(!ui) return;

    snprintf(assetname, GFCLINELEN, "images/ui/%s.png", filename);
    if(ui->texture)
    {
        slog("cannot call this more than once per component");
        return;
    }

    ui->texture = gf3d_texture_load(assetname);
    gf3d_ui_component_create_vertex_buffer(ui);

}

extern float timeSinceStart;
void gf3d_ui_update_uniform_buffer(uiComponent *ui)
{
    void* data;
    UniformBufferObject *ubo;
    // ubo = gf3d_vgraphics_get_uniform_buffer_object();
    ubo = gf3d_ui_component_manager.ubo;
    ubo->time = timeSinceStart;
    gfc_matrix_copy(ubo->model,ui->mat);
    ubo->model[0][0] *= ui->texture->w / gf3d_ui_component_manager.ext.y;
    ubo->model[2][2] *= ui->texture->h / gf3d_ui_component_manager.ext.y;
    vkMapMemory(gf3d_ui_component_manager.device, ui->uniformBufferMemory, 0, sizeof(UniformBufferObject), 0, &data);
    
        memcpy(data, ubo, sizeof(UniformBufferObject));

    vkUnmapMemory(gf3d_ui_component_manager.device, ui->uniformBufferMemory);
}

void gf3d_ui_component_update_descriptor_set(uiComponent *ui,VkDescriptorSet descriptorSet)
{
    VkDescriptorImageInfo imageInfo = {0};
    VkWriteDescriptorSet descriptorWrite[2] = {0};
    VkDescriptorBufferInfo bufferInfo = {0};

    if (!ui)
    {
        slog("no ui provided for descriptor set update");
        return;
    }
    if (descriptorSet == VK_NULL_HANDLE)
    {
        slog("null handle provided for descriptorSet");
        return;
    }

    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = ui->texture->textureImageView;
    imageInfo.sampler = ui->texture->textureSampler;

    gf3d_ui_update_uniform_buffer(ui);
    bufferInfo.buffer = ui->uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);        
    
    descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[0].dstSet = descriptorSet;
    descriptorWrite[0].dstBinding = 0;
    descriptorWrite[0].dstArrayElement = 0;
    descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite[0].descriptorCount = 1;
    descriptorWrite[0].pBufferInfo = &bufferInfo;

    descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[1].dstSet = descriptorSet;
    descriptorWrite[1].dstBinding = 1;
    descriptorWrite[1].dstArrayElement = 0;
    descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite[1].descriptorCount = 1;                        
    descriptorWrite[1].pImageInfo = &imageInfo;
    descriptorWrite[1].pTexelBufferView = NULL; // Optional

    vkUpdateDescriptorSets(gf3d_ui_component_manager.device, 2, descriptorWrite, 0, NULL);
}

void gf3d_ui_component_render( uiComponent *ui, uint32_t bufferFrame, VkCommandBuffer commandBuffer )
{
    VkDescriptorSet *descriptorSet = NULL;
    Pipeline *pipe = gf3d_vgraphics_get_graphics_pipeline();
    VkDeviceSize offsets[] = {0};

    // slog("draw ui component");

    if(!ui) 
    {
        slog("cannot render null ui");
        return;
    }

    descriptorSet = gf3d_pipeline_get_descriptor_set(pipe, bufferFrame );
    if(!descriptorSet)
    {
        slog("failed to get free descriptor set for ui");
        return;
    }

    gf3d_ui_component_update_descriptor_set(ui, *descriptorSet);

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &ui->vertexBuffer, offsets);
    
    vkCmdBindIndexBuffer(commandBuffer, gf3d_ui_component_manager.faceBuffer, 0, VK_INDEX_TYPE_UINT32);
    
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe->pipelineLayout, 0, 1, descriptorSet, 0, NULL);
    
    vkCmdDrawIndexed(commandBuffer, gf3d_ui_component_manager.fcount * 3, 1, 0, 0, 0);
}