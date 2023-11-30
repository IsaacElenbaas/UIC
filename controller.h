#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <atomic>
#include <forward_list>
#include "user.h"

struct cntlr_calibration {
	std::atomic<bool> done = false;
	analog_2d_calibrations analog_2ds[CNTLR_MAX_ANALOG_2DS];
	analog_calibrations analogs[CNTLR_MAX_ANALOGS];
	button_calibrations buttons[CNTLR_MAX_BUTTONS];
};

#include "main.h"
void calibrate(device* dev);
void calibration_event(device* dev, int type, int code, int value);
#endif
