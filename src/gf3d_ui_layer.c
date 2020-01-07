#include "gf3d_ui_layer.h"
#include "simple_logger.h"

#include <SDL2/SDL.h>
#include "gf3d_vgraphics.h"

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
    UILayer                 *layer_list;
    uint32_t                count;

    Pipeline *pipe;
    VkDevice device;
} UIManager;

static UIManager gf3d_ui_manager = {0};

void gf3d_ui_manager_initialize_list();

void gf3d_ui_draw(UILayer *layer, uint32_t bufferFrame, VkCommandBuffer commandBuffer);

void gf3d_ui_manager_close()
{
    int i;

    slog("close ui system");

    if(gf3d_ui_manager.layer_list) 
    {
        for(i = 0; i < gf3d_ui_manager.count; i++)
        {
            /* TODO: free each layer */
        }

        free(gf3d_ui_manager.layer_list);
        gf3d_ui_manager.layer_list = NULL;
    }

    gf3d_ui_manager.count = 0;
}

void gf3d_ui_manager_initialize_list()
{
    int i;
    UILayer *layer = NULL;
    SDL_Surface *surface = NULL;
    SDL_Renderer *renderer = NULL;
    VkExtent2D ext = gf3d_vgraphics_get_view_extent();
    SDL_Rect rect = {0};

    slog("ui system init list");

    rect.x = rect.y = rect.h = rect.w = 100;
    
    surface = SDL_CreateRGBSurface(
        0,
        ext.width, ext.height,
        32,
        rmask, gmask, bmask, amask
    );

    renderer = SDL_CreateSoftwareRenderer(surface);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);

    for(i = 0; i < gf3d_ui_manager.count; i++)
    {
        layer = &gf3d_ui_manager.layer_list[i];
        memset(layer, 0, sizeof(UILayer));

        gf3d_texture_from_surface(layer->display, surface);

        layer->visible = layer->active = layer->_inuse = 1;
        slog("inuse %d, visible %d, texture %d", layer->_inuse, layer->visible, layer->display);
    }

    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
}

void gf3d_ui_manager_init(uint32_t count)
{
    slog("initialize ui system");

    gf3d_ui_manager.device = gf3d_vgraphics_get_default_logical_device();
    gf3d_ui_manager.pipe = gf3d_vgraphics_get_graphics_pipeline();

    gf3d_ui_manager.layer_list = (UILayer*)gfc_allocate_array(sizeof(UILayer), count);
    if(!gf3d_ui_manager.layer_list)
    {
        slog("unable to initialize ui layers");
        return;
    }
    gf3d_ui_manager.count = count;

    gf3d_ui_manager_initialize_list();
    
    atexit(gf3d_ui_manager_close);
}

void gf3d_ui_manager_draw(uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    int i;
    UILayer *layer = NULL;

    for(i = 0; i < gf3d_ui_manager.count; i++)
    {
        layer = &gf3d_ui_manager.layer_list[i];
        if(!layer->_inuse || !layer->visible) continue;

        gf3d_ui_draw(layer, bufferFrame, commandBuffer);
    }
}

void gf3d_ui_draw(UILayer *layer, uint32_t bufferFrame, VkCommandBuffer commandBuffer)
{
    

    if(!layer) return;

    // slog("ui draw layer");

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, )
}