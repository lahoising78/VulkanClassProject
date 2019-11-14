#include "gf3d_gui.h"

#include "simple_logger.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    static Uint32 rmask = 0xff000000;
    static Uint32 gmask = 0x00ff0000;
    static Uint32 bmask = 0x0000ff00;
    static Uint32 amask = 0x000000ff;
#else
    static Uint32 rmask = 0x000000ff;
    static Uint32 gmask = 0x0000ff00;
    static Uint32 bmask = 0x00ff0000;
    static Uint32 amask = 0xff000000;
#endif

// extern Texture *ui_tex;
#define _UI_ATTRIBUTE_COUNT_ 3

typedef struct 
{
    Uint32 count;
    Gui *gui_list;

    Pipeline *pipe;
    VkDevice device;

    VkVertexInputAttributeDescription attributeDescriptions[_UI_ATTRIBUTE_COUNT_];
    VkVertexInputBindingDescription bindingDescription;
} GuiManager;

static GuiManager gf3d_gui = {0};

void gf3d_gui_init(Gui *gui);

void gf3d_gui_manager_close()
{
    int i;
    Gui *gui = NULL;

    slog("gui manager close");

    if(!gf3d_gui.gui_list) return;

    for(i = 0; i < gf3d_gui.count; i++)
    {
        gui = &gf3d_gui.gui_list[i];
        gf3d_gui_free(gui);
        if(gui->surface) SDL_FreeSurface(gui->surface);
        if(gui->renderer) SDL_DestroyRenderer(gui->renderer);
        gui->surface = NULL;
        gui->renderer = NULL;
        gui->_inuse = 0;
    }

    free(gf3d_gui.gui_list);
    gf3d_gui.count = 0;
}

void gf3d_gui_manager_init(Uint32 count, Pipeline *pipe, VkDevice device)
{
    gf3d_gui.gui_list = (Gui*)gfc_allocate_array(sizeof(Gui), count);
    if(!gf3d_gui.gui_list)
    {
        slog("Could not allocate gui list");
        return;
    }

    gf3d_gui.count = count;

    gf3d_gui.device = device;
    gf3d_gui.pipe = pipe;

    gf3d_gui.bindingDescription.binding = 1;
    gf3d_gui.bindingDescription.stride = sizeof(GuiVertex);
    gf3d_gui.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    gf3d_gui.attributeDescriptions[0].binding = 1;
    gf3d_gui.attributeDescriptions[0].location = 0;
    gf3d_gui.attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    gf3d_gui.attributeDescriptions[0].offset = offsetof(GuiVertex, pos);
    
    gf3d_gui.attributeDescriptions[1].binding = 1;
    gf3d_gui.attributeDescriptions[1].location = 1;
    gf3d_gui.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    gf3d_gui.attributeDescriptions[1].offset = offsetof(GuiVertex, color);
    
    gf3d_gui.attributeDescriptions[2].binding = 1;
    gf3d_gui.attributeDescriptions[2].location = 2;
    gf3d_gui.attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    gf3d_gui.attributeDescriptions[2].offset = offsetof(GuiVertex, texel);

    atexit(gf3d_gui_manager_close);
}

void gf3d_gui_manager_draw(Uint32 bufferFrame, VkCommandBuffer commandBuffer)
{
    int i;
    Gui *gui;

    // slog("draw gui");
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gf3d_gui.pipe->pipeline);

        vkCmdBindDescriptorSets(
            commandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            gf3d_gui.pipe->pipelineLayout,
            0,
            1, 
            &gf3d_gui.pipe->descriptorSets[0][bufferFrame],
            0, NULL
        );

        for(i = 0; i < gf3d_gui.count; i++)
        {
            gui = &gf3d_gui.gui_list[i];
            if(!gui->_inuse) continue;

            gf3d_gui_draw(gui, bufferFrame, commandBuffer);
        }

        vkCmdDraw(commandBuffer, 4, 1, 0, 0);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gf3d_vgraphics_get_graphics_pipeline()->pipeline);
    // slog("draw end");
}

Gui *gf3d_gui_new(Uint32 count, int depth)
{
    int i;
    Gui *gui = NULL;

    for(i = 0; i < gf3d_gui.count; i++)
    {
        gui = &gf3d_gui.gui_list[i];
        if(gui->_inuse) continue;

        gf3d_gui_init(gui);
        gui->elements = (GuiElement**)gfc_allocate_array(sizeof(GuiElement*), count);
        if(!gui->elements)
        {
            slog("unable to allocate enough elements");
            return NULL;
        }
        gui->elementCount = count;

        gui->_inuse = 1;
        return gui;
    }

    return NULL;
}

void gf3d_gui_free(Gui *gui)
{
    int i;
    GuiElement *element = NULL;

    slog("gui free");
    if(!gui) return;

    if(gui->elements) free(gui->elements);

    memset(gui, 0, sizeof(Gui));
}

void gf3d_gui_add_element(Gui *gui, GuiElement *element)
{
    int i;

    if(!gui) return;

    for(i = 0; i < gui->elementCount; i++)
    {
        if(gui->elements[i]) continue;
        gui->elements[i] = element;
        return;
    }
}

void gf3d_gui_init(Gui *gui)
{
    gui->surface = SDL_CreateRGBSurface(
        0,
        gf3d_vgraphics_get_view_extent().width,
        gf3d_vgraphics_get_view_extent().height,
        32,
        rmask, gmask, bmask, amask
    );

    gui->renderer = SDL_CreateSoftwareRenderer(gui->surface);
}

void gf3d_gui_draw(Gui *gui, Uint32 bufferFrame, VkCommandBuffer commandBuffer)
{
    int i;

    if(!gui || !gui->renderer) return;

    SDL_RenderClear(gui->renderer);

    for(i = 0; i < gui->elementCount; i++)
    {
        if(gui->elements[i]) gf3d_gui_element_draw(*gui->elements[i], gui->renderer);
    }

    SDL_RenderPresent(gui->renderer);
}

VkVertexInputBindingDescription *gf3d_gui_get_bind_description()
{
    return &gf3d_gui.bindingDescription;
}

VkVertexInputAttributeDescription *gf3d_gui_get_attribute_descriptions(Uint32 *count)
{
    if(count) *count = _UI_ATTRIBUTE_COUNT_;
    return gf3d_gui.attributeDescriptions;
}