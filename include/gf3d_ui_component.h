#ifndef _GF3D_UI_COMPONENT_H_
#define _GF3D_UI_COMPONENT_H_

#include "gf3d_texture.h"
#include "gfc_matrix.h"
#include "gfc_vector.h"
#include <vulkan/vulkan.h>

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
    Vector2D                    position;
    Vector2D                    extents;
    Vector2D                    rotation;

    Matrix4                     mat;

    VkBuffer                    uniformBuffer;
    VkDeviceMemory              uniformBufferMemory;
    VkBuffer                    vertexBuffer;
    VkDeviceMemory              vertexBufferMemory;

    Texture                     *texture;
    SDL_Surface                 *surface;
    SDL_Renderer                *renderer;

    uint8_t                     active;
    uint8_t                     visible;

    uint8_t                     _inuse;
} uiComponent;

void gf3d_ui_component_manager_init();

void gf3d_ui_component_init( uiComponent *ui );
void gf3d_ui_component_update( uiComponent *ui );
void gf3d_ui_component_free( uiComponent *ui );
void gf3d_ui_component_attach_texture_from_file( uiComponent *ui, const char *filename );
void gf3d_ui_component_render( uiComponent *ui, uint32_t bufferFrame, VkCommandBuffer commandBuffer );

#endif