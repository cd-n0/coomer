#include "cd_clock.h"
#include "unix_clock.c"
#include "win32_clock.c"

void cd_clock_update(cd_clock* clock) {
    if (clock->start_time != 0) {
        clock->elapsed = platform_get_absolute_time() - clock->start_time;
    }
}

void cd_clock_start(cd_clock* clock) {
    clock->start_time = platform_get_absolute_time();
    clock->elapsed = 0;
}

void cd_clock_stop(cd_clock* clock) {
    clock->start_time = 0;
}
