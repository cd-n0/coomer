#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "types.h"

b8 platform_initialize(const char *window_name, b8 windowed);
void platform_deinitialize(void);
b8 platform_screenshot(u8 **raw, u8 *comp, frame_t *size);
b8 platform_get_display_size(frame_t *display_size);
void platform_pump_messages(void);
void platform_gl_swap_buffers(void);

#endif /* PLATFORM_H_ */
