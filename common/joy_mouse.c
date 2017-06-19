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

#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include <math.h>
#include "keycode.h"
#include "host.h"
#include "timer.h"
#include "print.h"
#include "debug.h"
#include "joy_mouse.h"

// These prescaler values are for high speed mode, ADHSM = 1
#if F_CPU == 16000000L
#define ADC_PRESCALER ((1<<ADPS2) | (1<<ADPS1))
#elif F_CPU == 8000000L
#define ADC_PRESCALER ((1<<ADPS2) | (1<<ADPS0))
#elif F_CPU == 4000000L
#define ADC_PRESCALER ((1<<ADPS2))
#elif F_CPU == 2000000L
#define ADC_PRESCALER ((1<<ADPS1) | (1<<ADPS0))
#elif F_CPU == 1000000L
#define ADC_PRESCALER ((1<<ADPS1))
#else
#define ADC_PRESCALER ((1<<ADPS0))
#endif
// some avr-libc versions do not properly define ADHSM
#if defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
#if !defined(ADHSM)
#define ADHSM (7)
#endif
#endif

#define DEAD_ZONE_WIDTH 4

static report_mouse_t mouse_report = {};
static uint8_t mousekey_repeat =  0;
static uint8_t mousekey_accel = 0;

static void mousekey_debug(void);
static uint16_t joy_mouse_read_adc(bool);


/*
 * Mouse keys  acceleration algorithm
 *  http://en.wikipedia.org/wiki/Mouse_keys
 *
 *  speed = delta * max_speed * (repeat / time_to_max)**((1000+curve)/1000)
 */
/* milliseconds between the initial key press and first repeated motion event (0-2550) */
uint8_t mk_delay = JOY_MOUSE_DELAY/10;
/* milliseconds between repeated motion events (0-255) */
uint8_t mk_interval = JOY_MOUSE_INTERVAL;
/* steady speed (in action_delta units) applied each event (0-255) */
uint8_t mk_max_speed = JOY_MOUSE_MAX_SPEED;
/* number of events (count) accelerating to steady speed (0-255) */
uint8_t mk_time_to_max = JOY_MOUSE_TIME_TO_MAX;
/* ramp used to reach maximum pointer speed (NOT SUPPORTED) */
//int8_t mk_curve = 0;
/* wheel params */
uint8_t mk_wheel_max_speed = JOY_MOUSE_WHEEL_MAX_SPEED;
uint8_t mk_wheel_time_to_max = JOY_MOUSE_WHEEL_TIME_TO_MAX;


static uint16_t last_timer = 0;


static int8_t move_unit(bool is_9)
{
    int16_t unit;
    //if (mousekey_accel & (1<<0)) {
    //    unit = (JOY_MOUSE_MOVE_DELTA * mk_max_speed)/4;
    //} else if (mousekey_accel & (1<<1)) {
    //    unit = (JOY_MOUSE_MOVE_DELTA * mk_max_speed)/2;
    //} else if (mousekey_accel & (1<<2)) {
    //    unit = (JOY_MOUSE_MOVE_DELTA * mk_max_speed);
    //} else if (mousekey_repeat == 0) {
    //    unit = JOY_MOUSE_MOVE_DELTA;
    //} else if (mousekey_repeat >= mk_time_to_max) {
    //    unit = JOY_MOUSE_MOVE_DELTA * mk_max_speed;
    //} else {
    //    unit = (JOY_MOUSE_MOVE_DELTA * mk_max_speed * mousekey_repeat) / mk_time_to_max;
    //}
    unit = ((joy_mouse_read_adc(is_9)* 25) / 950) - 12;
    //return (unit > JOY_MOUSE_MOVE_MAX ? JOY_MOUSE_MOVE_MAX : (unit == 0 ? 1 : unit));
    if( unit > JOY_MOUSE_MOVE_MAX){ unit = JOY_MOUSE_MOVE_MAX;}
    if( unit < -JOY_MOUSE_MOVE_MAX)  {unit = -JOY_MOUSE_MOVE_MAX;}
    if (abs(unit) < DEAD_ZONE_WIDTH){ unit = 0;}
    return unit;
}

