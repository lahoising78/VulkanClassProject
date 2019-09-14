#include <string.h>
#include <stdio.h>
#include <vulkan/vulkan.h>

#include "simple_logger.h"
#include "gfc_types.h"
#include "gfc_vector.h"
#include "gf3d_validation.h"



// validation layers
typedef struct
{
    Uint32 layerCount;
    VkLayerProperties *availableLayers;
    const char* * layerNames;
}vValidation;

static vValidation gf3d_validation = {0};

void gf3d_validation_query_layer_properties()
{
    int i, j;
    Uint32 found = 0;
    int displacement = 0;
    vValidation temp = {0};
    char* ignoreLayers[] = {"VK_LAYER_LUNARG_vktrace", "VK_LAYER_LUNARG_api_dump", "VK_LAYER_LUNARG_demo_layer"};
    const unsigned int ignoreLayersCount = sizeof(ignoreLayers) / sizeof(char*);

    vkEnumerateInstanceLayerProperties(&temp.layerCount, NULL);
    slog("discovered %i validation layers",temp.layerCount);
    
    if (!temp.layerCount)return;
    
    temp.availableLayers = (VkLayerProperties *)gfc_allocate_array(sizeof(VkLayerProperties),temp.layerCount);
    vkEnumerateInstanceLayerProperties(&temp.layerCount, temp.availableLayers);
    
    temp.layerNames = (const char* * )gfc_allocate_array(sizeof(const char *),temp.layerCount);
    for (i = 0; i < temp.layerCount;i++)
    {

        temp.layerNames[i] = (const char *)temp.availableLayers[i].layerName;
        slog("Validation layer available: [%d] - %s", i, temp.availableLayers[i].layerName);
        
        for (j = 0; j < ignoreLayersCount; j++)
        {
            if ( strcmp(temp.availableLayers[i].layerName, ignoreLayers[j]) == 0 )
            {
                found++;
                slog("Skipped %s", temp.availableLayers[i].layerName);
                strcpy( temp.availableLayers[i].layerName, "" );
                break;
            }
        }

    }

    slog("---- Adding final list of validation layers ----");
    gf3d_validation.layerCount = temp.layerCount - found;
    slog("gf3d: %d, temp: %d", gf3d_validation.layerCount, temp.layerCount);
    gf3d_validation.availableLayers = (VkLayerProperties *)gfc_allocate_array(sizeof(VkLayerProperties), gf3d_validation.layerCount);
    gf3d_validation.layerNames = (const char* *)gfc_allocate_array(sizeof(const char *), gf3d_validation.layerCount);
    for (i = 0; i < temp.layerCount; i++)
    {
        if ( strcmp(temp.availableLayers[i].layerName, "") == 0 )
        {
            displacement++;
            continue;
        }

        gf3d_validation.availableLayers[i - displacement] = temp.availableLayers[i];
        gf3d_validation.layerNames[i - displacement] = temp.layerNames[i];
        slog("Validation layer enabled: [%d] - %s", i - displacement, gf3d_validation.availableLayers[i - displacement].layerName);
        
    }
}

void gf3d_validation_close()
{
    if (gf3d_validation.availableLayers)
    {
        free(gf3d_validation.availableLayers);
        gf3d_validation.availableLayers = NULL;
    }
    if (gf3d_validation.layerNames)
    {
        free(gf3d_validation.layerNames);
        gf3d_validation.layerNames = NULL;
    }
    memset(&gf3d_validation,0,sizeof(vValidation));
}

void gf3d_validation_init()
{
    gf3d_validation_query_layer_properties();
    atexit(gf3d_validation_close);
}

Bool gf3d_validation_check_layer_support(char *layerName)
{
    int i;
    for (i = 0; i < gf3d_validation.layerCount;i++)
    {
        if (strcmp(layerName,gf3d_validation.availableLayers[i].layerName) == 0)
        {
            return true;
        }
    }
    return false;
}

Uint32 gf3d_validation_get_validation_layer_count()
{
    return gf3d_validation.layerCount;
}

VkLayerProperties *gf3d_validation_get_validation_layer_data()
{
    return gf3d_validation.availableLayers;
}

const char* const* gf3d_validation_get_validation_layer_names()
{
    return (const char* const* )gf3d_validation.layerNames;
}

/*eol@eof*/
