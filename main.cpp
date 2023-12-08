/*{{{ includes*/
#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <fcntl.h>
#include <filesystem>
#include <forward_list>
#include <future>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <mutex>
#include <semaphore>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include "controller.h"
#include "main.h"
#include "user.h"
/*}}}*/

static std::list<device> devices;
static std::counting_semaphore inputs_sem{0};
std::forward_list<std::forward_list<input>> inputs;
std::forward_list<std::forward_list<input>> user_inputs;
double elapsed = 0;

static std::atomic<bool> stopping = false;
static void signal(int signum) {
	(void)signum;
	std::cout << "\r\033[KSubmit an input on each device you used that is still connected to exit" << std::endl;
	stopping = true;
	user_stop();
	close(STDIN_FILENO);
	// unfortunately closing evdev device FDs or freeing the device objects does not interrupt next_event
}

/*{{{ void push/pop_timer(uintptr_t hash, int time_ms, bool repeat)*/
static std::forward_list<std::tuple<double, double, uintptr_t>> clock_timers;

void push_timer(uintptr_t hash, double time_ms, bool repeat) {
	clock_timers.push_front({hash, ((repeat) ? 1 : -1)*abs(time_ms), 0});
}

void pop_timer(uintptr_t hash, double time_ms, bool repeat) {
	time_ms = ((repeat) ? 1 : -1)*abs(time_ms);
	for(auto i = clock_timers.begin(), last = clock_timers.before_begin(); i != clock_timers.end(); last = i++) {
		if(std::get<0>(*i) == hash && (
			std::get<1>(*i) == time_ms || time_ms == -1
		)) {
			clock_timers.erase_after(i = last);
			if(time_ms != -1) break;
		}
	}
}
/*}}}*/

/*{{{ clock*/
static std::mutex clock_mut;
static std::promise<void>* clock_timer_promise = NULL;
static std::shared_future<void> clock_timer;
static std::binary_semaphore clock_interrupt_sem{1};
static bool clock_interrupt_later = false;
static void process_frame();
static void clock_thread() {
	clock_mut.lock();
	clock_timer_promise = new std::promise<void>();
	clock_timer = clock_timer_promise->get_future().share();
	std::promise<void>* clock_timer_promise = ::clock_timer_promise;
	std::shared_future<void> clock_timer = ::clock_timer;
	clock_mut.unlock();
	auto last = std::chrono::high_resolution_clock::now();
	double error = 0;
	while(true) {
		int sleep = std::numeric_limits<int>::max();
		clock_mut.lock();
		if(::clock_timer_promise != clock_timer_promise) {
			delete clock_timer_promise;
			clock_timer_promise = ::clock_timer_promise;
			clock_timer = ::clock_timer;
			clock_interrupt_sem.release();
		}
		if(!clock_interrupt_later) {
			clock_mut.unlock();
			for(auto i = clock_timers.begin(); i != clock_timers.end(); ++i) {
				sleep = std::min(sleep, (int)std::round(abs(std::get<1>(*i))-std::get<2>(*i)));
			}
			// account for time spent in process_frame and average error
			sleep = std::max(0.0, sleep-round(((std::chrono::duration<double, std::milli>)(std::chrono::high_resolution_clock::now()-last)).count()+error));
		}
		else {
			clock_interrupt_later = false;
			clock_mut.unlock();
		}
		std::future_status status = std::future_status::ready;
		if(sleep > 0) {
			if(sleep != std::numeric_limits<int>::max())
				status = clock_timer.wait_for(std::chrono::milliseconds(sleep));
			else
				clock_timer.wait();
		}
		if(stopping) break;
		auto now = std::chrono::high_resolution_clock::now();
		elapsed = ((std::chrono::duration<double, std::milli>)(now-last)).count();
		// things expecting to fire because of their timer should always do so
		if(status == std::future_status::timeout && sleep != std::numeric_limits<int>::max())
			elapsed = std::max(elapsed, sleep+error+0.1);
		last = now;
		for(auto i = clock_timers.begin(), last = clock_timers.before_begin(); i != clock_timers.end(); last = i++) {
			std::get<2>(*i) += elapsed;
			if(std::get<2>(*i) >= abs(std::get<1>(*i))) {
				if(std::get<1>(*i) > 0) {
					while(std::get<2>(*i) > std::get<1>(*i)) {
						std::get<2>(*i) -= std::get<1>(*i);
					}
				}
				else if(std::get<1>(*i) <= 0)
					clock_timers.erase_after(i = last);
			}
		}
		if(status == std::future_status::timeout)
			error = error/(CLOCK_ERROR_HISTORY-1)+std::max(0.0, elapsed-sleep)/CLOCK_ERROR_HISTORY;
		process_frame();
	}
	clock_mut.lock();
	delete ::clock_timer_promise;
	::clock_timer_promise = NULL;
	clock_mut.unlock();
}
/*}}}*/

