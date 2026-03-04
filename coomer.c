#include "globals.h"
#include "platform.h"
#include "render.h"

int main(void) {
    platform_initialize("coomer", false);
    render_initialize();
    while (running) {
        platform_pump_messages();
        render_draw();
    }
    render_deinitialize();
    platform_deinitialize();
}
