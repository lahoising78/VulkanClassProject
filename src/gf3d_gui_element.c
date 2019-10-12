#include "gf3d_gui_element.h"

Square gf3d_gui_element_square( Vector2D position, Vector2D extents, Vector2D *uv )
{
    int i;
    Square sq;
    sq.position = position;
    sq.extents = extents;

    if (!uv) 
    {
        for (i = 0; i < 4; i++)
        {
            sq.uv[i] = vector2d(0,0); 
        }
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            sq.uv[i] = uv[i]; 
        }
    } 

    return sq;
}