#ifndef RENDER_H_
#define RENDER_H_

#include "types.h"

b8 render_initialize(void);
void render_deinitialize(void);
void render_draw(void);
void render_process_resize(u32 x, u32 y);

#endif /* RENDER_H_ */
