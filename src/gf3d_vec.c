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

SJson *gf3d_vec2_json(Vector2D vec)
{
    SJson *arr = NULL;
    SJson *obj = NULL;

    arr = sj_array_new();
    if(!arr) return NULL;

    obj = sj_new_float(vec.x);
    sj_array_append(arr, obj);
    obj = sj_new_float(vec.y);
    sj_array_append(arr, obj);

    return arr;
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

SJson *gf3d_vec3_json(Vector3D vec)
{
    SJson *arr = NULL;
    SJson *obj = NULL;

    arr = gf3d_vec2_json(vector2d(vec.x, vec.y));
    if(!arr) return NULL;

    obj = sj_new_float(vec.z);
    sj_array_append(arr, obj);

    return arr;
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

SJson *gf3d_vec4_json(Vector4D vec)
{
    SJson *arr = NULL;
    SJson *obj = NULL;

    arr = gf3d_vec3_json(vector3d(vec.x, vec.y, vec.z));
    if(!arr) return NULL;

    obj = sj_new_float(vec.w);
    sj_array_append(arr, obj);

    return arr;
}