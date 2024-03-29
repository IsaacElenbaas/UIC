#include <algorithm>
#include <iostream>
#include "user.h"
#include "util/util.h"

input
	*DPAD,
	*LTHUMB,
	*RTHUMB,
	*LTRIGGER,
	*RTRIGGER,
	*START,
	*SELECT,
	*HOME,
	*LBUMPER,
	*RBUMPER,
	*LTRIGGER_BUTTON,
	*RTRIGGER_BUTTON,
	*LTHUMB_BUTTON,
	*RTHUMB_BUTTON,
	*Y,
	*A,
	*X,
	*B
;
Controller* con = NULL;

void user_print_profiles() {
	std::cout << "0: Passthrough" << std::endl;
}

static input* deref(std::forward_list<input>* in_con, decltype(in_con->end()) i) { return (i != in_con->end()) ? &(*i) : NULL; }
void user_switch_profile(int profile, std::forward_list<std::forward_list<input>>* inputs) {
	if(con == NULL) con = new Controller();
	con->reset();
	// add extra inputs
	con->init();
	std::forward_list<input>* in_con = &inputs->front();
	DPAD = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.axes == 2 &&
		((analog_2d_calibrations*)i.calibration)->type == analog_2d_primary_dpad
	;}));
	LTHUMB = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.axes == 2 &&
		((analog_2d_calibrations*)i.calibration)->type == analog_2d_primary_thumbstick &&
		((analog_2d_calibrations*)i.calibration)->side == -1
	;}));
	RTHUMB = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.axes == 2 &&
		((analog_2d_calibrations*)i.calibration)->type == analog_2d_primary_thumbstick &&
		((analog_2d_calibrations*)i.calibration)->side == 1
	;}));
	LTRIGGER = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		!i.digital &&
		i.axes == 1 &&
		((analog_calibrations*)i.calibration)->type == analog_primary_trigger &&
		((analog_calibrations*)i.calibration)->side == -1
	;}));
	RTRIGGER = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		!i.digital &&
		i.axes == 1 &&
		((analog_calibrations*)i.calibration)->type == analog_primary_trigger &&
		((analog_calibrations*)i.calibration)->side == 1
	;}));
	START = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_start
	;}));
	SELECT = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_select
	;}));
	HOME = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_home
	;}));
	LBUMPER = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_bumper &&
		((button_calibrations*)i.calibration)->side == -1
	;}));
	RBUMPER = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_bumper &&
		((button_calibrations*)i.calibration)->side == 1
	;}));
	LTRIGGER_BUTTON = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_trigger &&
		((button_calibrations*)i.calibration)->side == -1
	;}));
	RTRIGGER_BUTTON = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_trigger &&
		((button_calibrations*)i.calibration)->side == 1
	;}));
	LTHUMB_BUTTON = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_thumb_press &&
		((button_calibrations*)i.calibration)->side == -1
	;}));
	RTHUMB_BUTTON = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_thumb_press &&
		((button_calibrations*)i.calibration)->side == 1
	;}));
	Y = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_face_up
	;}));
	A = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_face_down
	;}));
	X = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_face_left
	;}));
	B = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_face_right
	;}));
}

void user_process_frame() {
	if(DPAD            != NULL) con->inputs[CNTLR_DPAD]     = *DPAD;
	if(LTHUMB          != NULL) con->inputs[CNTLR_LTHUMB]   = *LTHUMB;
	if(RTHUMB          != NULL) con->inputs[CNTLR_RTHUMB]   = *RTHUMB;
	//PulseLength(28, Bounds({{0, 1, 0, -1}}, Aggregate(&con->inputs[CNTLR_RTHUMB])), Aggregate(&con->inputs[CNTLR_RTHUMB])).set(0, RTHUMB->values[0]);
	if(LTRIGGER        != NULL) con->inputs[CNTLR_LTRIGGER] = *LTRIGGER;
	//PulseLength(28, &con->inputs[CNTLR_LTRIGGER]).set(0, LTRIGGER->values[0]);
	if(RTRIGGER        != NULL) con->inputs[CNTLR_RTRIGGER] = *RTRIGGER;
	//Bounds({{0, 1, 1, 0, 0.5}}, &con->inputs[CNTLR_RTRIGGER]).set(0, RTRIGGER->values[0]);
	//Bounds({{0, 1, 0, 1, 0.35}}, &con->inputs[CNTLR_RTRIGGER]).set(0, RTRIGGER->values[0]);
	if(START           != NULL) con->inputs[CNTLR_START]           = *START;
	if(SELECT          != NULL) con->inputs[CNTLR_SELECT]          = *SELECT;
	if(HOME            != NULL) con->inputs[CNTLR_HOME]            = *HOME;
	if(LBUMPER         != NULL) con->inputs[CNTLR_LBUMPER]         = *LBUMPER;
	if(RBUMPER         != NULL) con->inputs[CNTLR_RBUMPER]         = *RBUMPER;
	if(LTRIGGER_BUTTON != NULL) con->inputs[CNTLR_LTRIGGER_BUTTON] = *LTRIGGER_BUTTON;
	if(RTRIGGER_BUTTON != NULL) con->inputs[CNTLR_RTRIGGER_BUTTON] = *RTRIGGER_BUTTON;
	if(LTHUMB_BUTTON   != NULL) con->inputs[CNTLR_LTHUMB_BUTTON]   = *LTHUMB_BUTTON;
	if(RTHUMB_BUTTON   != NULL) con->inputs[CNTLR_RTHUMB_BUTTON]   = *RTHUMB_BUTTON;
	if(Y               != NULL) con->inputs[CNTLR_Y]               = *Y;
	//Toggle(&con->inputs[CNTLR_Y]).set(0, Y->values[0]);
	if(A               != NULL) con->inputs[CNTLR_A]               = *A;
	//AutoRelease(500, Aggregate(&con->inputs[CNTLR_A])).set(0, A->values[0]);
	if(X               != NULL) con->inputs[CNTLR_X]               = *X;
	if(B               != NULL) con->inputs[CNTLR_B]               = *B;
	//TapDance(1000, {new Extend(500, &con->inputs[CNTLR_B]), new Extend(500, Aggregate(&con->inputs[CNTLR_A]))}).set(0, B->values[0]);
	con->apply();
}

void user_stop() {
	if(con != NULL) {
		con->reset();
		delete con;
		con = NULL;
	}
}
