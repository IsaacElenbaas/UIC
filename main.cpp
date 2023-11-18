/*{{{ includes*/
#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <fcntl.h>
#include <filesystem>
#include <future>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <mutex>
#include <semaphore>
#include <set>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include "controller.h"
#include "main.h"
/*}}}*/

std::list<device> devices;
double elapsed = 0;

std::atomic<bool> stopping = false;
static int max_fd = 0;
static void signal(int signum) {
	std::cout << "\r\033[KSubmit an input on each device you used that is still connected to exit" << std::endl;
	stopping = true;
	close(STDIN_FILENO);
	// unfortunately closing evdev device FDs or freeing the device objects does not interrupt next_event
}

/*{{{ void push/pop_timer(input_t input, int time_ms, bool repeat)*/
static std::multiset<std::tuple<int, int, input_t>> clock_timers;

void push_timer(input_t input, int time_ms, bool repeat) {
	clock_timers.insert({0, ((repeat) ? 1 : -1)*abs(time_ms), input});
}

void pop_timer(input_t input, int time_ms, bool repeat) {
	time_ms = ((repeat) ? 1 : -1)*abs(time_ms);
	for(auto i = clock_timers.begin(); i != clock_timers.end(); ++i) {
		if(std::get<2>(*i) == input && std::get<1>(*i) == time_ms) {
			clock_timers.erase(i);
			break;
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
				sleep = std::min(sleep, (int)std::round(std::get<1>(*i)-std::get<0>(*i)));
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
		int elapsed_int = std::round(elapsed);
		last = now;
		decltype(clock_timers) new_clock_timers;
		for(auto i = clock_timers.begin(); i != clock_timers.end(); ++i) {
			auto new_timer = *i;
			std::get<0>(new_timer) += elapsed_int;
			if(std::get<0>(new_timer) >= abs(std::get<1>(new_timer))) {
				if(std::get<1>(new_timer) > 0) std::get<0>(new_timer) %= std::get<1>(new_timer);
				if(std::get<1>(*i) >= 0) new_clock_timers.insert(new_timer);
			}
			else new_clock_timers.insert(new_timer);
		}
		clock_timers = new_clock_timers;
		if(status == std::future_status::timeout)
			error = error/(CLOCK_ERROR_HISTORY-1)+std::max(0.0, elapsed-sleep)/CLOCK_ERROR_HISTORY;
		process_frame();
	}
	delete clock_timer_promise;
	clock_timer_promise = NULL;
}
/*}}}*/

static void process_frame() {
	std::cout << "process_frame" << std::endl;
}

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
				if(stopping) return;
				if(c == '\n') {
					if(dev_id.size() > 0) break;
					std::cout << "\033[A\033[KDevice [0-" << dev_count << "]: ";
					continue;
				}
				else if(c == '\033' && optional) {
					std::cout << "\r\033[K";
					return;
				}
				else if(c == 'r' || c == 'R') {
					dev_id.clear();
					refresh = true;
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
			if(c == EOF) exit(1);
			else if(refresh) break;
			if((dev->fd = open(("/dev/input/event" + dev_id).c_str(), O_RDONLY)) == -1) {
				std::cerr << "\033[A\033[KFailed to open device " << dev_id << "!" << std::endl;
				continue; }
			max_fd = std::max(max_fd, dev->fd);
			if(ioctl(dev->fd, EVIOCGRAB, 1) < 0) {
				std::cerr << "\033[A\033[KFailed to get device " << dev_id << " exclusively!" << std::endl;
				close(dev->fd); dev->fd = -1; continue; }
			if(libevdev_new_from_fd(dev->fd, &dev->dev) != 0) {
				std::cerr << "\033[A\033[KFailed to load device " << dev_id << " through evdev!" << std::endl;
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

		// EV_KEY       0x01
		// EV_REL       0x02
		// EV_ABS       0x03
		//   libevdev_get_abs_minimum/maximum(dev, code)
		// XInput supports 8-direction D-pad, 2 triggers, 10 buttons, 4 axes (thumbsticks)
		// DirectInput supports POV (hat switch, overlapping 4-direction d-pad), 128 buttons, 8 axes
		if(!dev->calibration.done)
			calibration_event(dev, ev.type, ev.code, ev.value);
		else {
			switch(ev.type) {
				case EV_KEY:
				//case EV_REL:
				case EV_ABS:
					std::cout << "Event type: " << ev.type << ", code: " << ev.code << ", value: " << ev.value << std::endl;
					break;
				case EV_SYN:
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
	device_thread_exit_sem.acquire();
	device_thread_exit = std::this_thread::get_id();
	device_thread_exited = true;
}
/*}}}*/

/*{{{ int main()*/
int main() {

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
			if(device_threads.empty()) {
				if(stopping) break;
				else {
					std::cout << "All devices were disconnected!" << std::endl;
					dev = &devices.emplace_back();
					select_device(dev, false);
					if(dev->fd == -1) break;
					std::thread dev_thread(device_thread, dev);
					std::thread::id device_thread_id = dev_thread.get_id();
					device_threads[device_thread_id] = {std::move(dev_thread), dev};
				}
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
