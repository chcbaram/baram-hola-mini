// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H




const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
    * ,-----------------------------------------------------------------------------------.
    * | Tab  |   Q  |   W  |   E  |   R  |   T  |   Y  |   U  |   I  |   O  |   P  | Bksp | 
    * |------+------+------+------+------+------+------+------+------+------+------+------|
    * | Esc  |   A  |   S  |   D  |   F  |   G  |   H  |   J  |   K  |   L  |   ;  |  '   |
    * |------+------+------+------+------+------+------+------+------+------+------+------|
    * | Shift|   Z  |   X  |   C  |   V  |   B  |   N  |   M  |   ,  |   .  |   /  |Enter |
    * |------+------+------+------+------+------+------+------+------+------+------+------|
    * |Adjust| Ctrl | Alt  | GUI  |Lower |   Space     |Raise | Left | Down |  Up  |Right |
    * `-----------------------------------------------------------------------------------'
    */
    [0] = {
        {KC_ESC,        KC_Q,    KC_W,    KC_E,    KC_R,          KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC  },
        {MO(1),         KC_A,    KC_S,    KC_D,    KC_F,          KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    _______, KC_ENT   },
        {LM(2,MOD_LSFT),KC_Z,    KC_X,    KC_C,    KC_V,          KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  _______, KC_TRNS  },
        {KC_LCTL,       KC_LGUI, KC_LALT, _______, KC_SPC,        _______, _______, KC_SPC,  KC_TRNS, KC_TRNS, _______, KC_TRNS  }
    }         
};
