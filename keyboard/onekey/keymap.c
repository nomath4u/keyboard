/*
Copyright 2012,2013 Jun Wako <wakojun@gmail.com>

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
#include <stdbool.h>
#include <avr/pgmspace.h>
#include "keycode.h"
#include "action.h"
#include "action_code.h"
#include "action_macro.h"
#include "action_util.h"
#include "report.h"
#include "host.h"
#include "print.h"
#include "debug.h"
#include "keymap.h"

// Convert physical keyboard layout to matrix array.
// This is a macro to define keymap easily in keyboard layout form.

#define KEYMAP( \
          K140, K130, K120, K110, K100, K90, K80, K70, K60, K50, K40, K30, K20, K10, K00,\
    K151, K141, K131, K121, K111, K101, K91, K81, K71, K61, K51, K41, K31, K21, K11, K01,\
    K152, K142, K132, K122, K112, K102, K92, K82, K72, K62, K52, K42, K32, K22, K12,\
          K143, K133, K123, K113, K103, K93, K83, K73, K63, K53, K43, K33, K23, \
          K144, K134, K124, K114, K104, K94, K84, K74, K64, K54, K44, K34, K24, \
          K145, K135, K125, K115, K105, K95, K85, K75, K65, K55, K45, K35, K25, K16 \
) { \
    { KC_##K00, KC_##K10, KC_##K20, KC_##K30, KC_##K40, KC_##K50, KC_##K60, KC_##K70, KC_##K80, KC_##K90, KC_##K100, KC_##K110, KC_##K120, KC_##K130, KC_##K140, KC_NO },\
    { KC_##K01, KC_##K11, KC_##K21, KC_##K31, KC_##K41, KC_##K51, KC_##K61, KC_##K71, KC_##K81, KC_##K91, KC_##K101, KC_##K111, KC_##K121, KC_##K131, KC_##K141, KC_##K151 },\
    { KC_NO,    KC_##K12, KC_##K22, KC_##K32, KC_##K42, KC_##K52, KC_##K62, KC_##K72, KC_##K82, KC_##K92, KC_##K102, KC_##K112, KC_##K122, KC_##K132, KC_##K142, KC_##K152 },\
    { KC_NO,    KC_NO,    KC_##K23, KC_##K33, KC_##K43, KC_##K53, KC_##K63, KC_##K73, KC_##K83, KC_##K93, KC_##K103, KC_##K113, KC_##K123, KC_##K133, KC_##K143, KC_NO},\
    { KC_NO,    KC_NO,    KC_##K24, KC_##K34, KC_##K44, KC_##K54, KC_##K64, KC_##K74, KC_##K84, KC_##K94, KC_##K104, KC_##K114, KC_##K124, KC_##K134, KC_##K144, KC_NO},\
    { KC_NO,    KC_##K16,    KC_##K25, KC_##K35, KC_##K45, KC_##K55, KC_##K65, KC_##K75, KC_##K85, KC_##K95, KC_##K105, KC_##K115, KC_##K125, KC_##K135, KC_##K145, KC_NO}\
}

static const uint8_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
     KEYMAP( CAPS,   F1,   F2,   F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, INS, DEL, \
           UP,   GRV,  1,   2,   3, 4, 5, 6, 7, 8, 9, 0, MINS, EQL,DEL, BSPC, \
           DOWN, TAB,   Q,   W,   E, R, T,Y,U,I,O,P, LBRC, RBRC, ENT, \
                 ESC,   A,   S,   D, F, G, H, J, K, L, SCLN, QUOT,RSFT,\
	         LSFT, Z, X, C, V, B, N, M, COMM, DOT, SLSH, RSFT,RSFT, \
	         LCTL, FN0, LALT, SPC, SPC, SPC, SPC, SPC, RALT, RCTL, UP, LEFT, DOWN, RGHT
	   ),
};
/*
 * Fn action definition
 */
static const uint16_t PROGMEM fn_actions[] = {
};



#define KEYMAPS_SIZE    (sizeof(keymaps) / sizeof(keymaps[0]))
#define FN_ACTIONS_SIZE (sizeof(fn_actions) / sizeof(fn_actions[0]))

/* translates key to keycode */
uint8_t keymap_key_to_keycode(uint8_t layer, keypos_t key)
{
    if (layer < KEYMAPS_SIZE) {
        return pgm_read_byte(&keymaps[(layer)][(key.row)][(key.col)]);
    } else {
        // fall back to layer 0
        return pgm_read_byte(&keymaps[0][(key.row)][(key.col)]);
    }
}

/* translates Fn keycode to action */
action_t keymap_fn_to_action(uint8_t keycode)
{
    action_t action;
    if (FN_INDEX(keycode) < FN_ACTIONS_SIZE) {
        action.code = pgm_read_word(&fn_actions[FN_INDEX(keycode)]);
    } else {
        action.code = ACTION_NO;
    }
    return action;
}
