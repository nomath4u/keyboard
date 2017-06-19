/*
Copyright 2012 Jun Wako <wakojun@gmail.com>

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

/*
 * scan matrix
 */
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include "print.h"
#include "debug.h"
#include "util.h"
#include "matrix.h"


#ifndef DEBOUNCE
#   define DEBOUNCE	5
#endif
static uint8_t debouncing = DEBOUNCE;

/* matrix state(1:on, 0:off) */
static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_debouncing[MATRIX_ROWS];

static matrix_row_t read_cols(void);
static void init_cols(void);
static void unselect_rows(void);
static void select_row(uint8_t row);


inline
uint8_t matrix_rows(void)
{
    return MATRIX_ROWS;
}

inline
uint8_t matrix_cols(void)
{
    return MATRIX_COLS;
}

void matrix_init(void)
{
    debug_enable = true;
    debug_matrix = true;
    debug_mouse = true;
    // initialize row and col
    unselect_rows();
    init_cols();

    // initialize matrix state: all keys off
    for (uint8_t i=0; i < MATRIX_ROWS; i++) {
        matrix[i] = 0;
        matrix_debouncing[i] = 0;
    }
}

uint8_t matrix_scan(void)
{
    //report_mouse_t mouse_report = {0, 0, 0, 0, 0};
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        select_row(i);
        _delay_us(30);  // without this wait read unstable value.
        matrix_row_t cols = read_cols();
	DDRD |= (1 << 3);
	PORTD &= ~(1 << 3);
        if (matrix_debouncing[i] != cols) {
	  /*  for(int k = 0; k < cols; k++){
	        PORTD |= (1 << 6);
                _delay_us(100000);  // without this wait read unstable value.
	        PORTD &= ~(1 << 6);
                _delay_us(100000);  // without this wait read unstable value.
		
            }*/
            matrix_debouncing[i] = cols;
            if (debouncing) {
                debug("bounce!: "); debug_hex(debouncing); debug("\n");
            }
            debouncing = DEBOUNCE;
        }
        unselect_rows();
    }

    if (debouncing) {
        if (--debouncing) {
            _delay_ms(1);
        } else {
            for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
                matrix[i] = matrix_debouncing[i];
            }
        }
    }

    //matrix_print();
    //mouse_report.buttons = 0;
    //mouse_report.x = 1;
    //mouse_report.y = 1;
    //mouse_report.v = 1;
    //mouse_report.h = 1;
    //send_mouse(mouse_report);
    return 1;
}

bool matrix_is_modified(void)
{
    if (debouncing) return false;
    return true;
}

inline
bool matrix_is_on(uint8_t row, uint8_t col)
{
    return (matrix[row] & ((matrix_row_t)1<<col));
}

inline
matrix_row_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}

void matrix_print(void)
{
    print("\nr/c 0123456789ABCDEF\n");
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        phex(row); print(": ");
        pbin_reverse16(matrix_get_row(row));
        print("\n");
    }
}

uint8_t matrix_key_count(void)
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        count += bitpop16(matrix[i]);
    }
    return count;
}

/* Column pin configuration
 * col: 0
 * pin: B0
 */
static void  init_cols(void)
{
    // Input with pull-up(DDR:0, PORT:1)
    /*DDRB  &= ~(1<<0);
    PORTB |=  (1<<0);
    DDRB  &= ~(1<<1);
    PORTB |=  (1<<1);*/
    DDRB = 0;
    DDRF = 0;
    DDRC = 0;
    for(int i = 0; i < 8; i++){
        PORTB |= (1 << i);
    }
    //PORTF = 0xFF;
    PORTF |= (1 << 0 );
    PORTF |= (1 << 1 );
    PORTF |= (1 << 4 );
    PORTF |= (1 << 5 );
    PORTF |= (1 << 6 );
    PORTF |= (1 << 7 );

    PORTC |= (1 << 6);
    PORTC |= (1 << 7);
}

static matrix_row_t read_cols(void)
{
    int i = 0;
    matrix_row_t stow = 0; 
//    stow |= (PINB&(1<<0) ? 0 : (1<<0));
//    stow |= (PINB&(1<<1) ? 0 : (1<<1));
    for(int i = 0; i < 8; i++){
        stow |= (PINB&(1<<i) ? 0 : (1<<i));
    }
    /*On the farther end from the pinB stuff*/
    //stow |= (PINF&(1<<0) ? 0 : (1 << (8 + 0)));
    //stow |= (PINF&(1<<1) ? 0 : (1 << (8 + 1)));
    //stow |= (PINF&(1<<4) ? 0 : (1 << (8 + 2)));
    //stow |= (PINF&(1<<6) ? 0 : (1 << (8 + 3)));
    stow |= (PINF&(1<<0) ? 0 : (1 << (8 + 0)));
    stow |= (PINF&(1<<1) ? 0 : (1 << (8 + 1)));
    stow |= (PINF&(1<<4) ? 0 : (1 << (8 + 2)));
    stow |= (PINF&(1<<5) ? 0 : (1 << (8 + 3)));
    stow |= (PINF&(1<<6) ? 0 : (1 << (8 + 4)));
    stow |= (PINF&(1<<7) ? 0 : (1 << (8 + 5)));

    stow |= (PINC&(1<<6) ? 0 : (1 << (8 + 6)));
    stow |= (PINC&(1<<7) ? 0 : (1 << (8 + 7)));
    return stow;
    //return (PINB&(1<<0) ? 0 : (1<<0));
}

/* Row pin configuration
 * row: 0
 * pin: B1
 */
static void unselect_rows(void)
{
    // Hi-Z(DDR:0, PORT:0) to unselect
    DDRD  &= ~0b00111111;
    PORTD &= ~0b00111111;
}

static void select_row(uint8_t row)
{
    // Output low(DDR:1, PORT:0) to select
    /*switch (row) {
        case 0:
            DDRB  |= (1<<2);
            PORTB &= ~(1<<2);
            break;
	case 1:
            DDRB  |= (1<<3);
            PORTB &= ~(1<<3);
            break;

    }*/
    DDRD |= (1 << row);
    PORTD &= ~(1 << row);
}
