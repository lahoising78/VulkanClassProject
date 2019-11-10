#include "gf3d_gui.h"

#include "gf3d_vgraphics.h"

typedef struct
{
    // Uint32 max_elements;
    // GuiElement *elements;
    Uint32 chain_length;
    VkDevice device;
    Pipeline *pipe;
} GuiManager;

GuiManager gf3d_gui_manager = {0};

void gf3d_gui_manager_close()
{

}

void gf3d_gui_manager_init(Uint32 count, Uint32 chain_length, VkDevice device)
{
    // gf3d_gui_manager.elements = (GuiElement*)gfc_allocate_array(sizeof(GuiElement), count);
    // if(!gf3d_gui_manager.elements)
    // {
    //     slog("failed to allocate gui elements");
    //     return;
    // }
    // // memset(gf3d_gui_manager.elements, 0, sizeof(GuiElement) * count);
    // gf3d_gui_manager.max_elements = count;
    gf3d_gui_manager.chain_length = chain_length;
    gf3d_gui_manager.device = device;
    gf3d_gui_manager.pipe = gf3d_vgraphics_get_graphics_pipeline2D();
    atexit(gf3d_gui_manager_close);
}

GuiElement gf3d_gui(Vector2D position, Vector2D extents, Color color)
{
    GuiElement element = {0};

    element._inuse = 1;
    element.transform = gf3d_shape(
        vector3d(position.x, position.y, 0),
        vector3d(extents.x, extents.y, 0),
        NULL
    );

    element.vertices[0].color = color;
    element.vertices[0].position = position;
    
    element.vertices[1].color = color;
    element.vertices[1].position = vector2d(position.x + extents.x, position.y);
    
    element.vertices[2].color = color;
    element.vertices[2].position = vector2d(position.x + extents.x, position.y + extents.y);
    
    element.vertices[3].color = color;
    element.vertices[3].position = vector2d(position.x, position.y + extents.y);

    element.color = color;

    return element;

    // int i;

    // for(i = 0; i < gf3d_gui_manager.max_elements; i++)
    // {
    //     if(gf3d_gui_manager.elements[i]._inuse) continue;

    //     element = &gf3d_gui_manager.elements[i];
    //     memset(element, 0, sizeof(GuiElement));
    //     element->transform = gf3d_shape(
    //         vector3d(position.x, position.y, 0),
    //         vector3d(extents.x, extents.y, 0),
    //         NULL
    //     );
    //     element->color = color;
    //     element->_inuse = 1;
    // }
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