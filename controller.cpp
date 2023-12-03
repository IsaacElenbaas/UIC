#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <forward_list>
#include <iostream>
#include <libevdev/libevdev.h>
#include <limits>
#include <linux/uinput.h>
#include <mutex>
#include <semaphore>
#include <unistd.h>
#include "controller.h"

#define C_ANGLES_STEPS 100

extern std::forward_list<std::forward_list<input>> inputs;

/*{{{ calibration*/
	/*{{{ void calibration_save/load(device* dev)*/
void calibration_save(device* dev) {
	int max_analog_2ds, max_analogs, max_buttons;

		/*{{{ get max values*/
	for(max_analog_2ds = 0; max_analog_2ds < CNTLR_MAX_ANALOG_2DS; max_analog_2ds++) {
		if(!dev->calibration.analog_2ds[max_analog_2ds].present) break;
	}
	for(max_analogs = 0; max_analogs < CNTLR_MAX_ANALOGS; max_analogs++) {
		if(!dev->calibration.analogs[max_analogs].present) break;
	}
	for(max_buttons = 0; max_buttons < CNTLR_MAX_BUTTONS; max_buttons++) {
		if(!dev->calibration.buttons[max_buttons].present) break;
	}
		/*}}}*/

		/*{{{ write*/
	int fd = open(".dev-calibration", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	write(fd, &max_analog_2ds, sizeof(max_analog_2ds));
	write(fd, &max_analogs,    sizeof(max_analogs   ));
	write(fd, &max_buttons,    sizeof(max_buttons   ));
	for(int i = 0; i < max_analog_2ds; i++) {
		write(fd, &dev->calibration.analog_2ds[i].type,        sizeof(dev->calibration.analog_2ds[i].type    ));
		write(fd, &dev->calibration.analog_2ds[i].side,        sizeof(dev->calibration.analog_2ds[i].side    ));
		write(fd, &dev->calibration.analog_2ds[i].x_code,      sizeof(dev->calibration.analog_2ds[i].x_code  ));
		write(fd, &dev->calibration.analog_2ds[i].y_code,      sizeof(dev->calibration.analog_2ds[i].y_code  ));
		write(fd, &dev->calibration.analog_2ds[i].center_x,    sizeof(dev->calibration.analog_2ds[i].center_x));
		write(fd, &dev->calibration.analog_2ds[i].center_y,    sizeof(dev->calibration.analog_2ds[i].center_y));
		write(fd, &dev->calibration.analog_2ds[i].deadzone,    sizeof(dev->calibration.analog_2ds[i].deadzone));
		write(fd, &dev->calibration.analog_2ds[i].xl_scale,    sizeof(dev->calibration.analog_2ds[i].xl_scale));
		write(fd, &dev->calibration.analog_2ds[i].xr_scale,    sizeof(dev->calibration.analog_2ds[i].xr_scale));
		write(fd, &dev->calibration.analog_2ds[i].yd_scale,    sizeof(dev->calibration.analog_2ds[i].yd_scale));
		write(fd, &dev->calibration.analog_2ds[i].yu_scale,    sizeof(dev->calibration.analog_2ds[i].yu_scale));
		write(fd, &dev->calibration.analog_2ds[i].circular,    sizeof(dev->calibration.analog_2ds[i].circular));
		write(fd, &dev->calibration.analog_2ds[i].q_scales[0], sizeof(dev->calibration.analog_2ds[i].q_scales));
		write(fd, &dev->calibration.analog_2ds[i].q_pows[0],   sizeof(dev->calibration.analog_2ds[i].q_pows  ));
	}
	for(int i = 0; i < max_analogs; i++) {
		write(fd, &dev->calibration.analogs[i].type,        sizeof(dev->calibration.analogs[i].type       ));
		write(fd, &dev->calibration.analogs[i].side,        sizeof(dev->calibration.analogs[i].side       ));
		write(fd, &dev->calibration.analogs[i].code,        sizeof(dev->calibration.analogs[i].code       ));
		write(fd, &dev->calibration.analogs[i].center,      sizeof(dev->calibration.analogs[i].center     ));
		write(fd, &dev->calibration.analogs[i].plus_scale,  sizeof(dev->calibration.analogs[i].plus_scale ));
		write(fd, &dev->calibration.analogs[i].minus_scale, sizeof(dev->calibration.analogs[i].minus_scale));
	}
	for(int i = 0; i < max_buttons; i++) {
		write(fd, &dev->calibration.buttons[i].type,        sizeof(dev->calibration.buttons[i].type       ));
		write(fd, &dev->calibration.buttons[i].side,        sizeof(dev->calibration.buttons[i].side       ));
		write(fd, &dev->calibration.buttons[i].code,        sizeof(dev->calibration.buttons[i].code       ));
	}
	close(fd);
		/*}}}*/

}

void calibration_load(device* dev) {
	int max_analog_2ds, max_analogs, max_buttons;

		/*{{{ read*/
	int fd = open(".dev-calibration", O_RDONLY);
	if(fd == -1) return;
	read(fd, &max_analog_2ds, sizeof(max_analog_2ds));
	read(fd, &max_analogs,    sizeof(max_analogs   ));
	read(fd, &max_buttons,    sizeof(max_buttons   ));
	for(int i = 0; i < max_analog_2ds; i++) {
		dev->calibration.analog_2ds[i].present = true;
		read(fd, &dev->calibration.analog_2ds[i].type,        sizeof(dev->calibration.analog_2ds[i].type    ));
		read(fd, &dev->calibration.analog_2ds[i].side,        sizeof(dev->calibration.analog_2ds[i].side    ));
		read(fd, &dev->calibration.analog_2ds[i].x_code,      sizeof(dev->calibration.analog_2ds[i].x_code  ));
		read(fd, &dev->calibration.analog_2ds[i].y_code,      sizeof(dev->calibration.analog_2ds[i].y_code  ));
		read(fd, &dev->calibration.analog_2ds[i].center_x,    sizeof(dev->calibration.analog_2ds[i].center_x));
		read(fd, &dev->calibration.analog_2ds[i].center_y,    sizeof(dev->calibration.analog_2ds[i].center_y));
		read(fd, &dev->calibration.analog_2ds[i].deadzone,    sizeof(dev->calibration.analog_2ds[i].deadzone));
		read(fd, &dev->calibration.analog_2ds[i].xl_scale,    sizeof(dev->calibration.analog_2ds[i].xl_scale));
		read(fd, &dev->calibration.analog_2ds[i].xr_scale,    sizeof(dev->calibration.analog_2ds[i].xr_scale));
		read(fd, &dev->calibration.analog_2ds[i].yd_scale,    sizeof(dev->calibration.analog_2ds[i].yd_scale));
		read(fd, &dev->calibration.analog_2ds[i].yu_scale,    sizeof(dev->calibration.analog_2ds[i].yu_scale));
		read(fd, &dev->calibration.analog_2ds[i].circular,    sizeof(dev->calibration.analog_2ds[i].circular));
		read(fd, &dev->calibration.analog_2ds[i].q_scales[0], sizeof(dev->calibration.analog_2ds[i].q_scales));
		read(fd, &dev->calibration.analog_2ds[i].q_pows[0],   sizeof(dev->calibration.analog_2ds[i].q_pows  ));
	}
	for(int i = 0; i < max_analogs; i++) {
		dev->calibration.analogs[i].present = true;
		read(fd, &dev->calibration.analogs[i].type,        sizeof(dev->calibration.analogs[i].type       ));
		read(fd, &dev->calibration.analogs[i].side,        sizeof(dev->calibration.analogs[i].side       ));
		read(fd, &dev->calibration.analogs[i].code,        sizeof(dev->calibration.analogs[i].code       ));
		read(fd, &dev->calibration.analogs[i].center,      sizeof(dev->calibration.analogs[i].center     ));
		read(fd, &dev->calibration.analogs[i].plus_scale,  sizeof(dev->calibration.analogs[i].plus_scale ));
		read(fd, &dev->calibration.analogs[i].minus_scale, sizeof(dev->calibration.analogs[i].minus_scale));
	}
	for(int i = 0; i < max_buttons; i++) {
		dev->calibration.buttons[i].present = true;
		read(fd, &dev->calibration.buttons[i].type,        sizeof(dev->calibration.buttons[i].type       ));
		read(fd, &dev->calibration.buttons[i].side,        sizeof(dev->calibration.buttons[i].side       ));
		read(fd, &dev->calibration.buttons[i].code,        sizeof(dev->calibration.buttons[i].code       ));
	}
	close(fd);
		/*}}}*/

	dev->calibration.done = true;
}
	/*}}}*/

struct stage { int a, b; };
static std::atomic<stage> c_stage = (stage){0, 0};

	/*{{{ void assign_type(int* input_type, int* input_side)*/
void assign_type(int* input_type, int* input_side) {
	int stage = c_stage.load().a;

		/*{{{ type text*/
	char analog_2d_types[] = "\
1: Primary thumbstick\n\
2: Extra thumbstick\n\
3: Primary D-pad\n\
4: Extra D-pad\n\
5: Joystick\n\
6: Other / Extra\n\
";
	char analog_types[] = "\
1: Primary trigger\n\
2: Extra trigger\n\
3: Joystick (No sideways)\n\
4: Joystick Twist\n\
5: Other / Extra that does not re-center\n\
6: Other / Extra\n\
";
	char button_types[] = "\
 1: Start\n\
 2: Select\n\
 3: Home\n\
 4: Bumper\n\
 5: Trigger Button\n\
 6: Thumb Button\n\
 7: D-pad Up\n\
 8: D-pad Down\n\
 9: D-pad Left\n\
10: D-pad Right\n\
11: Face Up\n\
12: Face Down\n\
13: Face Left\n\
14: Face Right\n\
15: Other / Extra\n\
";
		/*}}}*/

	char* stage_types[] = {analog_2d_types, analog_types, button_types};
	std::cout << stage_types[stage-1];
	*input_type = -1;
	while(*input_type == -1) {
		std::cout << "Input type: ";
		std::string type;
		int c;
		while((c = getchar()) != EOF) {
			if(c == '\n') {
				if(type.size() > 0) break;
				std::cout << "\033[A\r\033[KInput type: ";
				continue;
			}
			else if(c == '\177') {
				std::cout << "\b\b  \b\b";
				if(!type.empty()) {
					std::cout << "\b \b";
					type.erase(type.size()-1);
				}
				continue;
			}
			else if(c < '0' || c > '9') {
				if(c >= ' ') std::cout << "\b \b";
				else std::cout << "\r\033[KInput type: " << type;
				continue;
			}
			type += c;
		}
		if(c == EOF) return;
		sscanf(type.c_str(), "%d", input_type);
		if(
			*input_type > ((stage == 1)
				? (int)analog_2d_type_max : (stage == 2)
				? (int)analog_type_max
				: (int)button_type_max
			)
		) {
			std::cout << "\033[A\r\033[KInput type: ";
			*input_type = -1;
		}
	}
	std::cout << "Press L if this input is on the left side of the controller or R if on the right" << std::endl;
	int c;
	do { c = getchar(); } while(c != 'l' && c != 'L' && c != 'r' && c != 'R');
	*input_side = (c == 'r' || c == 'R') ? 1 : -1;
}
	/*}}}*/

std::mutex c_mut;
static std::binary_semaphore calibrate_continue_sem{0};
static int c_i;
static std::atomic<double> c_axis[ABS_MAX];
static bool c_updated;
static double c_x, c_y;
static int c_min_x, c_max_x, c_min_y, c_max_y;
static int c_angle_spins;
static double c_angles[C_ANGLES_STEPS];
static std::atomic<int> c_last_button;

void calibrate(device* dev) {
	calibration_load(dev);
	cntlr_calibration* cal = &dev->calibration;
	if(!dev->calibration.done) {
	// TODO: needs to handle ctrl+c and input disconnecting
	char c;
	int available_axes = CNTLR_MAX_ANALOGS;

	// TODO: gyro

	/*{{{ analog 2Ds*/
	for(c_i = 0; true; c_i++) {
		if(available_axes < 2) {
			std::cout << "Not enough analog axes available, continuing" << std::endl;
			break;
		}
		c_stage = (stage){1, 0};
		std::cout << "Press C to add a 2D input or space to continue" << std::endl;
		do { c = getchar(); std::cout << "\r\033[K"; } while(c != 'c' && c != 'C' && c != ' ');
		if(c == ' ') break;

		/*{{{ identify*/
		c_mut.lock();
		for(size_t i = 0; i < sizeof(c_axis)/sizeof(c_axis[0]); i++) {
			c_axis[i] = 0;
		}
		c_mut.unlock();
		std::cout << "Spin to identify it" << std::endl;
		c_stage = (stage){c_stage.load().a, c_stage.load().b+1};
		calibrate_continue_sem.acquire();
		c_mut.lock(); c_mut.unlock(); // need to ensure second axis was set
		cal->analog_2ds[c_i].x_code = std::max_element(std::begin(c_axis), std::end(c_axis))-c_axis;
		c_axis[cal->analog_2ds[c_i].x_code] = 0;
		cal->analog_2ds[c_i].y_code = std::max_element(std::begin(c_axis), std::end(c_axis))-c_axis;
		/*}}}*/

		/*{{{ get top*/
		c_mut.lock();
		c_axis[cal->analog_2ds[c_i].x_code] = 0;
		c_axis[cal->analog_2ds[c_i].y_code] = 0;
		c_stage = (stage){c_stage.load().a, c_stage.load().b+1};
		// clearing in case calibration_event did it twice
		calibrate_continue_sem.try_acquire();
		c_mut.unlock();
		std::cout << "Move to the top of the input and press any key" << std::endl;
		getchar(); std::cout << "\r\033[K";
		if(fabs(c_axis[cal->analog_2ds[c_i].x_code]) > fabs(c_axis[cal->analog_2ds[c_i].y_code]))
			std::swap(cal->analog_2ds[c_i].x_code, cal->analog_2ds[c_i].y_code);
		// 10 because we'll shrink it later
		cal->analog_2ds[c_i].yu_scale = (c_axis[cal->analog_2ds[c_i].y_code] > 0) ? 10 : -10;
		cal->analog_2ds[c_i].yd_scale = cal->analog_2ds[c_i].yu_scale;
		/*}}}*/

		/*{{{ get right*/
		c_mut.lock(); c_axis[cal->analog_2ds[c_i].x_code] = 0; c_mut.unlock();
		std::cout << "Move to the right of the input and press any key" << std::endl;
		getchar(); std::cout << "\r\033[K";
		// 10 because we'll shrink it later
		cal->analog_2ds[c_i].xl_scale = (c_axis[cal->analog_2ds[c_i].x_code] > 0) ? 10 : -10;
		cal->analog_2ds[c_i].xr_scale = cal->analog_2ds[c_i].xl_scale;
		/*}}}*/

		/*{{{ get center*/
		c_stage = (stage){c_stage.load().a, c_stage.load().b+1};
		std::cout << "Center the input and press any key" << std::endl;
		getchar(); std::cout << "\r\033[K";
		/*}}}*/

		/*{{{ get extents, calculate adjustments*/
		c_updated = false;
		c_x = c_y = 0;
		c_min_x = c_max_x = c_min_y = c_max_y = 0;
		c_angle_spins = 0;
		memset(c_angles, 0, sizeof(c_angles)); c_angle_spins = 0;
		std::cout << "Spin at a moderate speed 10-15 revolutions at the extents of the input" << std::endl;
		c_stage = (stage){c_stage.load().a, c_stage.load().b+1};
		calibrate_continue_sem.acquire();
		std::cout << "Should this input be circular (C) or square (S)?" << std::endl;
		do { c = getchar(); std::cout << "\r\033[K"; } while(c != 'c' && c != 'C' && c != 's' && c != 'S');
		bool circular = c == 'c' || c == 'C';

			/*{{{ calculate quadrant scales and spread powers*/
		for(int q = 0; q < 4; q++) {
			cal->analog_2ds[c_i].q_scales[q] = c_angles[(int)((q/4.0+1/8.0)*C_ANGLES_STEPS)];
			cal->analog_2ds[c_i].q_scales[q] = ((circular) ? 1 : sqrt(2))/cal->analog_2ds[c_i].q_scales[q];
			double max_error = std::numeric_limits<double>::max();
			for(int p = 0; p <= 300; p++) {
				double error = 0;
				for(int i = (int)((q/4.0)*C_ANGLES_STEPS); i < (int)(((q+1)/4.0)*C_ANGLES_STEPS); i++) {
					double angle = ((i+0.5)/C_ANGLES_STEPS)*(2*M_PI);
					double radius = (circular) ? 1 : std::min(fabs(1/cos(angle)), fabs(1/sin(angle)));
					double q_progress = angle;
					while(q_progress > M_PI/2) { q_progress -= M_PI/2; }
					q_progress /= M_PI/2;
					double scale = cal->analog_2ds[c_i].q_scales[q];
					scale = 1+std::min(1.0, cos(M_PI*pow(fabs(2*q_progress-1), p/100.0))+1)*(scale-1);
					error += fabs(radius-scale*c_angles[i]);
				}
				if(error > max_error) break;
				else {
					cal->analog_2ds[c_i].q_pows[q] = p/100.0;
					max_error = error;
				}
			}
		}
			/*}}}*/
		/*}}}*/

		c_mut.lock();
		c_stage = (stage){c_stage.load().a, c_stage.load().b+1};
		// clearing in case calibration_event did it twice
		calibrate_continue_sem.try_acquire();
		std::cout << "Check the input below and press D to discard or space to continue" << std::endl;
		c_mut.unlock();
		do { c = getchar(); std::cout << "\r\033[K"; } while(c != 'd' && c != 'D' && c != ' ');
		if(c == 'd' || c == 'D') c_i--;
		else {
			int min = libevdev_get_abs_minimum(dev->dev, cal->analog_2ds[c_i].x_code);
			int max = libevdev_get_abs_maximum(dev->dev, cal->analog_2ds[c_i].x_code);
			cal->analog_2ds[c_i].xl_scale /= cal->analog_2ds[c_i].center_x-min;
			cal->analog_2ds[c_i].xr_scale /= max-cal->analog_2ds[c_i].center_x;
			min = libevdev_get_abs_minimum(dev->dev, cal->analog_2ds[c_i].y_code);
			max = libevdev_get_abs_maximum(dev->dev, cal->analog_2ds[c_i].y_code);
			cal->analog_2ds[c_i].yd_scale /= cal->analog_2ds[c_i].center_y-min;
			cal->analog_2ds[c_i].yu_scale /= max-cal->analog_2ds[c_i].center_y;
			cal->analog_2ds[c_i].present = true;
			c_stage = (stage){c_stage.load().a, c_stage.load().b+1};
			assign_type((int*)&cal->analog_2ds[c_i].type, &cal->analog_2ds[c_i].side);
			available_axes -= 2;
		}
		std::cout << "\r\033[K";
	}
	/*}}}*/

	/*{{{ analogs*/
	for(c_i = 0; true; c_i++) {
		if(available_axes == 0) {
			std::cout << "No more analog axes available, continuing" << std::endl;
			break;
		}
		c_stage = (stage){2, 0};
		std::cout << "Press C to add an analog input or space to continue" << std::endl;
		do { c = getchar(); std::cout << "\r\033[K"; } while(c != 'c' && c != 'C' && c != ' ');
		if(c == ' ') break;

		/*{{{ identify*/
		c_mut.lock();
		for(size_t i = 0; i < sizeof(c_axis)/sizeof(c_axis[0]); i++) {
			c_axis[i] = 0;
		}
		c_mut.unlock();
		std::cout << "Spam to identify it" << std::endl;
		c_stage = (stage){c_stage.load().a, c_stage.load().b+1};
		calibrate_continue_sem.acquire();
		cal->analogs[c_i].code = std::max_element(std::begin(c_axis), std::end(c_axis))-c_axis;
		c_axis[cal->analogs[c_i].code] = 0;
		/*}}}*/

		/*{{{ get max*/
		c_mut.lock();
		c_stage = (stage){c_stage.load().a, c_stage.load().b+1};
		// clearing in case calibration_event did it twice
		calibrate_continue_sem.try_acquire();
		c_mut.unlock();
		std::cout << "Move to the positive end of the input and press any key" << std::endl;
		getchar(); std::cout << "\r\033[K";
		cal->analogs[c_i].plus_scale = c_axis[cal->analogs[c_i].code];
		/*}}}*/

		/*{{{ get right*/
		std::cout << "Move to the negative end of the input if there is one else at rest and press any key" << std::endl;
		getchar(); std::cout << "\r\033[K";
		cal->analogs[c_i].minus_scale = c_axis[cal->analogs[c_i].code];
		/*}}}*/

		/*{{{ get center and calculate scales*/
		std::cout << "Put the input at rest and press any key" << std::endl;
		getchar(); std::cout << "\r\033[K";
		cal->analogs[c_i].center = c_axis[cal->analogs[c_i].code];
		bool invert = cal->analogs[c_i].plus_scale-cal->analogs[c_i].center < 0;
		cal->analogs[c_i].plus_scale = 1/(cal->analogs[c_i].plus_scale-cal->analogs[c_i].center);
		if(fabs(cal->analogs[c_i].minus_scale-cal->analogs[c_i].center) > 0.1*fabs(1/cal->analogs[c_i].plus_scale))
			cal->analogs[c_i].minus_scale = -1/(cal->analogs[c_i].minus_scale-cal->analogs[c_i].center);
		else cal->analogs[c_i].minus_scale = cal->analogs[c_i].plus_scale;
		if(invert) std::swap(cal->analogs[c_i].plus_scale, cal->analogs[c_i].minus_scale);
		/*}}}*/

		c_stage = (stage){c_stage.load().a, c_stage.load().b+1};
		std::cout << "Check the input below and press D to discard or space to continue" << std::endl;
		c_mut.unlock();
		do { c = getchar(); std::cout << "\r\033[K"; } while(c != 'd' && c != 'D' && c != ' ');
		if(c == 'd' || c == 'D') c_i--;
		else {
			cal->analogs[c_i].present = true;
			c_stage = (stage){c_stage.load().a, c_stage.load().b+1};
			assign_type((int*)&cal->analogs[c_i].type, &cal->analogs[c_i].side);
			available_axes--;
		}
		std::cout << "\r\033[K";
	}
	/*}}}*/

	/*{{{ buttons*/
	for(c_i = 0; c_i < CNTLR_MAX_BUTTONS; c_i++) {
		c_stage = (stage){3, 0};
		std::cout << "Press C to add a button input or space to continue" << std::endl;
		do { c = getchar(); std::cout << "\r\033[K"; } while(c != 'c' && c != 'C' && c != ' ');
		if(c == ' ') break;

		/*{{{ identify*/
		std::cout << "Check the input below and then press space to identify it or D to discard" << std::endl;
		c_stage = (stage){c_stage.load().a, c_stage.load().b+1};
		do { c = getchar(); std::cout << "\r\033[K"; } while(c != 'd' && c != 'D' && c != ' ');
		if(c == 'd' || c == 'D') c_i--;
		else {
			cal->buttons[c_i].present = true;
			c_stage = (stage){c_stage.load().a, c_stage.load().b+1};
			assign_type((int*)&cal->buttons[c_i].type, &cal->buttons[c_i].side);
			cal->buttons[c_i].code = c_last_button;
		}
		std::cout << "\r\033[K";
		/*}}}*/

	}
	/*}}}*/

	c_stage = (stage){0, 0};
	calibration_save(dev);
	}

	/*{{{ set up inputs*/
	std::forward_list<input>& inputs = ::inputs.emplace_front();
	for(int i = 0; i < CNTLR_MAX_ANALOG_2DS && cal->analog_2ds[i].present; i++) {
		input& add = inputs.emplace_front();
		cal->analog_2ds[i].out = &add;
		add.physical = true;
		add.digital = false;
		add.axes = 2;
		add.calibration = &cal->analog_2ds[i];
		add.side = cal->analog_2ds[i].side;
		add.circular = cal->analog_2ds[i].circular;
		add.hash = (uintptr_t)add.calibration;
	}
	for(int i = 0; i < CNTLR_MAX_ANALOGS && cal->analogs[i].present; i++) {
		input& add = inputs.emplace_front();
		cal->analogs[i].out = &add;
		add.physical = true;
		add.digital = false;
		add.axes = 1;
		add.calibration = &cal->analogs[i];
		add.side = cal->analogs[i].side;
		add.hash = (uintptr_t)add.calibration;
	}
	for(int i = 0; i < CNTLR_MAX_BUTTONS && cal->buttons[i].present; i++) {
		input& add = inputs.emplace_front();
		cal->buttons[i].out = &add;
		add.physical = true;
		add.digital = true;
		add.axes = 1;
		add.calibration = &cal->buttons[i];
		add.side = cal->buttons[i].side;
		add.hash = (uintptr_t)add.calibration;
	}
	/*}}}*/

	cal->done = true;
}
void calibration_event(device* dev, int type, int code, int value) {
	c_mut.lock();
	stage c_stage = ::c_stage.load();
	cntlr_calibration* cal = &dev->calibration;
	switch(c_stage.a) {

	/*{{{ analog 2Ds*/
		case 1: {
			if(c_stage.b < 4 && type != EV_ABS) break;
			int min = libevdev_get_abs_minimum(dev->dev, code);
			int max = libevdev_get_abs_maximum(dev->dev, code);
			double plusminus_1 = (2*value-(min+max))/(double)(max-min);
			switch(c_stage.b) {
				case 1: {
					double temp = c_axis[code]+fabs(plusminus_1);
					c_axis[code] = 0;
					if(std::min(temp, (double)*std::max_element(std::begin(c_axis), std::end(c_axis))) > 5)
						calibrate_continue_sem.release();
					c_axis[code] = temp;
					break; }
				case 2:
					c_axis[code] = plusminus_1;
					break;
				case 3:
					if(code == cal->analog_2ds[c_i].x_code)
						cal->analog_2ds[c_i].center_x = value;
					else if(code == cal->analog_2ds[c_i].y_code)
						cal->analog_2ds[c_i].center_y = value;
					break;
				case 4:
					if(type == EV_ABS) {

		/*{{{ store c_x and c_y*/
						if(code == cal->analog_2ds[c_i].x_code) {
							c_x = value;
							c_min_x = std::min(c_min_x, value);
							c_max_x = std::max(c_max_x, value);
							c_x = c_x-cal->analog_2ds[c_i].center_x;
							c_x /= (c_x < 0) ? cal->analog_2ds[c_i].center_x-min : max-cal->analog_2ds[c_i].center_x;
							c_x *= (c_x < 0) ? cal->analog_2ds[c_i].xl_scale : cal->analog_2ds[c_i].xr_scale;
							c_updated = true;
						}
						else if(code == cal->analog_2ds[c_i].y_code) {
							c_y = value;
							c_min_y = std::min(c_min_y, value);
							c_max_y = std::max(c_max_y, value);
							c_y = c_y-cal->analog_2ds[c_i].center_y;
							c_y /= (c_y < 0) ? cal->analog_2ds[c_i].center_y-min : max-cal->analog_2ds[c_i].center_y;
							c_y *= (c_y < 0) ? cal->analog_2ds[c_i].yd_scale : cal->analog_2ds[c_i].yu_scale;
							c_updated = true;
						}
		/*}}}*/

					}
					else if(type == EV_SYN && c_updated) {

		/*{{{ handle events*/
						if(abs(c_angle_spins) <= 5) {

			/*{{{ get left/right/up/down scales*/
							if(fabs(c_x) > 1) {
								if((c_x < 0) ^ (cal->analog_2ds[c_i].xl_scale < 0))
									cal->analog_2ds[c_i].xl_scale /= fabs(c_x);
								else
									cal->analog_2ds[c_i].xr_scale /= fabs(c_x);
								c_x = 0; // don't want to do this again if only other sends event
							}
							if(fabs(c_y) > 1) {
								if((c_y < 0) ^ (cal->analog_2ds[c_i].yd_scale < 0))
									cal->analog_2ds[c_i].yd_scale /= fabs(c_y);
								else
									cal->analog_2ds[c_i].yu_scale /= fabs(c_y);
								c_y = 0; // don't want to do this again if only other sends event
							}
			/*}}}*/

						}
						else if(abs(c_angle_spins) <= 15) {
							double angle = atan2(c_y, c_x);
							if(angle < 0) angle += 2*M_PI;
							int i = (angle/(2*M_PI))*C_ANGLES_STEPS;
							c_angles[i] = std::max(c_angles[i], sqrt(pow(c_x, 2)+pow(c_y, 2)));
						}

			/*{{{ c_angle_spins updating and filling c_angles*/
						if(0.9 < fabs(c_x) && fabs(c_x) < 1.1 && fabs(c_y) < 0.1) {
							if(c_angle_spins <= 0 && c_x > 0) {
								c_angle_spins = -c_angle_spins+1;
								bool done = false;
								if(c_angle_spins >= 10) {
									done = true;
									for(int i = 0; i < C_ANGLES_STEPS; i++) {
										if(c_angles[i] == 0) { done = false; break; }
									}
								}
								std::cout << c_angle_spins << " of 10-15" << std::endl;
								if(done || c_angle_spins > 15) {

				/*{{{ fill any remaining holes in c_angles*/
									int left = 0;
									if(c_angles[left] == 0) {
										for(left = C_ANGLES_STEPS-1; left >= 0; left--) {
											if(c_angles[left] != 0) break;
										}
									}
									int right = ((left-1)+C_ANGLES_STEPS)%C_ANGLES_STEPS;
									while(true) {
										while(c_angles[left] != 0) {
											if((left = (left+1)%C_ANGLES_STEPS) == right) break;
										}
										if(left == right) break;
										left = ((left-1)+C_ANGLES_STEPS)%C_ANGLES_STEPS;
										right = (left+2)%C_ANGLES_STEPS;
										while(c_angles[right] == 0) { right = (right+1)%C_ANGLES_STEPS; }
										int count = (right > left) ? right-left : (C_ANGLES_STEPS-left)+right;
										for(int i = 1; i < count; i++) {
											c_angles[(left+i)%C_ANGLES_STEPS] = c_angles[left]+(i/(double)count)*(c_angles[right]-c_angles[left]);
										}
										left = (right+1)%C_ANGLES_STEPS;
									}
				/*}}}*/

									calibrate_continue_sem.release();
									c_angle_spins = 15+1;
								}
							}
							else if(c_angle_spins > 0 && c_x < 0)
								c_angle_spins = -c_angle_spins;
						}
			/*}}}*/
		/*}}}*/

					}
					break;
				case 5:
					if(type == EV_ABS) {

		/*{{{ store c_x and c_y*/
						if(code == cal->analog_2ds[c_i].x_code) {
							c_x = value-cal->analog_2ds[c_i].center_x;
							c_x /= (c_x < 0) ? cal->analog_2ds[c_i].center_x-min : max-cal->analog_2ds[c_i].center_x;
							c_x *= (c_x < 0) ? cal->analog_2ds[c_i].xl_scale : cal->analog_2ds[c_i].xr_scale;
							c_updated = true;
						}
						else if(code == cal->analog_2ds[c_i].y_code) {
							c_y = value-cal->analog_2ds[c_i].center_y;
							c_y /= (c_y < 0) ? cal->analog_2ds[c_i].center_y-min : max-cal->analog_2ds[c_i].center_y;
							c_y *= (c_y < 0) ? cal->analog_2ds[c_i].yd_scale : cal->analog_2ds[c_i].yu_scale;
							c_updated = true;
						}
		/*}}}*/

					}
					else if(type == EV_SYN && c_updated) {
						double c_x = ::c_x, c_y = ::c_y;
						double angle = atan2(c_y, c_x);
						if(angle < 0) angle += 2*M_PI;
						double q_progress = angle;
						while(q_progress > M_PI/2) { q_progress -= M_PI/2; }
						q_progress /= M_PI/2;
						int q = (int)(angle/(M_PI/2));
						double scale = cal->analog_2ds[c_i].q_scales[q];
						scale = 1+std::min(1.0, cos(M_PI*pow(fabs(2*q_progress-1), cal->analog_2ds[c_i].q_pows[q]))+1)*(scale-1);
						c_x *= scale; c_y *= scale;
						printf("\r\033[KX: % 2.02lf Y: % 2.02lf ", c_x, c_y);
					}
					break;
			}
			break; }
	/*}}}*/

	/*{{{ analogs*/
		case 2: {
			if(type != EV_ABS) break;
			int min = libevdev_get_abs_minimum(dev->dev, code);
			int max = libevdev_get_abs_maximum(dev->dev, code);
			double plusminus_1 = (2*value-(min+max))/(double)(max-min);
			switch(c_stage.b) {
				case 1:
					c_axis[code] += fabs(plusminus_1);
					if(c_axis[code] > 5)
						calibrate_continue_sem.release();
					break;
				case 2:
					c_axis[code] = value;
					break;
				case 3: {
					double v = value-cal->analogs[c_i].center;
					v *= (v >= 0) ? cal->analogs[c_i].plus_scale : cal->analogs[c_i].minus_scale;
					printf("\r\033[KV: % 2.02lf ", v);
					break; }
			}
			break; }
	/*}}}*/

	/*{{{ buttons*/
		case 3:
			if(type == EV_KEY) {
				if(value == 1) {
					c_last_button = code;
					std::cout << "Pressed ";
				}
				else std::cout << "\r\033[K";
			}
			break;
	/*}}}*/

	}
	c_mut.unlock();
}
/*}}}*/

int Controller::add_button(int code) {
	if(available_buttons > 0) {
		inputs_vec.emplace(inputs_vec.begin());
		inputs_vec[0].digital = true;
		codes.insert(codes.begin(), (decltype(codes)::value_type){code, 0});
		// TODO: init button
		available_buttons--;
		extra_buttons++;
	}
	else return -1;
	return CNTLR_INPUT_MAX+extra_buttons+extra_analogs+extra_analog_2ds-1;
}
int Controller::add_analog(int code) {
	if(available_axes > 0) {
		inputs_vec.emplace(inputs_vec.begin());
		inputs_vec[0].digital = false;
		inputs_vec[0].axes = 1;
		codes.insert(codes.begin(), (decltype(codes)::value_type){code, 0});
		// TODO: init axis
		available_axes--;
		extra_analogs++;
	}
	else return -1;
	return CNTLR_INPUT_MAX+extra_buttons+extra_analogs+extra_analog_2ds-1;
}
int Controller::add_analog_2d(int x_code, int y_code) {
	if(available_axes > 0) {
		inputs_vec.emplace(inputs_vec.begin());
		inputs_vec[0].digital = false;
		inputs_vec[0].axes = 2;
		codes.insert(codes.begin(), (decltype(codes)::value_type){x_code, y_code});
		// TODO: init axes
		available_axes -= 2;
		extra_analog_2ds++;
	}
	else return -1;
	return CNTLR_INPUT_MAX+extra_buttons+extra_analogs+extra_analog_2ds-1;
}
static void init_abs(int fd, int code, int min, int max) {
	ioctl(fd, UI_SET_ABSBIT, code);
	struct uinput_abs_setup setup = {
		.code = (unsigned short)code,
		.absinfo = {
			.value = 0,
			.minimum = min,
			.maximum = max,
			.fuzz = 0, .flat = 0, .resolution = 0
		}
	};
	ioctl(fd, UI_ABS_SETUP, &setup);
}
void Controller::init() {
	auto i = inputs_vec.begin();
	auto c = codes.begin();
	i = inputs_vec.emplace(i);
	(*i).digital = false;
	(*i).axes = 2;
	c = codes.insert(c, (decltype(codes)::value_type){ABS_HAT0X, ABS_HAT0Y});
	i = inputs_vec.emplace(++i);
	(*i).digital = false;
	(*i).axes = 2;
	c = codes.insert(++c, (decltype(codes)::value_type){ABS_X, ABS_Y});
	i = inputs_vec.emplace(++i);
	(*i).digital = false;
	(*i).axes = 2;
	c = codes.insert(++c, (decltype(codes)::value_type){ABS_RY, ABS_GAS});
	i = inputs_vec.emplace(++i);
	(*i).digital = false;
	(*i).axes = 1;
	c = codes.insert(++c, (decltype(codes)::value_type){ABS_RX, 0});
	i = inputs_vec.emplace(++i);
	(*i).digital = false;
	(*i).axes = 1;
	c = codes.insert(++c, (decltype(codes)::value_type){ABS_BRAKE, 0});
	i = inputs_vec.emplace(++i);
	(*i).digital = true;
	c = codes.insert(++c, (decltype(codes)::value_type){BTN_TR2, 0});
	i = inputs_vec.emplace(++i);
	(*i).digital = true;
	c = codes.insert(++c, (decltype(codes)::value_type){BTN_TL2, 0});
	i = inputs_vec.emplace(++i);
	(*i).digital = true;
	c = codes.insert(++c, (decltype(codes)::value_type){-1/*BTN_MODE*/, 0});
	i = inputs_vec.emplace(++i);
	(*i).digital = true;
	c = codes.insert(++c, (decltype(codes)::value_type){BTN_TL, 0});
	i = inputs_vec.emplace(++i);
	(*i).digital = true;
	c = codes.insert(++c, (decltype(codes)::value_type){BTN_TR, 0});
	i = inputs_vec.emplace(++i);
	(*i).digital = true;
	c = codes.insert(++c, (decltype(codes)::value_type){BTN_TL2, 0});
	i = inputs_vec.emplace(++i);
	(*i).digital = true;
	c = codes.insert(++c, (decltype(codes)::value_type){BTN_TR2, 0});
	i = inputs_vec.emplace(++i);
	(*i).digital = true;
	c = codes.insert(++c, (decltype(codes)::value_type){BTN_THUMBL, 0});
	i = inputs_vec.emplace(++i);
	(*i).digital = true;
	c = codes.insert(++c, (decltype(codes)::value_type){BTN_THUMBR, 0});
	i = inputs_vec.emplace(++i);
	(*i).digital = true;
	c = codes.insert(++c, (decltype(codes)::value_type){BTN_Y, 0});
	i = inputs_vec.emplace(++i);
	(*i).digital = true;
	c = codes.insert(++c, (decltype(codes)::value_type){BTN_A, 0});
	i = inputs_vec.emplace(++i);
	(*i).digital = true;
	c = codes.insert(++c, (decltype(codes)::value_type){BTN_X, 0});
	i = inputs_vec.emplace(++i);
	(*i).digital = true;
	c = codes.insert(++c, (decltype(codes)::value_type){BTN_B, 0});
	last_inputs_vec = decltype(inputs_vec)(inputs_vec);
	for(size_t i = 0; i < inputs_vec.size(); i++) {
		inputs_vec[i].hash = (uintptr_t)&inputs_vec[i];
	}
	inputs = inputs_vec.data();

	bool xbox = extra_buttons+extra_analogs+extra_analog_2ds == 0;
	fd = open("/dev/uinput", O_WRONLY);
	ioctl(fd, UI_SET_EVBIT, EV_KEY); // enable button handling
	ioctl(fd, UI_SET_KEYBIT, BTN_TR2);
	ioctl(fd, UI_SET_KEYBIT, BTN_TL2);
	//ioctl(fd, UI_SET_KEYBIT, BTN_MODE);
	ioctl(fd, UI_SET_KEYBIT, BTN_TL);
	ioctl(fd, UI_SET_KEYBIT, BTN_TR);
	ioctl(fd, UI_SET_KEYBIT, BTN_THUMBL);
	ioctl(fd, UI_SET_KEYBIT, BTN_THUMBR);
	ioctl(fd, UI_SET_KEYBIT, BTN_Y); // called C sometimes
	ioctl(fd, UI_SET_KEYBIT, BTN_A);
	ioctl(fd, UI_SET_KEYBIT, BTN_B);
	ioctl(fd, UI_SET_KEYBIT, BTN_X);

	ioctl(fd, UI_SET_EVBIT, EV_ABS); // enable absolute analog handling
	init_abs(fd, ABS_HAT0X, -1, 1);
	init_abs(fd, ABS_HAT0Y, -1, 1);
	init_abs(fd, ABS_X,   -32768, 32767);
	init_abs(fd, ABS_Y,   -32768, 32767);
	init_abs(fd, ABS_RY,  -32768, 32767);
	init_abs(fd, ABS_GAS, -32768, 32767);
	init_abs(fd, ABS_RX,    0, 1023);
	init_abs(fd, ABS_BRAKE, 0, 1023);

	struct uinput_setup setup = {
		.id = {
			.bustype = BUS_USB,
			.vendor  = (unsigned short)((xbox) ? 0x045E : 0x0001),
			.product = (unsigned short)((xbox) ? 0x02D1 : 0x0001),
			.version = (unsigned short)((xbox) ? 0xAB00 : 0x0001),
		},
		.name = "Xbox One Controller",
		.ff_effects_max = 0
	};
	const char name[] = "UIC Controller";
	if(!xbox) strcpy(setup.name, name);

	ioctl(fd, UI_DEV_SETUP, &setup);
	ioctl(fd, UI_DEV_CREATE);
}
void Controller::reset() {
	// TODO: reset all inputs to 0
	inputs_vec.clear();
	last_inputs_vec.clear();
	codes.clear();
	if(fd != -1) {
		ioctl(fd, UI_DEV_DESTROY);
		close(fd);
		fd = -1;
	}
}
void Controller::apply() {
	std::vector<struct input_event> events;
	for(size_t i = 0; i < inputs_vec.size(); i++) {
		int axes = (inputs_vec[i].digital) ? 1 : inputs_vec[i].axes;
		for(int a = 0; a < axes; a++) {
			if(inputs_vec[i].values[a] != last_inputs_vec[i].values[a]) {
				if(codes[i][a] < 0) continue;
				events.emplace(events.end());
				events[events.size()-1].type = (inputs_vec[i].digital) ? EV_KEY : EV_ABS;
				events[events.size()-1].code = codes[i][a];
				if(inputs_vec[i].digital)
					events[events.size()-1].value = inputs_vec[i].values[a];
				else switch(codes[i][a]) {
					case ABS_HAT0X:
					case ABS_HAT0Y:
						events[events.size()-1].value = std::round(inputs_vec[i].values[a]);
						break;
					case ABS_BRAKE:
					case ABS_RX:
						events[events.size()-1].value = std::round(1023*inputs_vec[i].values[a]);
						break;
					default:
						events[events.size()-1].value = std::round(32767*inputs_vec[i].values[a]);
				}
			}
		}
	}

	if(!events.empty()) {
		events.emplace(events.end());
		events[events.size()-1].type = EV_SYN;
		events[events.size()-1].code = SYN_REPORT;
		events[events.size()-1].value = 0;
		write(fd, events.data(), events.size()*sizeof(decltype(events)::value_type));
	}
	for(size_t i = 0; i < inputs_vec.size(); i++) { last_inputs_vec[i] = inputs_vec[i]; }
}
