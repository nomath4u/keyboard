/*
Copyright 2011 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef JOY_MOUSE_H
#define  JOY_MOUSEJOY_MOUSE_H

#include <stdbool.h>
#include "host.h"

#define X_AXIS_PIN A9
#define Y_AXIS_PIN A10

/* max value on report descriptor */
//#define JOY_MOUSE_MOVE_MAX       127
#define JOY_MOUSE_MOVE_MAX       255
#define JOY_MOUSE_WHEEL_MAX      127

#ifndef JOY_MOUSE_MOVE_DELTA
#define JOY_MOUSE_MOVE_DELTA     5
#endif
#ifndef JOY_MOUSE_WHEEL_DELTA
#define JOY_MOUSE_WHEEL_DELTA    1
#endif
#ifndef JOY_MOUSE_DELAY
#define JOY_MOUSE_DELAY 100
#endif
#ifndef JOY_MOUSE_INTERVAL
#define JOY_MOUSE_INTERVAL 50
#endif
#ifndef JOY_MOUSE_MAX_SPEED
#define JOY_MOUSE_MAX_SPEED 40
#endif
#ifndef JOY_MOUSE_TIME_TO_MAX
#define JOY_MOUSE_TIME_TO_MAX 20
#endif
#ifndef JOY_MOUSE_WHEEL_MAX_SPEED
#define JOY_MOUSE_WHEEL_MAX_SPEED 8
#endif
#ifndef JOY_MOUSE_WHEEL_TIME_TO_MAX
#define JOY_MOUSE_WHEEL_TIME_TO_MAX 40
#endif


#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t mk_delay;
extern uint8_t mk_interval;
extern uint8_t mk_max_speed;
extern uint8_t mk_time_to_max;
extern uint8_t mk_wheel_max_speed;
extern uint8_t mk_wheel_time_to_max;


void joy_mouse_task(void);
void joy_mouse_on(uint8_t);
void joy_mouse_off(uint8_t);
void joy_mouse_send(void);
void joy_mouse_init(void);

#ifdef __cplusplus
}
#endif

#endif
