#include <csignal>
#include <iostream>
#include "playback.h"

Playback::Playback(size_t max_events, std::forward_list<Controller*> cons, std::forward_list<Keyboard*> kbs) : recording(false), playing(false), max_events(max_events), events(0), cons(cons), kbs(kbs) {
	timestamps = new double[max_events];
	devices = new int[max_events];
	inputs = new size_t[max_events];
	axes = new int[max_events];
	values = new double[max_events];
	if(timestamps == NULL || devices == NULL || inputs == NULL || axes == NULL || values == NULL) {
		std::cerr << "Failed to malloc for new Playback!" << std::endl;
		std::raise(SIGINT);
	}
}
Playback::~Playback() {
	push_timer(0, 0, false);
	delete timestamps;
	delete devices;
	delete inputs;
	delete axes;
	delete values;
}
void Playback::process() {
	timer += elapsed_real;
	if(playing) {
		while(event < events && timestamps[event] <= timer+1) {
			std::vector<input>* inputs_vec = (devices[event] > 0)
				? &con_inputs_vecs_play[devices[event]-1]
				: &kb_inputs_vecs_play[(-devices[event])-1]
			;
			(*inputs_vec)[inputs[event]].values[axes[event]] = values[event];
			event++;
		}
		int device = 0;
		auto ci = cons.begin();
		auto ki = kbs.begin();
		while(true) {
			if(device >= 0 && ci != cons.end()) {
				device++;
				if(device != 1) ++ci;
				if(ci == cons.end()) continue;
				(*ci)->inputs_vec = con_inputs_vecs_play[device-1];
			}
			else {
				if(device >= 0) device = 0;
				device--;
				if(device != -1) ++ki;
				if(ki == kbs.end()) break;
				(*ki)->inputs_vec = kb_inputs_vecs_play[(-device)-1];
			}
		}
		if(event < events) push_timer(0, timestamps[event]-timer, false);
		else playing = false;
	}
	else if(recording) {
		int device = 0;
		auto ci = cons.begin();
		auto ki = kbs.begin();
		while(true) {
			if(device >= 0 && ci != cons.end()) {
				device++;
				if(device != 1) ++ci;
				if(ci == cons.end()) continue;
			}
			else {
				if(device >= 0) device = 0;
				device--;
				if(device != -1) ++ki;
				if(ki == kbs.end()) break;
			}
			std::vector<input>* inputs_vec = &((device > 0) ? (*ci)->inputs_vec : (*ki)->inputs_vec);
			std::vector<input>* last_inputs_vec = &((device > 0) ? (*ci)->last_inputs_vec : (*ki)->last_inputs_vec);
			for(size_t input = 0; input < inputs_vec->size(); input++) {
				int axes_count = ((*inputs_vec)[input].digital) ? 1 : (*inputs_vec)[input].axes;
				for(int axis = 0; axis < axes_count; axis++) {
					if((*inputs_vec)[input].values[axis] != (*last_inputs_vec)[input].values[axis]) {
						if(events == max_events) {
							while(events > 0 && timestamps[events-1] == timer) { events--; }
							recording = false;
							return;
						}
						timestamps[events] = timer;
						devices[events] = device;
						inputs[events] = input;
						axes[events] = axis;
						values[events] = (*inputs_vec)[input].values[axis];
						events++;
					}
				}
			}
		}
	}
}
void Playback::record() {
	recording = true;
	if(!playing) {
		timer = -elapsed_real;
		events = 0;
		int device = 0;
		auto ci = cons.begin();
		auto ki = kbs.begin();
		con_inputs_vecs.clear();
		kb_inputs_vecs.clear();
		while(true) {
			if(device >= 0 && ci != cons.end()) {
				device++;
				if(device != 1) ++ci;
				if(ci == cons.end()) continue;
				con_inputs_vecs.push_back((*ci)->last_inputs_vec);
			}
			else {
				if(device >= 0) device = 0;
				device--;
				if(device != -1) ++ki;
				if(ki == kbs.end()) break;
				kb_inputs_vecs.push_back((*ki)->last_inputs_vec);
			}
		}
	}
}
void Playback::stop_record(double erase_ms) {
	recording = false;
	double timestamp = timer+elapsed_real-erase_ms;
	while(events > 0 && timestamps[events-1] <= timestamp) { events--; }
}
void Playback::play() {
	if(recording) return;
	playing = true;
	timer = -elapsed_real;
	event = 0;
	con_inputs_vecs_play = con_inputs_vecs;
	kb_inputs_vecs_play = kb_inputs_vecs;
}
void Playback::stop_play() {
	if(!playing) return;
	playing = false;
	push_timer(0, 0, false);
}
void Playback::clear() {
	events = 0;
	playing = false;
	recording = false;
	con_inputs_vecs.clear();
	kb_inputs_vecs.clear();
	push_timer(0, 0, false);
}
