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
int KB_EXTRA[17];
enum {
	extra_hookline,
	extra_f3,
	extra_f4,
	extra_repeat_emote,
	extra_recover,
	extra_kill,
	extra_kill_command,
	extra_left,
	extra_right,
	extra_hammer,
	extra_switch_weapon,
	extra_hook,
	extra_spec,
	extra_reset_zoom,
	extra_zoom_out,
	extra_disable_hookline,
	extra_enable_hookline,
};
bool fire_last;
bool hook_last;
int KB_EMOTE[6];
int emote_last = -1;
bool emoting = false;
bool emote_l, emote_r = false;
double emote_time = 0;

#define SPIN_TICK 10
double spin_interval_this = 0;
double spin_interval_last;
#define FLY_SPIN_ENABLED true
#define FLY_SPIN_COUNT 5
#define FLY_SPIN_WINDOW 30
unsigned int fly_spin = 0;
double hook_time = 0;
double fire_time = 0;
double fly_spin_interval;
double fly_spin_intervals[FLY_SPIN_COUNT];
#define IDLE_SPIN_ENABLED true
#define IDLE_SPIN_COUNT 10
#define IDLE_SPIN_MIN_INTERVAL 250
#define IDLE_SPIN_VARIANCE 50
#define IDLE_ANGLE_RANGE (M_PI/2/8)
unsigned int idle_spin = 0;
double idle_spin_center;
double idle_spin_interval;
double idle_spin_intervals[IDLE_SPIN_COUNT];

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

