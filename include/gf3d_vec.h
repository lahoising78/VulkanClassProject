#ifndef _GF3D_VEC_H_
#define _GF3D_VEC_H_

#include "simple_json.h"
#include "gfc_vector.h"

Vector2D gf3d_vec2_load(SJson *jArr);
SJson *gf3d_vec2_json(Vector2D vec);

Vector3D gf3d_vec3_load(SJson *jArr);
SJson *gf3d_vec3_json(Vector3D vec);

Vector4D gf3d_vec4_load(SJson *jArr);
SJson *gf3d_vec4_json(Vector4D vec);

#endif