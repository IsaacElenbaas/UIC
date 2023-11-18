#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <libevdev/libevdev.h>
#include <limits>
#include <mutex>
#include <semaphore>
#include "controller.h"

#define C_ANGLES_STEPS 100

std::mutex c_mut;

static int c_i;
static double c_axis[REL_MAX];
static bool c_updated;
static double c_x, c_y;
static int c_min_x, c_max_x, c_min_y, c_max_y;
static int c_angle_spins;
static double c_angles[C_ANGLES_STEPS];

typedef struct stage { int a, b; } stage;
static std::atomic<stage> c_stage = (stage){0, 0};
static std::binary_semaphore calibrate_continue_sem{0};
void calibrate(device* dev) {
	dev->calibration.done = false;
	cntlr_calibration* cal = &dev->calibration;
	// TODO: needs to handle ctrl+c and input disconnecting
	// TODO: attempt to load calibration from file
	char c;
	for(c_i = 0; c_i < CNTLR_MAX_ANALOG_2D; c_i++) {
		c_stage = (stage){1, 0};
		std::cout << "Press space to add a 2D input or C to continue" << std::endl;
		do { c = getchar(); std::cout << "\r\033[K"; } while(c != ' ' && c != 'c' && c != 'C');
		if(c == 'c' || c == 'C') break;

	/*{{{ identify*/
		c_mut.lock(); memset(c_axis, 0, sizeof(c_axis)); c_mut.unlock();
		std::cout << "Spin to identify it" << std::endl;
		c_stage = (stage){c_stage.load().a, c_stage.load().b+1};
		calibrate_continue_sem.acquire();
		cal->analog_2ds[c_i].x_code = std::max_element(std::begin(c_axis), std::end(c_axis))-c_axis;
		c_axis[cal->analog_2ds[c_i].x_code] = 0;
		cal->analog_2ds[c_i].y_code = std::max_element(std::begin(c_axis), std::end(c_axis))-c_axis;
	/*}}}*/

	/*{{{ get top*/
		c_mut.lock();
		memset(c_axis, 0, sizeof(c_axis));
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
		c_mut.lock(); memset(c_axis, 0, sizeof(c_axis)); c_mut.unlock();
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
		std::cout << "Check the input below and press C to continue or D to discard" << std::endl;
		c_mut.unlock();
		do { c = getchar(); std::cout << "\r\033[K"; } while(c != 'c' && c != 'C' && c != 'd' && c != 'D');
		if(c == 'd' || c == 'D') c_i--;
		else cal->analog_2ds[c_i].present = true;
		c_stage = (stage){c_stage.load().a, c_stage.load().b+1};
		std::cout << "\r\033[K";
	}
	// TODO: save calibration to file
	dev->calibration.done = true;
}
void calibration_event(device* dev, int type, int code, int value) {
	c_mut.lock();
	stage c_stage = ::c_stage.load();
	cntlr_calibration* cal = &dev->calibration;
	switch(c_stage.a) {
		case 1: {
			if(c_stage.b < 4 && type != EV_ABS) break;
			int min = libevdev_get_abs_minimum(dev->dev, code);
			int max = libevdev_get_abs_maximum(dev->dev, code);
			double plusminus_1 = (2*value-(min+max))/(double)(max-min);
			switch(c_stage.b) {
				case 1: {
					double temp = c_axis[code]+fabs(plusminus_1);
					c_axis[code] = 0;
					if(std::min(temp, *std::max_element(std::begin(c_axis), std::end(c_axis))) > 5)
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
	}
	c_mut.unlock();
}
