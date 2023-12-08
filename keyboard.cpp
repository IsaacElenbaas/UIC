#include <fcntl.h>
#include <forward_list>
#include <libevdev/libevdev.h>
#include <linux/uinput.h>
#include <unistd.h>
#include "user.h"

int Keyboard::add_key(int code) {
	inputs_vec.emplace_back();
	inputs_vec[inputs_vec.size()-1].digital = true;
	codes.push_back(code);
	if(fd == -1) {
		fd = open("/dev/uinput", O_WRONLY);
		ioctl(fd, UI_SET_EVBIT, EV_KEY); // enable button handling
	}
	ioctl(fd, UI_SET_KEYBIT, code);
	return inputs_vec.size()-1;
}
void Keyboard::init() {
	if(inputs_vec.size() == 0) return;
	for(size_t i = 0; i < inputs_vec.size(); i++) {
		inputs_vec[i].hash = (uintptr_t)&inputs_vec[i];
	}
	last_inputs_vec = decltype(inputs_vec)(inputs_vec);
	inputs = inputs_vec.data();

	struct uinput_setup setup = {
		.id = {
			.bustype = BUS_USB,
			.vendor  = 0x0001,
			.product = 0x0002,
			.version = 0x0001,
		},
		.name = "UIC Keyboard",
		.ff_effects_max = 0
	};

	ioctl(fd, UI_DEV_SETUP, &setup);
	ioctl(fd, UI_DEV_CREATE);
}
void Keyboard::reset() {
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
extern unsigned int scancodes[KEY_MAX];
void Keyboard::apply() {
	std::vector<struct input_event> events;
	for(size_t i = 0; i < inputs_vec.size(); i++) {
		if(inputs_vec[i].values[0] != last_inputs_vec[i].values[0]) {
			if(codes[i] < 0) continue;
			if(scancodes[codes[i]] != 0) {
				events.emplace_back();
				events[events.size()-1].type = EV_MSC;
				events[events.size()-1].code = MSC_SCAN;
				events[events.size()-1].value = scancodes[codes[i]];
			}
			events.emplace_back();
			events[events.size()-1].type = EV_KEY;
			events[events.size()-1].code = codes[i];
			events[events.size()-1].value = inputs_vec[i].values[0];
		}
	}

	if(!events.empty()) {
		events.emplace_back();
		events[events.size()-1].type = EV_SYN;
		events[events.size()-1].code = SYN_REPORT;
		events[events.size()-1].value = 0;
		write(fd, events.data(), events.size()*sizeof(decltype(events)::value_type));
	}
	for(size_t i = 0; i < inputs_vec.size(); i++) { last_inputs_vec[i] = inputs_vec[i]; }
}
