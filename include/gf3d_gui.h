#ifndef _GF3D_GUI_H_
#define _GF3D_GUI_H_

#include "gf3d_shape.h"
#include "gfc_color.h"

typedef struct
{
    Color color;
    Vector2D position;
} GuiVertex;

/* All gui elements will be squares */
typedef struct
{
    Uint8 _inuse;       
    Shape transform;    /* to define position and extents. Model and modelMat should be NULL */
    GuiVertex vertices[4];

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    Uint16 indices[6];
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    
    VkImage image;
    VkImageView imageView;
    
    Color color;
} GuiElement;

/*
 * @brief initialize the gui manager
 * @param count : amount of gui elements to allocate for
 */
void gf3d_gui_manager_init(Uint32 count, Uint32 chain_length, VkDevice device);

/* 
 * @brief draw all guis
 */
void gf3d_gui_manager_draw(Uint32 bufferFrame, VkCommandBuffer commandBuffer);

/* 
 * @brief create a new gui element
 * @param position : the position of the element on screen
 * @param extents  : width and height of element
 * @param color    : the color of the element
 */
GuiElement *gf3d_gui(Vector2D position, Vector2D extents, Color color);

/* 
 * @brief free gui element 
 * @param gui : the gui element to free
 */
void gf3d_gui_free(GuiElement *gui);

#endif