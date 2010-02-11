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

/** This file is a wrapper around libcgroup */

#define CPU_CGROUP_NAME "cpu"
#define CPUSET_CGROUP_NAME "cpuset"


/** moves all tasks present on cpu_cgroup_root
 * to a group one level below
 * @param cpu_cgroup_root the cpu control group root under which we will work
 * @param alltasks_name the name given to the alltasks group
 */
int setup_system(std::string cpu_cgroup_root, std::string alltasks_name);


#endif // !CPUINJECTOR_HPP
