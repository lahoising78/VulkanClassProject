#include "gf3d_gui_element.h"

#include "simple_logger.h"

static Texture *square_tex = NULL;

void gf3d_gui_element_set_square_tex(Texture *tex)
{
    square_tex = tex;
}

ColorRGBA colorRGBA(int r, int g, int b, int a)
{
    ColorRGBA c;

    if(r < 0)
        r = 0;
    else if(r > 255)
        r = 255;

    if(g < 0)
        g = 0;
    else if(g > 255)
        g = 255;

    if(b < 0)
        b = 0;
    else if(b > 255)
        b = 255;

    if(a < 0)
        a = 0;
    else if(a > 255)
        a = 255;

    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;

    return c;
}

GuiElement gf3d_gui_element_create(Vector2D pos, Vector2D extents, ColorRGBA color)
{
    GuiElement e = {0};
    e.position = pos;
    e.extents = extents;
    e.color = color;
    return e;
}

void gf3d_gui_element_draw(GuiElement *e, VkCommandBuffer commandBuffer, VkImage *image)
{
    VkImageBlit blit = {0};
    if(!e || !image) return;

    blit.srcOffsets[0].x = 0;
    blit.srcOffsets[0].y = 0;
    blit.srcOffsets[0].z = 1;

    blit.srcOffsets[1].x = 1;
    blit.srcOffsets[1].y = 1;
    blit.srcOffsets[1].z = 1;

    blit.dstOffsets[0].x = e->position.x;
    blit.dstOffsets[0].y = e->position.y;
    blit.dstOffsets[0].z = 1;
    
    blit.srcOffsets[1].x = e->extents.x;
    blit.srcOffsets[1].y = e->extents.y;
    blit.srcOffsets[1].z = 1;

    vkCmdBlitImage(
        commandBuffer,
        square_tex->textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        *image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &blit,
        0
    );
}