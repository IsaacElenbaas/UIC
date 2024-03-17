#include <algorithm>
#include <chrono>
#include <cmath>
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
	*B,
	*LBACK,
	*RBACK
;
Controller* con = NULL;
Keyboard* kb = NULL;
int KB_EXTRA[15];
int KB_EMOTE[6];

void user_print_profiles() {
	std::cout << "0: Passthrough" << std::endl;
}

/*{{{ void user_switch_profile(int profile, std::forward_list<std::forward_list<input>>* inputs)*/
static input* deref(std::forward_list<input>* in_con, decltype(in_con->end()) i) { return (i != in_con->end()) ? &(*i) : NULL; }
void user_switch_profile(int profile, std::forward_list<std::forward_list<input>>* inputs) {
	if(con == NULL) con = new Controller();
	con->reset();
	con->init();
	if(kb == NULL) kb = new Keyboard();
	kb->reset();
	for(int i = 0; i < (int)(sizeof(KB_EXTRA)/sizeof(KB_EXTRA[0])); i++) {
		KB_EXTRA[i] = kb->add_key(KEY_1+i);
	}
	for(int i = 0; i < (int)(sizeof(KB_EMOTE)/sizeof(KB_EMOTE[0])); i++) {
		KB_EMOTE[i] = kb->add_key(KEY_Z+i);
	}
	kb->init();
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
	LBACK = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_other &&
		((button_calibrations*)i.calibration)->side == -1
	;}));
	RBACK = deref(in_con, std::find_if(in_con->begin(), in_con->end(), [](input i) { return
		i.digital &&
		((button_calibrations*)i.calibration)->type == button_other &&
		((button_calibrations*)i.calibration)->side == 1
	;}));
}
/*}}}*/

