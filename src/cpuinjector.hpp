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

/** saves all configuration needed by the cpuinjector */
void cpuinjector_configure(std::string cpu_cg_root,std::string cpuset_cg_root,std::string all_name,std::string cg_basename,unsigned int all_prio);

/** moves all tasks present on cpu_cgroup_root
 * to a group one level below
 */
int setup_system();


/** setup a cpu: create a group for the burner and fork it */
int setup_cpu(unsigned int cpuid);

/** apply a share to a cpu
 * @param cpuid the cpuid to manipulate
 * @param share the share to apply
 */
int apply_share(unsigned int cpuid, unsigned int share);



#endif // !CPUINJECTOR_HPP
