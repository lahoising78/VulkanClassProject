#ifndef _GF3D_GUI_ELEMENT_H_
#define _GF3D_GUI_ELEMENT_H_

#include "gfc_vector.h"
#include "gfc_color.h"
#include "gf3d_texture.h"
#include "gf3d_pipeline.h"
#include "simple_json.h"
#include <SDL2/SDL_ttf.h>

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    // static Uint32 rmask = 0xff000000;
    // static Uint32 gmask = 0x00ff0000;
    // static Uint32 bmask = 0x0000ff00;
    // static Uint32 amask = 0x000000ff;
    #define rmask 0xff000000
    #define gmask 0x00ff0000
    #define bmask 0x0000ff00
    #define amask 0x000000ff
#else
    // static Uint32 rmask = 0x000000ff;
    // static Uint32 gmask = 0x0000ff00;
    // static Uint32 bmask = 0x00ff0000;
    // static Uint32 amask = 0xff000000;
    #define rmask 0x000000ff
    #define gmask 0x0000ff00
    #define bmask 0x00ff0000
    #define amask 0xff000000
#endif

enum GuiElementType
{
    GF3D_GUI_RECT = 0,
    GF3D_GUI_IMAGE = 1
};

typedef struct
{
    Vector2D pos;
    Vector4D color;
    Vector2D texel;
} GuiVertex;

typedef struct gui_element_t
{
    Vector2D position;
    Vector2D extents;
    Vector4D color;
    Texture *tex;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    GuiVertex vertices[4];
} GuiElement;

void gf3d_gui_element_manager_init(VkDevice device, Pipeline *pipe);

GuiElement *gf3d_gui_element_create(Vector2D pos, Vector2D ext, Vector4D color);
GuiElement *gf3d_gui_element_load(SJson *json);
SJson *gf3d_gui_element_to_json(GuiElement *e);
void gf3d_gui_element_attach_texture(GuiElement *gui, char *textureName);
void gf3d_gui_element_attach_texture_from_surface(GuiElement *gui, SDL_Surface *surface);

void gf3d_gui_element_free(GuiElement *e);
void gf3d_gui_element_update(GuiElement *e);
void gf3d_gui_element_draw(GuiElement *element, uint32_t bufferFrame, VkCommandBuffer commandBuffer);

TTF_Font *gf3d_gui_element_get_font();
// void gf3d_gui_element_draw(GuiElement *element, SDL_Renderer *renderer);

#endif