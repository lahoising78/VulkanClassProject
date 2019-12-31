#include "gf3d_ui.h"

void gf3d_ui_free(UI *ui)
{
    if(!ui) return;
    memset(ui, 0, sizeof(UI));
}