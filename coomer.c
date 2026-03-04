#include "globals.h"
#include "platform.h"
#include "render.h"
#include <stdbool.h>

int main(void) {
    platform_initialize("coomer", false);
    render_initialize();
    while (running) {
        platform_pump_messages();
        render_update();
        render_draw();
    }
    render_deinitialize();
    platform_deinitialize();
}
