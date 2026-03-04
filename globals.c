#include "globals.h"
#include "types.h"
#include <stdbool.h>

GLuint shader;
camera_t camera = {.scale = 1.f};
frame_t image;
frame_t window_size;
point_t mouse_position;
b8 running = true;
