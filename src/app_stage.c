#include "app_stage.h"

#include "simple_logger.h"
#include "gf3d_vec.h"

Stage app_stage_load(const char *filename)
{
    Stage stage = {0};

    SJson *json = NULL;
    SJson *obj = NULL;
    SJson *val = NULL;
    SJson *thirdLevelVal = NULL;
    TextLine assetname;

    char a[GFCLINELEN];
    char b[GFCLINELEN];

    slog("-------------- stage load --------------");

    snprintf(assetname, GFCLINELEN, "paks/%s.paks", filename);
    json = sj_load(assetname);
    if(!json)
    {
        return stage;
    }

    stage.floor = gf3d_entity_new();
    obj = sj_object_get_value(json, "floor");
    
    val = sj_object_get_value(obj, "position");
    vector2d_copy(stage.floor->position, gf3d_vec2_load(val));
    val = sj_object_get_value(obj, "scale");
    vector2d_copy(stage.floor->scale, gf3d_vec2_load(val));
    val = sj_object_get_value(obj, "model");
    thirdLevelVal = sj_object_get_value(val, "model");
    snprintf(a, GFCLINELEN, "%s", sj_get_string_value(thirdLevelVal));
    thirdLevelVal = sj_object_get_value(val, "texture");
    snprintf(b, GFCLINELEN, "%s", sj_get_string_value(thirdLevelVal));
    stage.floor->model = gf3d_model_load(a, b);

    gfc_matrix_identity(stage.floor->modelMat);
    gfc_matrix_make_translation(stage.floor->modelMat, stage.floor->position);
    gf3d_model_scale(stage.floor->modelMat, stage.floor->scale);

    sj_free(json);
    return stage;
}

void app_stage_free(Stage *stage)
{

}