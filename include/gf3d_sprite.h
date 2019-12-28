#ifndef _GF3D_SPRITE_H_
#define _GF3D_SPRITE_H_

#include "gf3d_texture.h"
#include "gfc_vector.h"

typedef struct sprite_t
{
    Texture *texture;
    Vector4D color;
} Sprite;

#endif