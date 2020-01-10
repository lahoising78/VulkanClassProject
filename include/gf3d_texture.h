#ifndef __GF3D_TEXTURE_H__
#define __GF3D_TEXTURE_H__

#include <vulkan/vulkan.h>
#include "gfc_types.h"
#include "gfc_text.h"

typedef struct
{
    Uint8               _inuse;
    Uint32              _refcount;
    uint32_t            w, h;
    TextLine            filename;
    VkImage             textureImage;
    VkDeviceMemory      textureImageMemory;
    VkImageView         textureImageView;
    VkSampler           textureSampler;
}Texture;

typedef struct
{
    Texture *texture;
    VkDeviceSize texturePixelDataSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    Uint32 w;
    Uint32 h;
} TextureUI;


void gf3d_texture_init(Uint32 max_textures);
Texture *gf3d_texture_load(char *filename);
void gf3d_texture_free(Texture *tex);

/* deprecated */
Texture *gf3d_texture_from_surface(Texture *tex, SDL_Surface *surface);

TextureUI *gf3d_texture_surface_init(SDL_Surface *tex_surface);

void gf3d_texture_surface_update(TextureUI *tex, SDL_Surface *tex_surface);

void gf3d_texture_ui_free(TextureUI *tex);

void gf3d_texture_copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

#endif
