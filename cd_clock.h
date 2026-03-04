#ifndef CD_CLOCK_H_
#define CD_CLOCK_H_

#include "types.h"

typedef struct cd_clock {
    f64 start_time;
    f64 elapsed;
} cd_clock;

void cd_clock_update(cd_clock* clock);
void cd_clock_start(cd_clock* clock);
void cd_clock_stop(cd_clock* clock);

#endif /* CD_CLOCK_H_ */
