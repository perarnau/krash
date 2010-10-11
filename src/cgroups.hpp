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
#ifndef CGROUPS_HPP
#define CGROUSP_HPP 1

#include <libcgroup.h>
#include <iostream>

/** Contains all the cgroup interaction code */
namespace cgroups {

/** All krash operations share the same goal: inject load towards a group of task.
 * This variable determines the path (from the root cgroup) to the task group we inject.
 * Default is to inject towards the whole system. */
extern std::string target_path;

/** To inject load correctly we need to move the target group on level below.
 * This variable determines the created group's name. */
extern std::string alltasks_groupname;

/* encapsulates all cgroup operations, using the libcg
 * By default all cgroups created have the cpu controller attached
 * (group scheduling can be manipulated on the group)
 * Note: libcg is a mess, to avoid using the whole C++ artilery
 * each time the lib does a crappy job, Cgroup structure creation
 * and destruction is separated from libcg interaction. This
 * way the constructors don't have to throw exceptions.
 */
class Cgroup {
	public:
		/** Basic constructor
		 * @param name this cgroup name
		 */
		Cgroup(std::string name) {
			this->cg = NULL;
			this->path = target_path + name;
		}

		/** Basic destroyer */
		~Cgroup() {
			this->cg = NULL;
		}

		/** calls the libcg and really do the job */
		int lib_attach(bool create);

		/** detach from libcg
		 * Expects the cgroup to be empty */
		int lib_detach();

		/** Move pids from this group to the target */
		int move_pids(Cgroup target);

		/** attaches one task to this group */
		int attach(pid_t p);

		/** retrieves the cpu.share value of the cgroup */
		int get_cpu_shares(u_int64_t *shares);

		/** sets the cpu.shares value of this cgroup */
		int set_cpu_shares(u_int64_t val);
	private:
		/** libcg structure associated with this cgroup */
		struct cgroup *cg;
		/** the full name of this cgroup */
		std::string path;
};

/** this target Cgroup object associated with the target cgroup path */
extern Cgroup *target;

/** this Cgroup will contain all the target tasks */
extern Cgroup *All;

/** initializes the library,
 * creates the target and All Cgroups
 * moves all tasks to All */
int init();

/** destroys the groups, move all tasks to target */
int cleanup();

} // end cgroups namespace

#endif // define CGROUPS_HPP
