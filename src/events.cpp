/*
 * This file is part of KRASH.
 *
 *  KRASH is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  KRASH is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with KRASH.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright Swann Perarnau, 2008
 *  Contact: firstname.lastname@imag.fr
 */

#include <iostream>
#include <map>
#include "events.hpp"

/** Class EventDriver */
EventDriver::EventDriver(ActionsList& l) : list(l) {
	loop = new ev::default_loop(EVFLAG_AUTO);
	watcher = NULL;
	sig_watcher = NULL;
}

EventDriver::~EventDriver() {
	if(watcher != NULL)
		delete watcher;

	if(sig_watcher != NULL)
		delete sig_watcher;

	delete loop;
}

void EventDriver::start() {
	// before starting we setup an signal handler to call stop on SIGINT
	sig_watcher = new ev::sig(*loop);
	sig_watcher->set<EventDriver,&EventDriver::sigint_callback>(this);
	sig_watcher->set(SIGINT);
	sig_watcher->start();

	// start the loop
	// to start the event handling we define a timer watcher
	// and use it to wake up every time an action needs to be
	// performed.
	if(!list.empty()) {
		// set the start time
		start_time = ev::now();
		// init timer with first time deadline
		Action *a = list.top();

		watcher = new ev::timer(*loop);
		watcher->set<EventDriver,&EventDriver::timer_callback>(this);
		watcher->set(a->get_time(),a->get_time() + 100);
		// start timer
		watcher->start();
		// return only if this->stop() is called
		loop->loop(0);
	}
	else {
		// TODO error
	}
}

void EventDriver::stop() {
	loop->unloop(ev::ALL);
}

void EventDriver::sigint_callback(ev::sig &w, int revents) {
	std::cerr << "SIGINT catched, exiting krash" << std::endl;
	this->stop();
}

void EventDriver::timer_callback(ev::timer &w,int revents) {
	// this callback activate all actions that should
	// have been activated by the time it get called.
	// However, it filter the actions: only one action
	// per identifier gets called, and always the last one.
	// This allow KRASH to cope with time drift.

	// a map to register all actions to execute.
	std::map< std::string, Action *>todo;
	// research actions to execute.
	ev::tstamp now = ev::now() - start_time;
	std::cout << "Wakeup, we have been injecting load for " << now << " seconds" << std::endl;
	Action *a;
	while(!list.empty() && list.top()->get_time() <= now) {
		a = list.top();
		todo[a->get_id()] = a;
		list.pop();
	}
	// activate all actions
	std::map<std::string,Action *>::iterator it;
	for(it = todo.begin(); it != todo.end(); it++) {
		it->second->activate();
	}
	// reinit timer for next event
	if(!list.empty()) {
		a = list.top();
		w.set(a->get_time() - now, a->get_time() - now);
		w.again();
	}
}
