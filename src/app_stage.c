#include "app_stage.h"

#include "simple_logger.h"
#include "gf3d_vec.h"

Stage app_stage_load(const char *filename)
{
    Stage stage = {0};

    SJson *json = NULL;
    SJson *obj = NULL;
    SJson *arr = NULL;
    TextLine assetname;

    int i;
    int c;

    slog("-------------- stage load --------------");

    snprintf(assetname, GFCLINELEN, "paks/%s.json", filename);
    json = sj_load(assetname);
    if(!json)
    {
        return stage;
    }

    obj = sj_object_get_value(json, "floor");
    stage.floor = gf3d_entity_load(obj);
    gfc_matrix_make_translation(stage.floor->modelMat, stage.floor->position);
    gf3d_model_scale(stage.floor->modelMat, stage.floor->scale);
    gfc_matrix_rotate(stage.floor->modelMat, stage.floor->modelMat, (stage.floor->rotation.x + 90) * GFC_DEGTORAD, vector3d(0, 0, 1));
    gfc_matrix_rotate(stage.floor->modelMat, stage.floor->modelMat, (stage.floor->rotation.y) * GFC_DEGTORAD, vector3d(1, 0, 0));
    gfc_matrix_rotate(stage.floor->modelMat, stage.floor->modelMat, (stage.floor->rotation.z) * GFC_DEGTORAD, vector3d(0, 1, 0));

    obj = sj_object_get_value(json, "skybox");
    if( obj )
    {
        stage.skybox = gf3d_entity_load(obj);
        if( stage.skybox )
        {
            gfc_matrix_make_translation(stage.skybox->modelMat, stage.skybox->position);
            gf3d_model_scale(stage.skybox->modelMat, stage.skybox->scale);
            gfc_matrix_rotate(stage.skybox->modelMat, stage.skybox->modelMat, (stage.skybox->rotation.x + 90) * GFC_DEGTORAD, vector3d(0, 0, 1));
            gfc_matrix_rotate(stage.skybox->modelMat, stage.skybox->modelMat, (stage.skybox->rotation.y) * GFC_DEGTORAD, vector3d(1, 0, 0));
            gfc_matrix_rotate(stage.skybox->modelMat, stage.skybox->modelMat, (stage.skybox->rotation.z) * GFC_DEGTORAD, vector3d(0, 1, 0));
        }
    }
    
    obj = sj_object_get_value(json, "stageObjectCount");
    if(obj)
    {
        sj_get_integer_value(obj, &c);
        stage.count = (uint32_t)c;
        stage.stageObjects = (Entity**)gfc_allocate_array(sizeof(Entity*), stage.count);
    }

    arr = sj_object_get_value(json, "stageObjects");
    if(arr)
    {
        for(i = 0; i < stage.count; i++)
        {
            obj = sj_array_get_nth(arr, i);
            if(!obj) break;

            stage.stageObjects[i] = gf3d_entity_load(obj);
            stage.stageObjects[i]->update = gf3d_entity_general_update;
        }
    }

    sj_free(json);
    return stage;
}

void app_stage_free(Stage *stage)
{
    int i;
    for(i = 0; i < stage->count; i++)
    {
        gf3d_entity_free(stage->stageObjects[i]);
    }
    free(stage->stageObjects);
    stage->stageObjects = NULL;
    stage->count = 0;
    gf3d_entity_free(stage->skybox);
    stage->skybox = NULL;
    gf3d_entity_free(stage->floor);
    stage->floor = NULL;
}