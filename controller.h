#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <atomic>

// DirectInput POV (hat) switch and 8 axes
#define CNTLR_MAX_ANALOG_2D (1+8/2)
// Hat switch is given by evdev as 2 axes so they could be used as such
#define CNTLR_MAX_ANALOG 2+8
#define CNTLR_MAX_BUTTONS 128

typedef struct analog_2d {
	bool present = false;
	int x_code, y_code;
	int center_x, center_y;
	int deadzone;
	double xl_scale, xr_scale, yd_scale, yu_scale;
	bool circular;
	double q_scales[4];
	double q_pows[4];
} analog_2d;

typedef struct analog {
	bool present = false;
	int code;
	int base;
	double scale;
} analog;

typedef struct cntlr_calibration {
	std::atomic<bool> done = false;
	analog_2d analog_2ds[CNTLR_MAX_ANALOG_2D];
	analog analogs[CNTLR_MAX_ANALOG];
} cntlr_calibration;

#include "main.h"
void calibrate(device* dev);
void calibration_event(device* dev, int type, int code, int value);
#endif
