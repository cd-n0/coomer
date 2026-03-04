#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "types.h"
#include <GL/gl.h>

typedef struct camera_s {
    point_t position;
    f32 scale;
    b8 panning;
} camera_t;

extern GLuint shader;
extern camera_t camera;
extern frame_t image;
extern frame_t window_size;
extern point_t mouse_position;
extern b8 running;

#endif /* GLOBALS_H_ */
