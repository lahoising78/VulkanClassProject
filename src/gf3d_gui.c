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

typedef struct 
{
    Uint32 count;
    Gui *gui_list;
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

void gf3d_gui_manager_init(Uint32 count)
{
    gf3d_gui.gui_list = (Gui*)gfc_allocate_array(sizeof(Gui), count);
    if(!gf3d_gui.gui_list)
    {
        slog("Could not allocate gui list");
        return;
    }

    gf3d_gui.count = count;
    atexit(gf3d_gui_manager_close);
}

void gf3d_gui_manager_draw()
{
    int i;
    Gui *gui;

    for(i = 0; i < gf3d_gui.count; i++)
    {
        gui = &gf3d_gui.gui_list[i];
        if(!gui->_inuse) continue;

        gf3d_gui_draw(gui);
    }
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

void gf3d_gui_draw(Gui *gui)
{
    int i;

    slog("draw sucia");

    if(!gui || !gui->renderer) return;

    slog("render clear");
    SDL_RenderClear(gui->renderer);

    slog("iterate elements");
    for(i = 0; i < gui->elementCount; i++)
    {
        if(gui->elements[i]) gf3d_gui_element_draw(*gui->elements[i], gui->renderer);
    }

    SDL_RenderPresent(gui->renderer);

    // slog("getting texture");
    // if(!ui_tex) ui_tex = gf3d_texture_from_surface(gui->surface);
    // slog("woop woop");
}