#include "gf3d_ui_layer.h"
#include "simple_logger.h"

typedef struct
{
    UILayer                 *layer_list;
    uint32_t                count;
} UIManager;

static UIManager gf3d_ui_manager = {0};

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

void gf3d_ui_manager_init(uint32_t count)
{
    slog("initialize ui system");

    gf3d_ui_manager.layer_list = (UILayer*)gfc_allocate_array(sizeof(UILayer), count);
    if(!gf3d_ui_manager.layer_list)
    {
        slog("unable to initialize ui layers");
        return;
    }

    gf3d_ui_manager.count = count;
    
    atexit(gf3d_ui_manager_close);
}