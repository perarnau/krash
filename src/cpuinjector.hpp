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
#ifndef CPUINJECTOR_HPP
#define CPUINJECTOR_HPP 1
#include <string>
#include "actions.hpp"

namespace cpuinjector {

/** do all the setup needed by the cpu injector */
int setup(ActionsList& list);

/** apply a share to a cpu
 * @param cpuid the cpuid to manipulate
 * @param share the share to apply
 */
int apply_share(unsigned int cpuid, unsigned int share);

/** cleanup code: to call before exiting krash, leaves the
 * system in a clean state*/
int cleanup();

/** setup a cpu: create a group for the burner and fork it */
int setup_cpu(unsigned int cpuid);

} // end namespace

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

		/** applies a load on the target cpu
		 * Using the load and cpu members, this function applies
		 * a load on the target CPU, using our CPU injector backend.
		 */
		int activate();

		/** the target cpu of this action */
		unsigned int cpu;

		/** the load to apply on the target cpu */
		unsigned int load;
};

#endif // !CPUINJECTOR_HPP
