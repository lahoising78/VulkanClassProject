#include "gf3d_gui.h"

#include "simple_logger.h"
#include "gf3d_texture.h"

#define MAX_ELEMENTS 64

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

typedef struct
{
    uint32_t max_layers;
    GuiLayer *layer_list;

    Texture *square_tex;
} GuiManager;

static GuiManager gf3d_gui_manager = {0};

void gf3d_gui_layer_draw(GuiLayer *gui, uint32_t bufferFrame, VkCommandBuffer commandBuffer);

void gf3d_gui_manager_close()
{
    int i;
    GuiLayer *gui = NULL;

    slog("clean gui manager");

    for(i = 0; i < gf3d_gui_manager.max_layers; i++)
    {
        gui = &gf3d_gui_manager.layer_list[i];
        gf3d_gui_layer_free(gui);
        if(gui->elements) free(gui->elements);
        gui->elements = NULL;
    }

    if(gf3d_gui_manager.layer_list) free(gf3d_gui_manager.layer_list);
    gf3d_gui_manager.max_layers = 0;
}

void gf3d_gui_manager_init(uint32_t count)
{
    int i;
    SDL_Surface *surface = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Rect rect = {0};

    gf3d_gui_manager.layer_list = (GuiLayer*)gfc_allocate_array(sizeof(GuiLayer), count);
    if(!gf3d_gui_manager.layer_list)
    {
        slog("could not allocate space for layer list");
        return;
    }

    for(i = 0; i < count; i++)
    {
        gf3d_gui_manager.layer_list[i].elements = (GuiElement**)gfc_allocate_array(sizeof(GuiElement*), MAX_ELEMENTS);
    }

    gf3d_gui_manager.max_layers = count;

    atexit(gf3d_gui_manager_close);

    surface = SDL_CreateRGBSurface(
        0,
        1, 1, 1,
        rmask, gmask, bmask, amask
    );
    if(!surface) return;

    renderer = SDL_CreateSoftwareRenderer(surface);
    if(!renderer) return;

    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    rect.x = rect.y = 0;
    rect.w = rect.h = 1;
    SDL_RenderFillRect(renderer, &rect);

    gf3d_gui_manager.square_tex = gf3d_texture_from_surface(surface);
    gf3d_gui_element_set_square_tex(gf3d_gui_manager.square_tex);

    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
}

void gf3d_gui_manager_draw(uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    int i;
    GuiLayer *gui = NULL;

    for(i = 0; i < gf3d_gui_manager.max_layers; i++)
    {
        gui = &gf3d_gui_manager.layer_list[i];
        if(!gui->_inuse) continue;
        gf3d_gui_layer_draw(gui, bufferFrame, commandBuffer);
    }
}

void gf3d_gui_layer_free(GuiLayer *gui)
{
    slog("free gui layer");

    if(!gui) return;

    gui->_inuse = 0;
}

GuiLayer *gf3d_gui_layer_new()
{
    int i;
    GuiLayer *gui = NULL;
    GuiElement **arr = NULL;

    for(i = 0; i < gf3d_gui_manager.max_layers; i++)
    {
        gui = &gf3d_gui_manager.layer_list[i];
        if(!gui || gui->_inuse) continue;

        /* clear vals but preserve the array */
        arr = gui->elements;
        memset(gui, 0, sizeof(GuiLayer));
        gui->elements = arr;

        gui->_inuse = 1;

        return gui;
    }

    return NULL;
}

int gf3d_gui_layer_add_element(GuiLayer *gui, GuiElement *element)
{
    int i = 0;

    if(!gui || !element) return -1;
    if(!gui->elements) return -1;

    // for(i = 0; i < gui->elementCount; i++)
    for(i = 0; i < MAX_ELEMENTS; i++)
    {
        if(gui->elements[i]) continue;
        gui->elements[i] = element;
        return i;
    }

    return -1;
}

void gf3d_gui_layer_draw(GuiLayer *gui, uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    int i;
    VkImage *image = NULL;
    if(!gui || !gui->elements) return;

    image = gf3d_swapchain_get_image(bufferFrame);

    for(i = 0; i < MAX_ELEMENTS; i++)
    {
        gf3d_gui_element_draw(gui->elements[i], commandBuffer, image);
    }
}