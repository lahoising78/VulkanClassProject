#include "gf3d_vec.h"

Vector2D gf3d_vec2_load(SJson *jArr)
{
    SJson *obj = NULL;
    Vector2D vec = {0};

    if(!jArr || !sj_is_array(jArr)) return vec;
    obj = sj_array_get_nth(jArr, 0);
    sj_get_float_value(obj, &vec.x);
    obj = sj_array_get_nth(jArr, 1);
    sj_get_float_value(obj, &vec.y);

    return vec;
}

Vector3D gf3d_vec3_load(SJson *jArr)
{
    SJson *obj = NULL;
    Vector3D vec;

    if(!jArr || !sj_is_array(jArr)) return vec;
    obj = sj_array_get_nth(jArr, 0);
    sj_get_float_value(obj, &vec.x);
    obj = sj_array_get_nth(jArr, 1);
    sj_get_float_value(obj, &vec.y);
    obj = sj_array_get_nth(jArr, 2);
    sj_get_float_value(obj, &vec.z);

    return vec;
}

Vector4D gf3d_vec4_load(SJson *jArr)
{
    SJson *obj = NULL;
    Vector4D vec;

    if(!jArr || !sj_is_array(jArr)) return vec;
    obj = sj_array_get_nth(jArr, 0);
    sj_get_float_value(obj, &vec.x);
    obj = sj_array_get_nth(jArr, 1);
    sj_get_float_value(obj, &vec.y);
    obj = sj_array_get_nth(jArr, 2);
    sj_get_float_value(obj, &vec.z);
    obj = sj_array_get_nth(jArr, 3);
    sj_get_float_value(obj, &vec.w);

    return vec;
}