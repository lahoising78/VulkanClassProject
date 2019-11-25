#ifndef _GF3D_VEC_H_
#define _GF3D_VEC_H_

#include "simple_json.h"
#include "gfc_vector.h"

Vector2D gf3d_vec2_load(SJson *jArr);
Vector3D gf3d_vec3_load(SJson *jArr);
Vector4D gf3d_vec4_load(SJson *jArr);

#endif