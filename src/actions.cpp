#include "actions.hpp"

/** for debug purpose only */
#include<iostream>

/** for time drifting code */
#include<map>

/* class Action */
Action::Action(std::string id, unsigned int time) {
	this->id = id;
	this->time = time;
}

/* class CPUAction */
CPUAction::CPUAction(std::string id, unsigned int time, unsigned int cpu, unsigned int load) : Action(id,time) {
	this->cpu = cpu;
	this->load = load;
}

inline unsigned int CPUAction::get_cpu() {
	return this->cpu;
}

inline unsigned int CPUAction::get_load() {
	return this->load;
}

void CPUAction::activate() {
	std::cout << "Activated: my cpu: " << get_cpu() << " my id: " << get_id() << std::endl;
}

/** Class ActionList */
ActionList::ActionList() {
	loop = new ev::default_loop(EVFLAG_AUTO);
	watcher = new ev::timer(*loop);
	watcher->set<ActionList,&ActionList::timer_callback>(this);
}

void ActionList::add_action(Action *a) {
	list.push(a);
}

void ActionList::start() {
	// to start the event handling we define a timer watcher
	// and use it to wake up every time an action needs to be
	// performed.
	if(!list.empty()) {
		// set the start time
		start_time = ev::now();
		// init timer with first time deadline
		Action *a = list.top();
		std::cout << "Starting: now: " << start_time << " next event: " << a->get_time() << std::endl;
		watcher->set(a->get_time(),a->get_time() + 100);
		// start timer
		watcher->start();
		// DOES NOT RETURN !
		loop->loop(0);
	}
	else {
		// TODO error
	}
}

void ActionList::timer_callback(ev::timer &w,int revents) {
	// this callback activate all actions that should
	// have been activated by the time it get called.
	// However, it filter the actions: only one action
	// per identifier gets called, and always the last one.
	// This allow KRASH to cope with time drift.

	// a map to register all actions to execute.
	std::map< std::string, Action *>todo;
	// research actions to execute.
	ev::tstamp now = ev::now() - start_time;
	std::cout << "Timer callback: now: " << now << std::endl;
	Action *a;
	while(!list.empty() && list.top()->get_time() <= now) {
		a = list.top();
		todo[a->get_id()] = a;
		list.pop();
	}
	std::cout << "Map: size: " << todo.size() << " top: " << list.top()->get_time() << std::endl;
	// activate all actions
	std::map<std::string,Action *>::iterator it;
	for(it = todo.begin(); it != todo.end(); it++) {
		it->second->activate();
		std::cout << "Activated : id: " << it->second->get_id() << " time: " << it->second->get_time() << std::endl;
	}
	// reinit timer for next event
	if(!list.empty()) {
		a = list.top();
		std::cout << "Next time is: " << a->get_time() << std::endl;
		w.set(a->get_time() - now, a->get_time() - now);
		w.again();
	}
}
