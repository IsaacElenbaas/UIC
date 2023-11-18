#ifndef MAIN_H
#define MAIN_H
#include <libevdev/libevdev.h>
#include "controller.h"

#define CLOCK_ERROR_HISTORY 100
typedef int input_t;

typedef struct device {
	size_t id;
	int fd;
	struct libevdev* dev;
	cntlr_calibration calibration;
} device;
#endif