bool emoting = false, emote_l, emote_r;
int emote_layer;
double emote_time = 0, emote_timeout;
void user_process_frame() {
	int E = 0;
	Bounds({{0, -1, 0, -1}, {0, 1, 0, 0}}, &kb->inputs[KB_EXTRA[E++]]).set(0, DPAD->values[1]);
	Bounds({{0, -1, 0,  0}, {0, 1, 0, 1}}, TapDance(1000, {new Extend(100, &kb->inputs[KB_EXTRA[E++]]), new Extend(100, &kb->inputs[KB_EXTRA[E++]])})).set(0, DPAD->values[1]);
	Bounds({{0, -1, 0, -1}, {0, 1, 0, 0}}, &kb->inputs[KB_EXTRA[E++]]).set(0, DPAD->values[0]);
	Bounds({{0, -1, 0,  0}, {0, 1, 0, 1}}, TapDance(300, {new Extend(100, &kb->inputs[KB_EXTRA[E++]]), new Extend(100, &kb->inputs[KB_EXTRA[E++]]), new Extend(100, &kb->inputs[KB_EXTRA[E++]])})).set(0, DPAD->values[0]);
	double a = atan2(RTHUMB->values[1], RTHUMB->values[0]); if(a < 0) a += 2*M_PI;
	double verticality = (a > M_PI+(M_PI/180)*20 && a < 2*M_PI-(M_PI/180)*20)
		? (a < M_PI+M_PI/2-(M_PI/180)*20 || a > M_PI+M_PI/2+(M_PI/180)*20)
			? 0//1-std::max((M_PI+M_PI/2-(M_PI/180)*20)-a, a-(M_PI+M_PI/2+(M_PI/180)*20))/(M_PI/2-2*(M_PI/180)*20)
			: 0//1
		: 0;
	// Teeworlds physics ticks are at 50Hz
	// want an input rate very much not a factor of 50 (so error is random and evens out)
	// sum_(i=1)^n(min((floor(ix/50)+1)*50-ix, 50-same)/n)
	// n is precision, want something just under 50 (as often as possible, but not so fast that an event can happen in less than one physics frame)
	// sinusoidal decaying from 49, I probably should have guessed that
	PulseLength(1000/49.0, &kb->inputs[KB_EXTRA[E]], &kb->inputs[KB_EXTRA[E+1]]).set(0,
		Deadzone(-(0.05+0.15*fabs(LTHUMB->values[1])),
			Deadzone(0.05, Noop())
		).set(0, (1-0.5*verticality)*LTHUMB->values[0])
	);
	E += 2;

/*{{{ angle magnet*/
	con->inputs[CNTLR_RTHUMB] = *RTHUMB;
	// this is option 2 in my saved desmos things
	{
	double angles[] = {0, 20, 70, 90, 110, 160, 180, 200, 250, 270, 290, 340};
	int angle_count = (int)(sizeof(angles)/sizeof(double));
	for(int i = 0; i < angle_count; i++) { angles[i] *= M_PI/180; }
	double s = sqrt(pow(RTHUMB->values[0], 2)+pow(RTHUMB->values[1], 2));
	double low = angles[(int)(a*angle_count/(2*M_PI))];
	double high = angles[((int)(a*angle_count/(2*M_PI)+1))%angle_count];
	double p = a*angle_count/(2*M_PI); p -= floor(p);
	auto magnet = [](double p, double c) {
		return std::max(0.0, 1-c)*p + std::min(1.0, c)*(tanh(std::max(1.0, c)*sin(M_PI*(p-0.5)))/tanh(std::max(1.0, c))+1)/2;
	};
	// magnet second param is snap strength
	a = low+magnet(p, 0.5*s)*((high-low > 0) ? high-low : high-low+2*M_PI);
	con->inputs[CNTLR_RTHUMB].values[0] = s*cos(a);
	con->inputs[CNTLR_RTHUMB].values[1] = s*sin(a);
	}
/*}}}*/

	if(Thresholds(0.1, 0.9, &kb->inputs[KB_EXTRA[E++]]).set(0, LTRIGGER->values[0]) == 0)
		RapidTrigger(0.1, 0.05, Extend(100, &kb->inputs[KB_EXTRA[E++]])).set(0, LTRIGGER->values[0]);
	else E++;
	RapidTrigger(0.1, 0.05, Extend(100, &kb->inputs[KB_EXTRA[E++]])).set(0, RTRIGGER->values[0]);
	con->inputs[CNTLR_START] = *START;
	con->inputs[CNTLR_SELECT] = *SELECT;
	kb->inputs[KB_EXTRA[E++]] = *HOME;
	con->inputs[CNTLR_LBUMPER] = *LBUMPER;
	con->inputs[CNTLR_RBUMPER] = *RBUMPER;
	con->inputs[CNTLR_Y] = *Y;
	con->inputs[CNTLR_A] = *A;
	con->inputs[CNTLR_X] = *X;
	TapDance(1000, {new Noop(), new Extend(100, &con->inputs[CNTLR_B])}).set(0, B->values[0]);

/*{{{ emoting*/
	double emote_LBACK = Extend(30, Noop(LBACK)).set(0, LBACK->values[0]);
	double emote_RBACK = Extend(30, Noop(RBACK)).set(0, RBACK->values[0]);
	if(!emoting) {
		if(emote_time <= 0) TapDance(1000, {new Rot(Extend(100, &kb->inputs[KB_EXTRA[E]])), new Extend(100, &kb->inputs[KB_EXTRA[E+1]])}).set(0, RBACK->values[0]);
		if(emote_l && emote_LBACK == 0) {
			emote_layer = 0;
			emoting = true;
			emote_timeout = 3000;
			for(int i = 0; i < (int)(sizeof(KB_EMOTE)/sizeof(KB_EMOTE[0])); i++) {
				kb->inputs[KB_EMOTE[i]].values[0] = 0;
			}
		}
		else if(emote_time > 0) {
			emote_time -= elapsed;
			if(emote_time <= 0) {
				for(int i = 0; i < (int)(sizeof(KB_EMOTE)/sizeof(KB_EMOTE[0])); i++) {
					kb->inputs[KB_EMOTE[i]].values[0] = 0;
				}
			}
		}
	}
	else {
		if(emote_layer < 0) {
			if(emote_LBACK == 0 && emote_RBACK == 0) emote_layer = -emote_layer;
		}
		else if((emote_l && emote_LBACK == 0) || (emote_r && emote_RBACK == 0)) {
			if(emote_LBACK+emote_RBACK != 0) {
				emote_layer = -std::min(2, emote_layer+1);
			}
			else if(emote_l != 0) {
				kb->inputs[KB_EMOTE[2*emote_layer]].values[0] = 1;
				emote_time = 100; push_timer(0, 100, false);
				emoting = false;
			}
			else {
				kb->inputs[KB_EMOTE[2*emote_layer+1]].values[0] = 1;
				emote_time = 100; push_timer(0, 100, false);
				emoting = false;
			}
			emote_timeout = 3000;
		}
		else if(emote_timeout-elapsed <= 0) {
			emoting = false;
		}
		emote_timeout -= elapsed;
	}
	emote_l = emote_LBACK != 0;
	emote_r = emote_RBACK != 0;
/*}}}*/

	con->apply();
	kb->apply();
}

void user_stop() {
	if(con != NULL) {
		con->reset();
		delete con;
		con = NULL;
	}
	if(kb != NULL) {
		kb->reset();
		delete kb;
		kb = NULL;
	}
}
