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
#ifndef EVENTS_HPP
#define EVENTS_HPP 1

#include <ev++.h>
#include "actions.hpp"

/** Contains a list of Actions
 *
 * This class handles a list of actions and the event loop code to activate KRASH componants when needed.
 */
class EventDriver {
	public:
		/** basic constructor */
		EventDriver(ActionsList& l);

		/** start the action handling, launching the event loop **/
		void start();

		/** stops the action handling, effectivelly making the start method
		 * to return */
		void stop();

		/** callback needed by the ev lib */
		void timer_callback(ev::timer &w, int revents);
	private:
		/** the list of actions, sorted by increasing times */
		ActionsList list;

		/** the event loop */
		struct ev::default_loop *loop;

		/** a timer watcher */
		ev::timer *watcher;

		/** the start time of the loop
		 * This is needed because ev handle timers
		 * in absolute time
		 */
		ev::tstamp start_time;
};


#endif // !EVENTS_HPP
