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
void gf3d_gui_update(Gui *gui, SDL_Event *events);
void gf3d_gui_draw(Gui *gui, uint32_t bufferFrame, VkCommandBuffer commandBuffer);
// void gf3d_gui_draw(Gui *gui, VkDescriptorSet *descriptorSet, VkCommandBuffer commandBuffer);
// void gf3d_gui_create_vertex_buffer(Gui *gui);
// void gf3d_gui_create_index_buffer(Gui *gui);
// void gf3d_gui_update_descriptor_set(Gui* gui, VkDescriptorSet *descriptorSet);
// void gf3d_gui_update_vertices(Gui *gui);

void gf3d_gui_manager_attach_pipe(Pipeline *pipe)
{
    gf3d_gui.pipe = pipe;
}

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
        gui->surface = NULL;
        if(gui->renderer) SDL_DestroyRenderer(gui->renderer);
        gui->renderer = NULL;
        if(gui->ui_tex) gf3d_texture_ui_free(gui->ui_tex);
        gui->ui_tex = NULL;
        gui->_inuse = 0;

        gf3d_gui_element_free(gui->bg);
    }

    free(gf3d_gui.gui_list);
    gf3d_gui.count = 0;
}

void gf3d_gui_manager_init(Uint32 count, VkDevice device)
{
    gf3d_gui.gui_list = (Gui*)gfc_allocate_array(sizeof(Gui), count);
    if(!gf3d_gui.gui_list)
    {
        slog("Could not allocate gui list");
        return;
    }

    gf3d_gui.count = count;

    gf3d_gui.device = device;

    gf3d_gui.bindingDescription.binding = 1;
    gf3d_gui.bindingDescription.stride = sizeof(GuiVertex);
    gf3d_gui.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    gf3d_gui.attributeDescriptions[0].binding = 1;
    gf3d_gui.attributeDescriptions[0].location = 0;
    gf3d_gui.attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    gf3d_gui.attributeDescriptions[0].offset = offsetof(GuiVertex, pos);
    
    gf3d_gui.attributeDescriptions[1].binding = 1;
    gf3d_gui.attributeDescriptions[1].location = 1;
    gf3d_gui.attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    gf3d_gui.attributeDescriptions[1].offset = offsetof(GuiVertex, color);
    
    gf3d_gui.attributeDescriptions[2].binding = 1;
    gf3d_gui.attributeDescriptions[2].location = 2;
    gf3d_gui.attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    gf3d_gui.attributeDescriptions[2].offset = offsetof(GuiVertex, texel);

    atexit(gf3d_gui_manager_close);
}

void gf3d_gui_manager_update(SDL_Event *events)
{
    int i;
    Gui *gui = NULL;

    for(i = 0; i < gf3d_gui.count; i++)
    {
        gui = &gf3d_gui.gui_list[i];
        if(!gui->_inuse) continue;
        gf3d_gui_update(gui, events);
    }
}

void gf3d_gui_manager_draw(Uint32 bufferFrame, VkCommandBuffer commandBuffer)
{
    int i;
    Gui *gui;
    // VkDescriptorSet *descriptorSets = NULL;
    
    // if(gf3d_gui.pipe && gf3d_gui.pipe->descriptorSets) 
    // {
    //     descriptorSets = gf3d_gui.pipe->descriptorSets[bufferFrame];
    // }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gf3d_gui.pipe->pipeline);

        for(i = 0; i < gf3d_gui.count; i++)
        {
            gui = &gf3d_gui.gui_list[i];
            if(!gui->_inuse) continue;

            gf3d_gui_draw(gui, bufferFrame, commandBuffer);
        }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gf3d_vgraphics_get_graphics_pipeline()->pipeline);
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
        gui->elements = (HudElement*)gfc_allocate_array(sizeof(HudElement), count);
        if(!gui->elements)
        {
            slog("unable to allocate enough elements");
            return NULL;
        }
        gui->elementCount = count;

        if(!gui->bg) gui->bg = gf3d_gui_element_create(
            vector2d(0.0f, 0.0f),
            vector2d(gf3d_vgraphics_get_view_extent().width, gf3d_vgraphics_get_view_extent().height),
            vector4d(50.0f, 50.0f, 50.0f, 255.0f)
        );

        gui->_inuse = 1;
        return gui;
    }

    return NULL;
}

void gf3d_gui_free(Gui *gui)
{
    int i;
    HudElement *element = NULL;
    SDL_Surface *surface;
    SDL_Renderer *renderer;
    GuiElement *bg;

    slog("gui free");
    if(!gui) return;

    for(i = 0; i < gui->elementCount; i++)
    {
        element = &gui->elements[i];
        gf3d_hud_element_free(element);
        memset(element, 0, sizeof(HudElement));
    }

    if(gui->elements) free(gui->elements);

    bg = gui->bg;
    surface = gui->surface;
    renderer = gui->renderer;

    memset(gui, 0, sizeof(Gui));

    /* Do this to save the surface and renderer */
    gui->bg = bg;
    gui->surface = surface;
    gui->renderer = renderer;
}

void gf3d_hud_add_element(Gui *gui, HudElement element)
{
    int i;

    if(!gui) return;

    for(i = 0; i < gui->elementCount; i++)
    {
        if(gui->elements[i].type) continue;
        gui->elements[i] = element;
        slog("hud type: %d", gui->elements[i].type);
        return;
    }
}

void gf3d_gui_init(Gui *gui)
{
    VkExtent2D ext = gf3d_vgraphics_get_view_extent();

    if(!gui->surface) gui->surface = SDL_CreateRGBSurface(
        0,
        ext.width,
        ext.height,
        32,
        rmask, gmask, bmask, amask
    );

    if(!gui->renderer) gui->renderer = SDL_CreateSoftwareRenderer(gui->surface);
}

void gf3d_gui_update(Gui *gui, SDL_Event *events)
{
    int i;
    HudElement *e = NULL;

    if(!gui) return;
    gf3d_gui_element_update(gui->bg);

    for(i = 0; i < gui->elementCount; i++)
    {
        e = &gui->elements[i];
        if(!e->type) continue;
        gf3d_hud_element_update(e, events);
    }
}

// void gf3d_gui_draw(Gui *gui, VkDescriptorSet *descriptorSet, VkCommandBuffer commandBuffer)
void gf3d_gui_draw(Gui *gui, uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    int i;

    if(!gui || !gui->renderer) return;

    gf3d_gui_element_draw(gui->bg, bufferFrame, commandBuffer);

    for(i = 0; i < gui->elementCount; i++)
    {
        if(gui->elements[i].type) 
        {
            gf3d_hud_element_draw(&gui->elements[i], bufferFrame, commandBuffer);
        }
    }
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