void user_process_frame() {

/*{{{ dpad*/
	Bounds({{0, -1, 0, -1}, {0, 1, 0, 0}}, &kb->inputs[KB_EXTRA[extra_hookline]]).set(0, DPAD->values[1]);
	Bounds({{0, -1, 0,  0}, {0, 1, 0, 1}}, TapDance(1000, {new Extend(50, &kb->inputs[KB_EXTRA[extra_f3]]), new Extend(50, &kb->inputs[KB_EXTRA[extra_f4]])})).set(0, DPAD->values[1]);
	Bounds({{0, -1, 0, -1}, {0, 1, 0, 0}}, AutoRelease(50, &kb->inputs[KB_EXTRA[extra_repeat_emote]])).set(0, DPAD->values[0]);
	if(kb->inputs[KB_EXTRA[extra_repeat_emote]].values[0] != 0) {
		kb->inputs[KB_EXTRA[extra_repeat_emote]].values[0] = 0;
		if(emote_last != -1) {
			kb->inputs[KB_EMOTE[emote_last]].values[0] = 1;
			emote_time = 100+elapsed; push_timer(0, 100, false);
			emoting = false;
			emote_l = false;
			emote_r = false;
		}
	}
	Bounds({{0, -1, 0,  0}, {0, 1, 0, 1}}, TapDance(300, {new Extend(50, &kb->inputs[KB_EXTRA[extra_recover]]), new Extend(50, &kb->inputs[KB_EXTRA[extra_kill]]), new Extend(50, &kb->inputs[KB_EXTRA[extra_kill_command]])})).set(0, DPAD->values[0]);
/*}}}*/

/*{{{ right trigger*/
	hook_last = kb->inputs[KB_EXTRA[extra_hook]].values[0] != 0;
	RapidTrigger(0.1, 0.1, Extend(50, &kb->inputs[KB_EXTRA[extra_hook]])).set(0, RTRIGGER->values[0]);
	if(RTRIGGER->values[0] > 0.8) kb->inputs[KB_EXTRA[extra_hook]].values[0] = 1;
/*}}}*/

/*{{{ left thumbstick*/
	double x = Deadzone(0.15, Deadzone(-(0.05+0.15*abs(LTHUMB->values[1])), Noop())).set(0, LTHUMB->values[0]);

	if(LTHUMB->values[1] > 0.85) {
		x = (kb->inputs[KB_EXTRA[extra_hook]].values[0] == 0) ? std::copysign(1, RTHUMB->values[0]) : std::copysign(0.1, -RTHUMB->values[0]);
	}

	// Teeworlds physics ticks are at 50Hz
	// want an input rate very much not a factor of 50 (so error is random and evens out)
	// sum_(i=1)^n(min((floor(ix/50)+1)*50-ix, 50-same)/n)
	// n is precision, want something just under 50 (as often as possible, but not so fast that an event can happen in less than one physics frame)
	// sinusoidal decaying from 49, I probably should have guessed that
	PulseLength(1000/49.0, &kb->inputs[KB_EXTRA[extra_left]], &kb->inputs[KB_EXTRA[extra_right]]).set(0, x);
/*}}}*/

/*{{{ right thumbstick*/
	con->inputs[CNTLR_RTHUMB] = *RTHUMB;
	double a;
	// this is option 2 in my saved desmos things
	{
	double angles[] = {0, 20, 70, 90, 110, 160, 180, 200, 250, 270, 290, 340};
	int angle_count = (int)(sizeof(angles)/sizeof(double));
	for(int i = 0; i < angle_count; i++) { angles[i] *= M_PI/180; }
	double s = sqrt(pow(RTHUMB->values[0], 2)+pow(RTHUMB->values[1], 2));
	a = atan2(RTHUMB->values[1], RTHUMB->values[0]); if(a < 0) a += 2*M_PI;
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
	if(s < 0.75) idle_spin = 0;
	}

	/*{{{ spin*/
	hook_time += elapsed_real;
	fire_time += elapsed_real;
	spin_interval_this += elapsed_real;
	if(kb->inputs[KB_EXTRA[extra_hook]].values[0] != 0 && !hook_last) {
		hook_time = 0;
		spin_interval_last = spin_interval_this;
		if(IDLE_SPIN_ENABLED) {
			if(idle_spin == 0 || abs(a-idle_spin_center) > IDLE_ANGLE_RANGE/2) {
				idle_spin = 1; idle_spin_center = a;
			}
			else {
				idle_spin++;
				if(idle_spin >= IDLE_SPIN_COUNT) push_timer(0, SPIN_TICK, false);
			}
			if(idle_spin <= IDLE_SPIN_COUNT) {
				if(idle_spin == 2) {
					idle_spin_interval = spin_interval_this;
					idle_spin_intervals[0] = spin_interval_this;
				}
				idle_spin_interval = ((idle_spin-1)*idle_spin_interval+spin_interval_this)/idle_spin;
				idle_spin_intervals[idle_spin-1] = spin_interval_this;
				if(abs(spin_interval_this-idle_spin_interval) > IDLE_SPIN_VARIANCE) idle_spin = 0;
			}
			else {
				int index = idle_spin%IDLE_SPIN_COUNT;
				idle_spin_interval = (IDLE_SPIN_COUNT*idle_spin_interval-idle_spin_intervals[index]+spin_interval_this)/IDLE_SPIN_COUNT;
				idle_spin_intervals[index] = spin_interval_this;
			}
		}
		if(fly_spin < FLY_SPIN_COUNT)
			spin_interval_this = 0;
	}
	if(RBUMPER->values[0] != 0 && !fire_last) {
		fire_time = 0;
		if(fly_spin >= FLY_SPIN_COUNT)
			spin_interval_this = 0;
	}
	if(hook_time == 0 || fire_time == 0) {
		if(FLY_SPIN_ENABLED && hook_time+fire_time < FLY_SPIN_WINDOW) {
			fly_spin++;
			if(fly_spin >= FLY_SPIN_COUNT) push_timer(0, SPIN_TICK, false);
			if(fly_spin <= FLY_SPIN_COUNT) {
				if(fly_spin == 2) {
					fly_spin_interval = spin_interval_last;
					fly_spin_intervals[0] = spin_interval_last;
				}
				fly_spin_interval = ((fly_spin-1)*fly_spin_interval+spin_interval_last)/fly_spin;
				fly_spin_intervals[fly_spin-1] = spin_interval_last;
			}
			else {
				int index = fly_spin%FLY_SPIN_COUNT;
				fly_spin_interval = (FLY_SPIN_COUNT*fly_spin_interval-fly_spin_intervals[index]+spin_interval_last)/FLY_SPIN_COUNT;
				fly_spin_intervals[index] = spin_interval_last;
			}
		}
		else if(hook_time+fire_time > 2000) fly_spin = 0;
	}
	if(fly_spin >= FLY_SPIN_COUNT) idle_spin = 0;
	if(idle_spin >= IDLE_SPIN_COUNT && abs(a-idle_spin_center) > IDLE_ANGLE_RANGE/2) idle_spin = 0;
	bool hookline_off = false;
	if(
		(
			idle_spin >= IDLE_SPIN_COUNT ||
			fly_spin >= FLY_SPIN_COUNT
		) && (
			(fly_spin >= FLY_SPIN_COUNT)
				? (spin_interval_this < fly_spin_interval)
				: (idle_spin_interval > IDLE_SPIN_MIN_INTERVAL && spin_interval_this < idle_spin_interval)
		) &&
		hook_time > 1.5*(1000/50.0) && fire_time > 1.5*(1000/50.0)
	) {
		if(spin_interval_this > 1.5*(1000/50.0)) {
			a += (fly_spin >= FLY_SPIN_COUNT)
				?                                  (spin_interval_this/fly_spin_interval)*(2*M_PI)
				: floor(idle_spin/IDLE_SPIN_COUNT)*(spin_interval_this/idle_spin_interval)*(2*M_PI)
			;
			con->inputs[CNTLR_RTHUMB].values[0] = cos(a);
			con->inputs[CNTLR_RTHUMB].values[1] = sin(a);
			hookline_off = true;
		}
		push_timer(0, SPIN_TICK, false);
	}
	AutoRelease(50, Extend(50, &kb->inputs[KB_EXTRA[extra_disable_hookline]])).set(0, (hookline_off) ? 1 : 0);
	AutoRelease(50, Extend(50, &kb->inputs[KB_EXTRA[extra_enable_hookline]])).set(0, (hookline_off) ? 0 : 1);
	/*}}}*/
/*}}}*/

/*{{{ left trigger*/
	RapidTrigger(0.05, 0.05, Extend(50, &kb->inputs[KB_EXTRA[extra_switch_weapon]])).set(0, LTRIGGER->values[0]);
	if(Thresholds(0.1, 0.9, &kb->inputs[KB_EXTRA[extra_hammer]]).set(0, LTRIGGER->values[0]) != 0)
		kb->inputs[KB_EXTRA[extra_switch_weapon]].values[0] = 0;
/*}}}*/

	con->inputs[CNTLR_START] = *START;
	con->inputs[CNTLR_SELECT] = *SELECT;
	kb->inputs[KB_EXTRA[extra_spec]] = *HOME;
	con->inputs[CNTLR_LBUMPER] = *LBUMPER;
	con->inputs[CNTLR_RBUMPER] = *RBUMPER;
	fire_last = RBUMPER->values[0] != 0;
	con->inputs[CNTLR_Y] = *Y;
	con->inputs[CNTLR_A] = *A;
	con->inputs[CNTLR_X] = *X;
	TapDance(1000, {new Noop(), new Extend(50, &con->inputs[CNTLR_B])}).set(0, B->values[0]);

/*{{{ emoting*/
	static int emote_layer;
	static double emote_timeout;
	double emote_LBACK = Extend(30, Noop(LBACK)).set(0, LBACK->values[0]);
	double emote_RBACK = Extend(30, Noop(RBACK)).set(0, RBACK->values[0]);
	if(!emoting) {
		if(emote_time <= 0) TapDance(1000, {new Rot(Extend(50, &kb->inputs[KB_EXTRA[extra_reset_zoom]])), new Extend(50, &kb->inputs[KB_EXTRA[extra_zoom_out]])}).set(0, RBACK->values[0]);
		if(emote_l && emote_LBACK == 0) {
			emote_l = false;
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
			if(emote_LBACK == 0 && emote_RBACK == 0) {
				emote_layer = -emote_layer;
				emote_l = false;
				emote_r = false;
			}
		}
		else if((emote_l && (emote_LBACK == 0)) || (emote_r && (emote_RBACK == 0))) {
			emote_timeout = 3000+elapsed;
			if(emote_l && emote_r) {
				emote_layer = -std::min(2, emote_layer+1);
			}
			else if(emote_l != 0) {
				emote_last = 2*emote_layer;
				kb->inputs[KB_EMOTE[emote_last]].values[0] = 1;
				emote_time = 100; push_timer(0, 100, false);
				emoting = false;
			}
			else {
				emote_last = 2*emote_layer+1;
				kb->inputs[KB_EMOTE[emote_last]].values[0] = 1;
				emote_time = 100; push_timer(0, 100, false);
				emoting = false;
			}
			emote_l = false;
			emote_r = false;
		}
		else if(emote_timeout-elapsed <= 0) {
			emoting = false;
			emote_l = false;
			emote_r = false;
		}
		emote_timeout -= elapsed;
	}
	if(!emote_l) emote_l = emote_LBACK != 0;
	if(!emote_r) emote_r = emote_RBACK != 0;
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