/*{{{ void process_frame()*/
extern void trigger_timeouts();
extern void trns_clear_used();
static void process_frame() {
	trigger_timeouts();
	size_t locks;
	for(locks = 0; locks < devices.size(); locks++) { inputs_sem.acquire(); }
	for(auto a = inputs.begin(), b = user_inputs.begin(); a != inputs.end(); ++a, ++b) {
		for(auto aa = (*a).begin(), bb = (*b).begin(); aa != (*a).end(); ++aa, ++bb) {
			*bb = *aa;
		}
	}
	user_process_frame();
	for(size_t i = 0; i < locks; i++) { inputs_sem.release(); }
	trns_clear_used();
}
/*}}}*/

/*{{{ void select_device(device* dev, bool optional)*/
static void select_device(device* dev, bool optional) {
	bool refresh;
	do {
		refresh = false;
		size_t dev_count = (size_t)std::distance(std::filesystem::directory_iterator{"/dev/input/"}, std::filesystem::directory_iterator{});
		for(size_t dev_id = 0; dev_id < dev_count; dev_id++) {
			if((dev->fd = open(("/dev/input/event" + std::to_string(dev_id)).c_str(), O_RDONLY)) == -1) continue;
			if(libevdev_new_from_fd(dev->fd, &dev->dev) == 0) {
				std::cout << dev_id << ": " << libevdev_get_name(dev->dev) << std::endl;
				libevdev_free(dev->dev);
			}
			close(dev->fd);
		}
		dev->fd = -1;
		std::cout << "Press R to refresh" << ((optional) ? " or Escape to abort" : "") << std::endl;
		while(dev->fd == -1) {
			std::cout << "Device [0-" << dev_count << "]: ";
			std::string dev_id;
			int c;
			while((c = getchar()) != EOF) {
				if(c == '\n') {
					if(dev_id.size() > 0) break;
					std::cout << "\033[A\r\033[KDevice [0-" << dev_count << "]: ";
					continue;
				}
				else if(c == '\033' && optional) {
					std::cout << "\r\033[K";
					return;
				}
				else if(c == 'r' || c == 'R') {
					dev_id.clear();
					refresh = true;
					std::cout << std::endl;
					break;
				}
				else if(c == '\177') {
					std::cout << "\b\b  \b\b";
					if(!dev_id.empty()) {
						std::cout << "\b \b";
						dev_id.erase(dev_id.size()-1);
					}
					continue;
				}
				else if(c < '0' || c > '9') {
					if(c >= ' ') std::cout << "\b \b";
					else std::cout << "\r\033[KDevice [0-" << dev_count << "]: " << dev_id;
					continue;
				}
				dev_id += c;
			}
			// TODO
			if(c == EOF) exit(1);
			else if(refresh) break;
			if((dev->fd = open(("/dev/input/event" + dev_id).c_str(), O_RDONLY)) == -1) {
				std::cerr << "\033[A\r\033[KFailed to open device " << dev_id << "!" << std::endl;
				continue; }
			if(ioctl(dev->fd, EVIOCGRAB, 1) < 0) {
				std::cerr << "\033[A\r\033[KFailed to get device " << dev_id << " exclusively!" << std::endl;
				close(dev->fd); dev->fd = -1; continue; }
			if(libevdev_new_from_fd(dev->fd, &dev->dev) != 0) {
				std::cerr << "\033[A\r\033[KFailed to load device " << dev_id << " through evdev!" << std::endl;
				ioctl(dev->fd, EVIOCGRAB, 0);
				close(dev->fd);
				continue; }
			sscanf(dev_id.c_str(), "%zu", &dev->id);
		}
	} while(refresh);
}
/*}}}*/

