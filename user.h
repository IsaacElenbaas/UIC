#ifndef USER_H
#define USER_H
#include <array>
#include <cstdint>
#include <forward_list>
#include <vector>

// Hat switch is given by evdev as 2 axes so they could be used as such
#define CNTLR_MAX_ANALOGS (2+8)
// DirectInput POV (hat) switch and 8 axes
#define CNTLR_MAX_ANALOG_2DS (CNTLR_MAX_ANALOGS/2)
#define CNTLR_MAX_BUTTONS 128

enum {
	CNTLR_DPAD = 0,
	CNTLR_LTHUMB,
	CNTLR_RTHUMB,
	CNTLR_LTRIGGER,
	CNTLR_RTRIGGER,
	CNTLR_START,
	CNTLR_SELECT,
	CNTLR_HOME,
	CNTLR_LBUMPER,
	CNTLR_RBUMPER,
	CNTLR_LTRIGGER_BUTTON,
	CNTLR_RTRIGGER_BUTTON,
	CNTLR_LTHUMB_BUTTON,
	CNTLR_RTHUMB_BUTTON,
	CNTLR_Y,
	CNTLR_A,
	CNTLR_X,
	CNTLR_B,
	CNTLR_INPUT_MAX
};

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
	analog_joystick,
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
	button_thumb_press,
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

typedef struct input {
	bool physical = false;
	// one can deduce the calibration type from these
	uint_least16_t key = 0;
	bool digital;
	int axes;
	void* calibration;
	int side;
	bool circular;
	double values[3] = {0, 0, 0};
	// to be used as a key for storing required persistent data
	// any unchanging parameters should also be relied on
	uintptr_t hash;
} input;

typedef struct analog_2d_calibrations {
	bool present = false;
	input* out;
	analog_2d_type type;
	int side;
	int x_code, y_code;
	int center_x, center_y;
	int deadzone;
	double xl_scale, xr_scale, yd_scale, yu_scale;
	bool circular;
	double q_scales[4];
	double q_pows[4];
	// not for use
	bool updated;
	int raw_x, raw_y;
} analog_2d;

typedef struct analog_calibrations {
	bool present = false;
	input* out;
	analog_type type;
	int side;
	int code;
	int center;
	double plus_scale, minus_scale;
} analog;

typedef struct button_calibrations {
	bool present = false;
	input* out;
	button_type type;
	int side;
	int code;
} button;

// milliseconds since last frame
extern double elapsed;

class Controller {
	int fd = -1;
	int available_buttons = CNTLR_MAX_BUTTONS-4-13;
	int available_axes = CNTLR_MAX_ANALOGS-3*2-2;
	int extra_buttons = 0;
	int extra_analogs = 0;
	int extra_analog_2ds = 0;
	std::vector<input> inputs_vec, last_inputs_vec;
	std::vector<std::array<int, 2>> codes;
public:
	input* inputs = NULL;
	// see https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
	int add_button(int code);
	int add_analog(int code);
	int add_analog_2d(int x_code, int y_code);
	void init();
	void reset();
	void apply();
};

extern void user_print_profiles();
extern void user_switch_profile(int profile, std::forward_list<std::forward_list<input>>* inputs);
extern void user_process_frame();
extern void user_stop();
#endif
