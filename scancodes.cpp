#include <cstring>
#include <libevdev/libevdev.h>

// based on https://github.com/torvalds/linux/blob/master/drivers/tty/vt/defkeymap.map
unsigned int scancodes[KEY_MAX];
void init_scancodes() {
	memset(scancodes, 0, sizeof(scancodes));
	scancodes[KEY_ESC] = 1; // Escape
	scancodes[KEY_1] = 2; // one, exclam
	scancodes[KEY_2] = 3; // two, at
	scancodes[KEY_3] = 4; // three, numbersign
	scancodes[KEY_4] = 5; // four, dollar
	scancodes[KEY_5] = 6; // five, percent
	scancodes[KEY_6] = 7; // six, asciicircum
	scancodes[KEY_7] = 8; // seven, ampersand
	scancodes[KEY_8] = 9; // eight, asterisk
	scancodes[KEY_9] = 10; // nine, parenleft
	scancodes[KEY_0] = 11; // zero, parenright
	scancodes[KEY_MINUS] = 12; // minus, underscore
	scancodes[KEY_EQUAL] = 13; // equal, plus
	scancodes[KEY_BACKSPACE] = 14; // Delete
	scancodes[KEY_TAB] = 15; // Tab
	scancodes[KEY_Q] = 16; // q
	scancodes[KEY_W] = 17; // w
	scancodes[KEY_E] = 18; // e
	scancodes[KEY_R] = 19; // r
	scancodes[KEY_T] = 20; // t
	scancodes[KEY_Y] = 21; // y
	scancodes[KEY_U] = 22; // u
	scancodes[KEY_I] = 23; // i
	scancodes[KEY_O] = 24; // o
	scancodes[KEY_P] = 25; // p
	scancodes[KEY_LEFTBRACE] = 26; // bracketleft, braceleft
	scancodes[KEY_RIGHTBRACE] = 27; // bracketright, braceright
	scancodes[KEY_ENTER] = 28; // Return
	scancodes[KEY_LEFTCTRL] = 29; // Control
	scancodes[KEY_A] = 30; // a
	scancodes[KEY_S] = 31; // s
	scancodes[KEY_D] = 32; // d
	scancodes[KEY_F] = 33; // f
	scancodes[KEY_G] = 34; // g
	scancodes[KEY_H] = 35; // h
	scancodes[KEY_J] = 36; // j
	scancodes[KEY_K] = 37; // k
	scancodes[KEY_L] = 38; // l
	scancodes[KEY_SEMICOLON] = 39; // semicolon, colon
	scancodes[KEY_APOSTROPHE] = 40; // apostrophe, quotedbl
	scancodes[KEY_GRAVE] = 41; // grave, asciitilde
	scancodes[KEY_LEFTSHIFT] = 42; // Shift
	scancodes[KEY_BACKSLASH] = 43; // backslash, bar
	scancodes[KEY_Z] = 44; // z
	scancodes[KEY_X] = 45; // x
	scancodes[KEY_C] = 46; // c
	scancodes[KEY_V] = 47; // v
	scancodes[KEY_B] = 48; // b
	scancodes[KEY_N] = 49; // n
	scancodes[KEY_M] = 50; // m
	scancodes[KEY_COMMA] = 51; // comma, less
	scancodes[KEY_DOT] = 52; // period, greater
	scancodes[KEY_SLASH] = 53; // slash, question
	scancodes[KEY_RIGHTSHIFT] = 54; // Shift
	scancodes[KEY_KPASTERISK] = 55; // KP_Multiply
	scancodes[KEY_LEFTALT] = 56; // Alt
	scancodes[KEY_SPACE] = 57; // space
	scancodes[KEY_CAPSLOCK] = 58; // Caps_Lock
	scancodes[KEY_F1] = 59; // F1
	scancodes[KEY_F2] = 60; // F2
	scancodes[KEY_F3] = 61; // F3
	scancodes[KEY_F4] = 62; // F4
	scancodes[KEY_F5] = 63; // F5
	scancodes[KEY_F6] = 64; // F6
	scancodes[KEY_F7] = 65; // F7
	scancodes[KEY_F8] = 66; // F8
	scancodes[KEY_F9] = 67; // F9
	scancodes[KEY_F10] = 68; // F10
	scancodes[KEY_NUMLOCK] = 69; // Num_Lock
	scancodes[KEY_SCROLLLOCK] = 70; // Scroll_Lock
	scancodes[KEY_KP7] = 71; // KP_7
	scancodes[KEY_KP8] = 72; // KP_8
	scancodes[KEY_KP9] = 73; // KP_9
	scancodes[KEY_KPMINUS] = 74; // KP_Subtract
	scancodes[KEY_KP4] = 75; // KP_4
	scancodes[KEY_KP5] = 76; // KP_5
	scancodes[KEY_KP6] = 77; // KP_6
	scancodes[KEY_KPPLUS] = 78; // KP_Add
	scancodes[KEY_KP1] = 79; // KP_1
	scancodes[KEY_KP2] = 80; // KP_2
	scancodes[KEY_KP3] = 81; // KP_3
	scancodes[KEY_KP0] = 82; // KP_0
	scancodes[KEY_KPDOT] = 83; // KP_Period
	//scancodes[KEY_???] = 86; // less, greater, bar
	scancodes[KEY_F11] = 87; // F11
	scancodes[KEY_F12] = 88; // F12
	scancodes[KEY_KPENTER] = 96; // KP_Enter
	scancodes[KEY_RIGHTCTRL] = 97; // Control
	scancodes[KEY_KPSLASH] = 98; // KP_Divide
	scancodes[KEY_RIGHTALT] = 100; // AltGr
	scancodes[KEY_BREAK] = 101; // Break
	scancodes[KEY_FIND] = 102; // Find
	scancodes[KEY_UP] = 103; // Up
	scancodes[KEY_PAGEUP] = 104; // Prior
	scancodes[KEY_LEFT] = 105; // Left
	scancodes[KEY_RIGHT] = 106; // Right
	scancodes[KEY_SELECT] = 107; // Select
	scancodes[KEY_DOWN] = 108; // Down
	scancodes[KEY_PAGEDOWN] = 109; // Next
	scancodes[KEY_INSERT] = 110; // Insert
	scancodes[KEY_DELETE] = 111; // Remove
	scancodes[KEY_F13] = 113; // F13
	scancodes[KEY_F14] = 114; // F14
	scancodes[KEY_HELP] = 115; // Help
	//scancodes[KEY_???] = 116; // Do
	scancodes[KEY_F17] = 117; // F17
	scancodes[KEY_KPPLUSMINUS] = 118; // KP_MinPlus
	scancodes[KEY_PAUSE] = 119; // Pause
}
