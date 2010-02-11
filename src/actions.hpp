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

#ifndef ACTIONS_HPP
#define ACTIONS_HPP 1

#include<string>
#include<queue>

/** Defines an action.
 * This class represents an action to execute at some point in time.
 *
 * Each action is represented by a time (since the beginning of the application)
 * and a identifier which represents the componant of KRASH needing to take action.
 */
class Action {
	public:
		/** basic constructor */
		Action(std::string id, unsigned int time);

		/** gets the time at which the action is supposed to activate.
		 * @return the time since the beginning of KRASH at which
		 *	we must activate this action.
		 */
		inline unsigned int get_time() const { return this->time;}

		/** gets the identifier associated
		 * @return the unique identifier associated with a component.
		 */
		inline std::string get_id() const { return this->id; }

		/** comparison operator
		 * @param a an action
		 * @return true if this.time > a.time
		 */
		inline bool operator>(const Action& a) const {
			return this->get_time() > a.get_time();
		}

		/** virtual method for the activation of an action */
		virtual void activate() { };

	protected:
		/** Time, since the beginning of KRASH, at which this action must
		 * be executed.
		 */
		unsigned int time;

		/** Unique identifier for the component needing this action. This identifier
		 * is used only to compare actions among them.
		 */
		std::string id;
};

/** this struct defines the operation needed to sort by
 * increasing time a list of pointers of Actions
 */
struct gt_pointers : std::binary_function <Action*, Action*, bool> {
	bool operator()(Action*& a, Action*& b) const {
		return a->operator>(*b);
	}
};

typedef std::priority_queue<Action*, std::vector<Action*>, gt_pointers> ActionsList;

/** CPU Injector action class
 *
 * This class specializes Action for the CPU Injector in KRASH.
 */
class CPUAction : public Action {
	public:
		/** Basic constructor
		 * @param id an identifier
		 * @param time the time to activate this action.
		 * @param cpu the cpu to load in this action.
		 * @param load the load to inflict in this action.
		 */
		CPUAction(std::string id, unsigned int time, unsigned int cpu, unsigned int load);

		/** Basic constructor
		 * @param time the time to activate this action.
		 * @param cpu the cpu to load in this action.
		 * @param load the load to inflict in this action.
		 */
		CPUAction(unsigned int time, unsigned int cpu, unsigned int load);

		/** gets the cpuid */
		inline unsigned int get_cpu();

		/** gets the load */
		inline unsigned int get_load();

		/** applies a load on the target cpu
		 * Using the load and cpu members, this function applies
		 * a load on the target CPU, using our CPU injector backend.
		 */
		void activate();
	protected:
		/** the target cpu of this action */
		unsigned int cpu;

		/** the load to apply on the target cpu */
		unsigned int load;
};



#endif // ACTIONS_HPP