//static uint8_t wheel_unit(void)
//{
//    uint16_t unit;
//    if (mousekey_accel & (1<<0)) {
//        unit = (JOY_MOUSE_WHEEL_DELTA * mk_wheel_max_speed)/4;
//    } else if (mousekey_accel & (1<<1)) {
//        unit = (JOY_MOUSE_WHEEL_DELTA * mk_wheel_max_speed)/2;
//    } else if (mousekey_accel & (1<<2)) {
//        unit = (JOY_MOUSE_WHEEL_DELTA * mk_wheel_max_speed);
//    } else if (mousekey_repeat == 0) {
//        unit = JOY_MOUSE_WHEEL_DELTA;
//    } else if (mousekey_repeat >= mk_wheel_time_to_max) {
//        unit = JOY_MOUSE_WHEEL_DELTA * mk_wheel_max_speed;
//    } else {
//        unit = (JOY_MOUSE_WHEEL_DELTA * mk_wheel_max_speed * mousekey_repeat) / mk_wheel_time_to_max;
//    }
//    return (unit > JOY_MOUSE_WHEEL_MAX ? JOY_MOUSE_WHEEL_MAX : (unit == 0 ? 1 : unit));
//}

//void mousekey_task(void)
//{
//    if (timer_elapsed(last_timer) < (mousekey_repeat ? mk_interval : mk_delay*10))
//        return;
//
//    if (mouse_report.x == 0 && mouse_report.y == 0 && mouse_report.v == 0 && mouse_report.h == 0)
//        return;
//
//    if (mousekey_repeat != UINT8_MAX)
//        mousekey_repeat++;
//
//
//    if (mouse_report.x > 0) mouse_report.x = move_unit();
//    if (mouse_report.x < 0) mouse_report.x = move_unit() * -1;
//    if (mouse_report.y > 0) mouse_report.y = move_unit();
//    if (mouse_report.y < 0) mouse_report.y = move_unit() * -1;
//
//    /* diagonal move [1/sqrt(2) = 0.7] */
//    if (mouse_report.x && mouse_report.y) {
//        mouse_report.x *= 0.7;
//        mouse_report.y *= 0.7;
//    }
//
//    if (mouse_report.v > 0) mouse_report.v = wheel_unit();
//    if (mouse_report.v < 0) mouse_report.v = wheel_unit() * -1;
//    if (mouse_report.h > 0) mouse_report.h = wheel_unit();
//    if (mouse_report.h < 0) mouse_report.h = wheel_unit() * -1;
//
//    mousekey_send();
//}

void joy_mouse_task(void)
{
    if (timer_elapsed(last_timer) < (mousekey_repeat ? mk_interval : mk_delay*10))
        return;

    //if (mouse_report.x == 0 && mouse_report.y == 0 && mouse_report.v == 0 && mouse_report.h == 0)
    //    return;

    if (mousekey_repeat != UINT8_MAX)
        mousekey_repeat++;
    
    mouse_report.x = move_unit(true);
    mouse_report.y = move_unit(false);

    joy_mouse_send();
}


void joy_mouse_on(uint8_t code)
{
    if (code == KC_MS_BTN1)     mouse_report.buttons |= MOUSE_BTN1;
    else if (code == KC_MS_BTN2)     mouse_report.buttons |= MOUSE_BTN2;
}

void joy_mouse_off(uint8_t code)
{
    if (code == KC_MS_BTN1) mouse_report.buttons &= ~MOUSE_BTN1;
    else if (code == KC_MS_BTN2) mouse_report.buttons &= ~MOUSE_BTN2;

    //if (mouse_report.x == 0 && mouse_report.y == 0 && mouse_report.v == 0 && mouse_report.h == 0)
    //    mousekey_repeat = 0;
}

//void mousekey_send(void)
//{
//    mousekey_debug();
//    host_mouse_send(&mouse_report);
//    last_timer = timer_read();
//}