/*{{{ void device_thread(device* dev)*/
static std::binary_semaphore device_thread_exit_sem{1};
static std::atomic<bool> device_thread_exited;
static std::thread::id device_thread_exit;
static void device_thread(device* dev) {
	inputs_sem.release();
	while(true) {
		struct input_event ev;
		int rc = libevdev_next_event(dev->dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
		if(stopping) break;
		if(rc == -EAGAIN) continue;
		else if(rc != 0) {
			std::cerr << "Error reading from device " << dev->id << " (" << rc << ")!" << std::endl;
			break;
		}
		// EV_MSC is used for syncodes, representing the physical button pressed
		// The thing the syncode maps to is sent afterwards and actually used
		// May want to implement MSCs for if games try to get raw or something, but shouldn't need to interpret them
		// type 4 (EV_MSC), code 4 (MSC_SCAN), value 90003
		// type 1 (EV_KEY), code 290 (BTN_THUMB2), value 0
		// XInput supports 8-direction D-pad, 2 triggers, 10 buttons, 4 axes (thumbsticks)
		// DirectInput supports POV (hat switch, overlapping 4-direction d-pad), 128 buttons, 8 axes
		if(!dev->calibration.done)
			calibration_event(dev, ev.type, ev.code, ev.value);
		else {
			switch(ev.type) {
				case EV_KEY:
					for(int i = 0; i < CNTLR_MAX_BUTTONS; i++) {
						if(!dev->calibration.buttons[i].present) break;
						if(dev->calibration.buttons[i].code == ev.code) {
							inputs_sem.acquire();
							dev->calibration.buttons[i].out->values[0] = ev.value;
							inputs_sem.release();
						}
					}
					break;
				//case EV_REL:
				case EV_ABS:
					for(int i = 0; i < CNTLR_MAX_ANALOGS; i++) {
						if(!dev->calibration.analogs[i].present) break;
						if(dev->calibration.analogs[i].code == ev.code) {
							inputs_sem.acquire();
							double* value = &dev->calibration.analogs[i].out->values[0];
							*value = ev.value-dev->calibration.analogs[i].center;
							*value *= (*value >= 0) ? dev->calibration.analogs[i].plus_scale : dev->calibration.analogs[i].minus_scale;
							inputs_sem.release();
						}
					}
					for(int i = 0; i < CNTLR_MAX_ANALOG_2DS; i++) {
						if(!dev->calibration.analog_2ds[i].present) break;
						if(dev->calibration.analog_2ds[i].x_code == ev.code) {
							dev->calibration.analog_2ds[i].updated = true;
							dev->calibration.analog_2ds[i].raw_x = ev.value;
						}
						if(dev->calibration.analog_2ds[i].y_code == ev.code) {
							dev->calibration.analog_2ds[i].updated = true;
							dev->calibration.analog_2ds[i].raw_y = ev.value;
						}
					}
					break;
				case EV_SYN:
					inputs_sem.acquire();
					for(int i = 0; i < CNTLR_MAX_ANALOG_2DS; i++) {
						if(dev->calibration.analog_2ds[i].updated) {
							double x = dev->calibration.analog_2ds[i].raw_x, y = dev->calibration.analog_2ds[i].raw_y;
							x -= dev->calibration.analog_2ds[i].center_x;
							y -= dev->calibration.analog_2ds[i].center_y;
							if(x < 0)
								x *= dev->calibration.analog_2ds[i].xl_scale;
							else
								x *= dev->calibration.analog_2ds[i].xr_scale;
							if(y < 0)
								y *= dev->calibration.analog_2ds[i].yd_scale;
							else
								y *= dev->calibration.analog_2ds[i].yu_scale;
							double angle = atan2(y, x);
							if(angle < 0) angle += 2*M_PI;
							double q_progress = angle;
							while(q_progress > M_PI/2) { q_progress -= M_PI/2; }
							q_progress /= M_PI/2;
							int q = (int)(angle/(M_PI/2));
							double scale = dev->calibration.analog_2ds[i].q_scales[q];
							scale = 1+std::min(1.0, cos(M_PI*pow(fabs(2*q_progress-1), dev->calibration.analog_2ds[i].q_pows[q]))+1)*(scale-1);
							x *= scale; y *= scale;
							dev->calibration.analog_2ds[i].out->values[0] = x;
							dev->calibration.analog_2ds[i].out->values[1] = -y;
							dev->calibration.analog_2ds[i].updated = false;
						}
					}
					inputs_sem.release();
					clock_mut.lock();
					if(clock_interrupt_sem.try_acquire()) {
						clock_timer_promise->set_value();
						clock_timer_promise = new std::promise<void>();
						clock_timer = clock_timer_promise->get_future().share();
						clock_mut.unlock();
					}
					else {
						clock_interrupt_later = true;
						clock_mut.unlock();
						std::this_thread::yield();
					}
			}
		}
	}
	libevdev_free(dev->dev);
	ioctl(dev->fd, EVIOCGRAB, 0);
	close(dev->fd);
	inputs_sem.acquire();
	device_thread_exit_sem.acquire();
	device_thread_exit = std::this_thread::get_id();
	device_thread_exited = true;
}
/*}}}*/

/*{{{ int main()*/
extern void init_scancodes();
int main() {
	init_scancodes();

	/*{{{ init*/
	struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	setbuf(stdout, NULL);
	std::cout.setf(std::ios::unitbuf);
	std::signal(SIGINT, signal);
	std::signal(SIGTERM, signal);

	std::thread clock(clock_thread);
	while(true) {
		clock_mut.lock();
		if(clock_timer_promise != NULL) {
			clock_mut.unlock();
			break;
		}
		clock_mut.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	std::map<std::thread::id, std::pair<std::thread, device*>> device_threads;
	/*}}}*/

	/*{{{ add first device*/
	device* dev = &devices.emplace_back();
	select_device(dev, false);
	if(dev->fd == -1) {
		clock_timer_promise->set_value();
		clock.join();
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
		return 0;
	}
	std::thread dev_thread(device_thread, dev);
	std::thread::id device_thread_id = dev_thread.get_id();
	device_threads[device_thread_id] = {std::move(dev_thread), dev};
	calibrate(dev);
	/*}}}*/

	// can only do this before user_switch_profile is called, it stores pointers
	// TODO: don't do these things here anyway
	user_inputs = decltype(inputs)(inputs);
	user_switch_profile(0, &user_inputs);

	/*{{{ TUI*/
	fd_set select_stdin;
	FD_ZERO(&select_stdin);
	FD_SET(STDIN_FILENO, &select_stdin);
	struct timeval tv;
	char c = '\0';
	while(true) {
		if(c != '\0') std::cout << "\r\033[K";
		switch(c) {
			//device* dev = &devices.emplace_back();
			//select_device(dev, true);
			//if(dev->fd != -1) {
			//	std::thread dev_thread(device_thread, dev);
			//	std::thread::id device_thread_id = dev_thread.get_id();
			//	device_threads[device_thread_id] = {std::move(dev_thread), dev};
			//	calibrate(dev);
			//}
			//else devices.pop_back();
		}
		c = '\0';

		/*{{{ handle disconnected devices*/
		if(device_thread_exited) {
			std::get<0>(device_threads[device_thread_exit]).join();
			for(auto i = devices.begin(); i != devices.end(); ++i) {
				if(&(*i) == std::get<1>(device_threads[device_thread_exit])) { devices.erase(i); break; }
			}
			device_threads.erase(device_thread_exit);
			device_thread_exited = false;
			device_thread_exit_sem.release();
			if(!stopping) {
				std::cout << "A hooked device was disconnected - exiting!" << std::endl;
				signal(SIGINT);
			}
			if(device_threads.empty()) {
				std::cout << "All devices were unhooked successfully" << std::endl;
				break;
			}
		}
		/*}}}*/

		/*{{{ get next key*/
		else if(!stopping) {
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			if(select(1, &select_stdin, NULL, NULL, &tv) == -1 && fcntl(STDIN_FILENO, F_GETFD) == -1) {
				if(!stopping) exit(1);
			}
			else {
				if(FD_ISSET(STDIN_FILENO, &select_stdin)) c = getchar();
				else {
					c = '\0';
					FD_SET(STDIN_FILENO, &select_stdin);
				}
			}
		}
		/*}}}*/

	}
	/*}}}*/

	/*{{{ teardown*/
	clock_mut.lock();
	if(clock_timer_promise != NULL)
		clock_timer_promise->set_value();
	clock_mut.unlock();
	clock.join();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return 0;
	/*}}}*/

}
/*}}}*/
