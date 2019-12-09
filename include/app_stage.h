#ifndef _APP_STAGE_H_
#define _APP_STAGE_H_

#include "simple_json.h"
#include "gf3d_entity.h"

typedef struct 
{
    Entity **stageObjects;
    uint32_t count;
    Entity *floor;
    Entity *fighters[2];
} Stage;

Stage app_stage_load(const char *filename);

void app_stage_free(Stage *stage);

#endif