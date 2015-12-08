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
#include "cpuinjector.hpp"
#include "events.hpp"
#include "utils.hpp"
#include "cgroups.hpp"
#include "errors.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#include <typeinfo>
#include <map>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE // need by sched.h
#endif
#include <sched.h> // for affinity
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

namespace cpuinjector {
using namespace cgroups;

// STATIC INTERNAL VARIABLES
static std::map< unsigned int, Cgroup* > burners_cgs;
static std::map< unsigned int, pid_t > burners_pids;
static bool error_on_ev_child = true;
static std::map< unsigned int, ev::child * > burners_watchers;
static std::map< pid_t, unsigned int > burners_cpus;

void child_callback(ev::child &w, int revents) {
	unsigned int cpu;
	cpu = burners_cpus[w.rpid];
	burners_pids.erase(cpu);
	burners_watchers[cpu]->stop();
	delete burners_watchers[cpu];
	if(error_on_ev_child)
	{
		std::cerr << "Error: child " << w.pid << " exited with status "
			<< w.rstatus << std::endl;
				events::stop(1);
	}
}

// kill and wait a task
static int kill_wait(pid_t task) {
	int err;
	int ret = 0;
	error_on_ev_child = false;
	err = kill(task,SIGKILL);
	SAVE_RET(err,ret)
	return ret;
}

int setup(ActionsList& list) {
	int err;
	std::set<unsigned int> cpus;
	std::set<unsigned int>::iterator it;
	ActionsList copy;

	/* we need to copy the list for its inspection */
	copy = list;
	while(!copy.empty()) {
		Action *a = copy.top();
		if(typeid(*a) == typeid(CPUAction)) {
			CPUAction *ca = (CPUAction *)a;
			cpus.insert(ca->cpu);
		}
		copy.pop();
	}
	/* iterate through cpus to setup them */
	for(it = cpus.begin(); it != cpus.end(); it++) {
		err = setup_cpu(*it);
		TEST_FOR_ERROR(err,error);
	}
	return 0;
error:
	return err;
}

int setup_cpu(unsigned int cpuid) {
	/* what we need to do:
	 * - create a cpu control group
	 * - fork a process
	 * - make it a member of this group
	 * - restrict it to the cpu
	 * - register this cpu as active
	 */
	int err;
	Cgroup *burner;
	std::string name;
	pid_t burner_pid;

	/* create the burner cpu cgroup */
	name = "cpu.burner." + itos(cpuid);
	burner = new Cgroup(name);
	err = burner->lib_attach(true);
	TEST_FOR_ERROR(err,error_lib);

	/* fork a burner process */
	burner_pid = fork();
	if(burner_pid == -1) {
		translate_error(errno);
		goto error_fork;
	}
	if(burner_pid == 0) { // never leave this part
		/* attach myself to a cpu */
		cpu_set_t mycpuset;
		CPU_ZERO(&mycpuset);
		CPU_SET(cpuid,&mycpuset);
		err = sched_setaffinity(0,sizeof(cpu_set_t),&mycpuset);
		/* loop until the end of time */
		while(1);
	}

	/* attach pid to the burner group */
	err = burner->attach(burner_pid);
	TEST_FOR_ERROR(err,error_free);

	burners_cgs[cpuid] = burner;
	burners_pids[cpuid] = burner_pid;
	burners_watchers[cpuid] = new ev::child(*events::loop);
	burners_watchers[cpuid]->set<child_callback>(NULL);
	burners_watchers[cpuid]->set(burner_pid,0);
	burners_watchers[cpuid]->start();
	return 0;

error_free: // we do not recover from this, too hard
	kill_wait(burner_pid);
error_fork:
	burner->lib_detach();
error_lib:
	delete burner;
error:
	return err;
}

int apply_share(unsigned int cpuid, unsigned int share) {
	/* what we need to do
	 * - know the priority of the alltask group
	 * - compute the priority to apply to a given cpu group
	 * - apply it
	 */
	int err;
	double s,a,nprio;
	Cgroup *burner;
	u_int64_t all_prio,new_prio;

	/* retrieve the cpu cgroup for all and get the cpu.shares value */
	err = All->get_cpu_shares(&all_prio);
	TEST_FOR_ERROR(err,error);

	/* get the burner cgroup */
	burner = burners_cgs[cpuid];

	/* compute share
	 * Here is the value we seek: new_prio = (share/100) * (new_prio + all_prio)
	 * The real computation is derived from it.
	 * We do computations in floats for better precision.
	 */
	// adjust share because our formula does not allows the value 100
	if(share == 100)
		share = 99;

	s = share;
	a = all_prio;
	nprio = (a*s/100.0) * (100.0 / (100.0 - s));
	new_prio = nprio;

	/* set the new value */
	err = burner->set_cpu_shares(new_prio);
	TEST_FOR_ERROR(err,error);

	return 0;
error:
	return err;
}

// we get called with every exit, on success AND on failure
// so we must check for each component to ensure everything is in good shape
int cleanup() {
	int err;
	int ret= 0;

	// cleanup burners
	std::map<unsigned int, Cgroup*>::iterator it;
	for(it = burners_cgs.begin(); it != burners_cgs.end(); it++) {
		Cgroup *cg = it->second;

		// find the pid of the burner
		std::map<unsigned int, pid_t>::iterator b;
		b = burners_pids.find(it->first);
		if(b != burners_pids.end())
		{
			// kill and wait him
			err = kill_wait(b->second);
			SAVE_RET(err,ret);
		}
		// free the structure
		err = cg->lib_detach();
		SAVE_RET(err,ret);
	}
	burners_cgs.clear();
	burners_pids.clear();
	burners_watchers.clear();
	burners_cpus.clear();
	return ret;
}

} // end namespace

// CPUAction implementation

/* class CPUAction */
CPUAction::CPUAction(std::string id, unsigned int time, unsigned int cpu, unsigned int load) : Action(id,time) {
	this->cpu = cpu;
	this->load = load;
}

CPUAction::CPUAction(unsigned int time, unsigned int cpu, unsigned int load) : Action(std::string("cpu"),time) {
	this->cpu = cpu;
	this->load = load;
	this->id += itos(cpu);
}

int CPUAction::activate() {
	int err;
	std::cout << "Applying share " << this->load << " on cpu " << this->cpu << " asked for time " << this->time << std::endl;
	err = cpuinjector::apply_share(this->cpu,this->load);
	return err;
}