void joy_mouse_send(void)
{
    mousekey_debug();
    host_mouse_send(&mouse_report);
    last_timer = timer_read();
}


void mousekey_clear(void)
{
    mouse_report = (report_mouse_t){};
    mousekey_repeat = 0;
    mousekey_accel = 0;
}

static void mousekey_debug(void)
{
    if (!debug_mouse) return;
    //for(int i = 0; i < 16; i++){
    //	print_dec(joy_mouse_read_adc((uint8_t)i)); print("\n");
    //}
    //print_dec(joy_mouse_read_adc(true)); print("\n");
    print_dec(joy_mouse_read_adc(false)); print("\n");
    //print("mousekey [btn|x y v h](rep/acl): [");
    //phex(mouse_report.buttons); print("|");
    //print_decs(mouse_report.x); print(" ");
    //print_decs(mouse_report.y); print(" ");
    //print_decs(mouse_report.v); print(" ");
    //print_decs(mouse_report.h); print("](");
    //print_dec(mousekey_repeat); print("/");
    //print_dec(mousekey_accel); print(")\n");
}

void joy_mouse_init(void){
    //ADMUX = (1<<REFS1); 
    //ADCSRA = (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    DIDR2 = (1<<1) | (1<<2); //Power saving digital input stuff
    //memset(&mouse_report, 0, sizeof(report_mouse_t));

    /*Maybe need to disable the voltage divider (not sure if anything is hooked up to this*/
    //DDRF &= ~(1<<5);
    //PORTF &= ~(1<<5);

}

uint16_t joy_mouse_read_adc(bool is_chnl_9){
    //Disable voltage divider
    //DDRF |= (1<<4);
    //PORTF |= (1<<4);
    //Need to turn on ADC9 for X-axis and ADC10 for Y axis
    //Test with X axis for now
    //ADMUX |= (1 << 3) | (1 << 0);
    //volatile uint16_t val;
    //ADCSRA |= (1<<ADEN); //Enable the ADC
    //_delay_ms(1); //Wait for the cap to charge

    //ADCSRA |= (1<<ADSC);
    //while (ADCSRA & (1<<ADSC)); //Wait for computation
    //val = ADC; //Read in the value

    //ADCSRA &= ~(1<<ADEN); //Disable the ADC
    //DDRF |= (1<<4);
    
    uint8_t low;
    //Ensure that PIN D6 and D7 are inputs without pullups
    DDRD &= ~(1 << 6);
    DDRD &= ~(1 << 7);
    PORTD &= ~(1 << 6);
    PORTD &= ~(1 << 7);

    //ADCSRA = (1<<ADEN) | ADC_PRESCALER;             // enable ADC
    //ADCSRB = (1<<ADHSM) | (pin_to_mux[mux] & 0x20);             // high speed mode
    ADCSRB = (1<<ADHSM);             // high speed mode
    //ADMUX = (1<<REFS0) | (pin_to_mux[mux] & 0x1F);                    // configure mux input
    ADMUX = 0; //0 it out just to be safe
    ADMUX = (1<<REFS0) |  ((1 << (is_chnl_9 ? 0 : 1)) & 0x1F);                    // configure mux input
    //if(is_chnl_9){
    //ADMUX = (1<<REFS0) |  ((1 << 0) & 0x1F);                    // configure mux input
    //} else {
    //ADMUX = (1<<REFS0) |  ((1 << 1) & 0x1F);                    // configure mux input
    //}
    ADCSRB = (1 << 5); //Get mux channel 5

    ADCSRA = (1<<ADEN) | ADC_PRESCALER ;
    _delay_ms(1); //Wait to charge up
    ADCSRA |= (1<<ADSC); // start the conversion
    while (ADCSRA & (1<<ADSC)) ;                    // wait for result
    low = ADCL;                                     // must read LSB first
    return (ADCH << 8) | low;                       // must read MSB only once!
//}//PORTF &= ~(1<<4);
//    return(val); //May want to do some offset calculations here based on some init datat
}
