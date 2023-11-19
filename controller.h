#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <atomic>

// DirectInput POV (hat) switch and 8 axes
#define CNTLR_MAX_ANALOG_2D (1+8/2)
// Hat switch is given by evdev as 2 axes so they could be used as such
#define CNTLR_MAX_ANALOG 2+8
#define CNTLR_MAX_BUTTONS 128

enum analog_2d_type {
	analog_2d_primary_thumbstick = 1,
	analog_2d_extra_thumbstick,
	analog_2d_primary_dpad,
	analog_2d_extra_dpad,
	analog_2d_joystick,
	analog_2d_type_max,
	analog_2d_other = 0
};
enum analog_type {
	analog_primary_trigger = 1,
	analog_extra_trigger,
	analog_joystick, // TODO: print as 1D
	analog_joystick_twist,
	analog_other_sticky,
	analog_type_max,
	analog_other = 0
};
enum button_type {
	button_start = 1,
	button_select,
	button_home,
	button_bumper,
	button_trigger,
	button_dpad_up,
	button_dpad_down,
	button_dpad_left,
	button_dpad_right,
	button_face_up,
	button_face_down,
	button_face_left,
	button_face_right,
	button_type_max,
	button_other = 0
};

typedef struct analog_2d_calibrations {
	bool present = false;
	analog_2d_type type;
	int side;
	int x_code, y_code;
	int center_x, center_y;
	int deadzone;
	double xl_scale, xr_scale, yd_scale, yu_scale;
	bool circular;
	double q_scales[4];
	double q_pows[4];
} analog_2d;

typedef struct analog_calibrations {
	bool present = false;
	analog_type type;
	int side;
	int code;
	int center;
	double plus_scale, minus_scale;
} analog;

typedef struct button_calibrations {
	bool present = false;
	button_type type;
	int side;
	int code;
} button;

typedef struct cntlr_calibration {
	std::atomic<bool> done = false;
	analog_2d_calibrations analog_2ds[CNTLR_MAX_ANALOG_2D];
	analog_calibrations analogs[CNTLR_MAX_ANALOG];
	button_calibrations buttons[CNTLR_MAX_BUTTONS];
} cntlr_calibration;

#include "main.h"
void calibrate(device* dev);
void calibration_event(device* dev, int type, int code, int value);
#endif
