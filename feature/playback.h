#ifndef PLAYBACK_H
#define PLAYBACK_H
#include <forward_list>
#include "user.h"

// process should be called at the end of user_process_frame before any relevant outputs are apply()d while the Playback exists
// one can call record (even immediately) after play to play, then resume recording from the end
class Playback {
	bool recording;
	bool playing;
	double timer;
	size_t max_events;
	size_t events;
	size_t event;
	double* timestamps;
	int* devices; // must be signed, we use for controller vs keyboard
	size_t* inputs;
	int* axes;
	double* values;
	std::forward_list<Controller*> cons;
	std::forward_list<Keyboard*> kbs;
	std::vector<std::vector<input>> con_inputs_vecs;
	std::vector<std::vector<input>> kb_inputs_vecs;
	std::vector<std::vector<input>> con_inputs_vecs_play;
	std::vector<std::vector<input>> kb_inputs_vecs_play;
public:
	Playback(size_t max_events, std::forward_list<Controller*> cons, std::forward_list<Keyboard*> kbs);
	Playback(size_t max_events, std::forward_list<Controller*> cons) : Playback(max_events, cons, {}) {};
	Playback(size_t max_events, std::forward_list<Keyboard*> kbs) : Playback(max_events, {}, kbs) {};
	~Playback();
	void process();
	void record();
	void stop_record(double erase_ms);
	void toggle_record(double erase_ms) { if(!recording) record(); else stop_record(erase_ms); }
	void play();
	void stop_play();
	void toggle_play() { if(!playing) play(); else stop_play(); }
	void clear();
};
#endif
