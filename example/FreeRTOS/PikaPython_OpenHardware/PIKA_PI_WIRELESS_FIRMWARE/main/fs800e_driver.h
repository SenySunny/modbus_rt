#ifndef _FS800E_DRIVER_H_
#define _FS800E_DRIVER_H_

#include "pika_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define FS800E_COM_MONITOR 1

extern int fs800e_init(void);
extern int fs800e_exit_transparent_mode(void);
extern int fs800e_echo_off(void);
extern int fs800e_at_test(void);

#endif
