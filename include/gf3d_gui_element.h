#ifndef _GF3D_GUI_ELEMENT_H_
#define _GF3D_GUI_ELEMENT_H_

#include "gfc_vector.h"

typedef struct Square_S
{
    Vector2D position;
    Vector2D extents;
    Vector2D uv[4];
} Square;

Square gf3d_gui_element_square( Vector2D position, Vector2D extents, Vector2D *uv );

#endif